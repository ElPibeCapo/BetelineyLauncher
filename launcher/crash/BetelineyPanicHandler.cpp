// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "BetelineyPanicHandler.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>

// ── Parte 1: signal handler (sin Qt, solo syscalls seguros) ────────────────

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)

#include <csignal>
#include <cstring>
#include <execinfo.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

static const char CRASH_PREFIX[] = "/tmp/beteliney_crash_";

// Escribe una cadena C en un fd usando write() (async-signal-safe)
static void safeWrite(int fd, const char* s)
{
    write(fd, s, strlen(s));
}

static void panicHandler(int sig)
{
    // Construir nombre de archivo: /tmp/beteliney_crash_<pid>.txt
    char path[64];
    char pidStr[24];
    pid_t pid = getpid();

    // itoa manual (async-signal-safe)
    int i = 0;
    pid_t tmp = pid;
    if (tmp == 0) {
        pidStr[i++] = '0';
    } else {
        while (tmp > 0) { pidStr[i++] = '0' + (tmp % 10); tmp /= 10; }
        // revertir
        for (int a = 0, b = i - 1; a < b; ++a, --b) {
            char c = pidStr[a]; pidStr[a] = pidStr[b]; pidStr[b] = c;
        }
    }
    pidStr[i] = '\0';

    memcpy(path, CRASH_PREFIX, sizeof(CRASH_PREFIX) - 1);
    memcpy(path + sizeof(CRASH_PREFIX) - 1, pidStr, i + 1);
    memcpy(path + sizeof(CRASH_PREFIX) - 1 + i, ".txt", 5);

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        safeWrite(fd, "BetelineyLauncher crash report\n");
        safeWrite(fd, "Signal: ");
        safeWrite(fd, strsignal(sig));
        safeWrite(fd, "\n\nBacktrace:\n");

        void* frames[64];
        int n = backtrace(frames, 64);
        backtrace_symbols_fd(frames, n, fd);
        close(fd);
    }

    // Restaurar handler por defecto y re-lanzar la señal (para core dump)
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigaction(sig, &sa, nullptr);
    raise(sig);
}

void Beteliney::installPanicHandler()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = panicHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;

    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGFPE,  &sa, nullptr);
    sigaction(SIGILL,  &sa, nullptr);
}

#elif defined(Q_OS_WIN32)

#include <windows.h>
#include <dbghelp.h>

static LONG WINAPI windowsPanicHandler(EXCEPTION_POINTERS* ep)
{
    // Escribir minidump en %TEMP%\beteliney_crash_<pid>.dmp
    wchar_t path[MAX_PATH];
    GetTempPathW(MAX_PATH, path);
    DWORD pid = GetCurrentProcessId();
    swprintf_s(path + wcslen(path), MAX_PATH - wcslen(path),
               L"beteliney_crash_%lu.dmp", pid);

    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mei;
        mei.ThreadId          = GetCurrentThreadId();
        mei.ExceptionPointers = ep;
        mei.ClientPointers    = FALSE;
        MiniDumpWriteDump(GetCurrentProcess(), pid, hFile,
                          MiniDumpNormal, &mei, nullptr, nullptr);
        CloseHandle(hFile);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

void Beteliney::installPanicHandler()
{
    SetUnhandledExceptionFilter(windowsPanicHandler);
}

#else
void Beteliney::installPanicHandler() {}
#endif

// ── Parte 2: detección al arrancar + diálogo Qt ────────────────────────────

void Beteliney::checkAndShowCrashReport(const QString& launcherVersion)
{
    // Buscar archivos de crash de sesiones anteriores
#if defined(Q_OS_WIN32)
    QDir tmp(qEnvironmentVariable("TEMP"));
    auto filter = QStringList{ "beteliney_crash_*.dmp" };
#else
    QDir tmp("/tmp");
    auto filter = QStringList{ "beteliney_crash_*.txt" };
#endif

    auto crashFiles = tmp.entryInfoList(filter, QDir::Files, QDir::Time);
    if (crashFiles.isEmpty())
        return;

    QFileInfo crashFile = crashFiles.first();  // el más reciente

    // Leer contenido
    QString reportText;
    QFile f(crashFile.absoluteFilePath());
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts(&f);
        reportText = ts.readAll();
        f.close();
    }

    // Mostrar diálogo
    QDialog dlg;
    dlg.setWindowTitle(QObject::tr("BetelineyLauncher — Crash detectado"));
    dlg.setMinimumSize(640, 480);
    auto* layout = new QVBoxLayout(&dlg);

    auto* header = new QLabel(
        QObject::tr("<b>El launcher se bloqueó en la sesión anterior.</b><br>"
                    "A continuación el reporte técnico. Podés ignorarlo o "
                    "reportarlo en GitHub para ayudar a mejorar el launcher."));
    header->setWordWrap(true);
    layout->addWidget(header);

    auto* textEdit = new QPlainTextEdit(reportText);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Monospace", 9));
    layout->addWidget(textEdit);

    auto* buttons = new QDialogButtonBox;
    auto* btnIgnore  = buttons->addButton(QObject::tr("Ignorar"),   QDialogButtonBox::RejectRole);
    auto* btnCopy    = buttons->addButton(QObject::tr("Copiar reporte"), QDialogButtonBox::ActionRole);
    auto* btnReport  = buttons->addButton(QObject::tr("Reportar en GitHub"), QDialogButtonBox::ActionRole);
    layout->addWidget(buttons);

    QObject::connect(btnIgnore, &QPushButton::clicked, &dlg, &QDialog::reject);

    QObject::connect(btnCopy, &QPushButton::clicked, [&reportText] {
        QGuiApplication::clipboard()->setText(reportText);
    });

    QObject::connect(btnReport, &QPushButton::clicked, [&reportText, &launcherVersion, &dlg] {
        QString title = QString("Crash report — BetelineyLauncher %1").arg(launcherVersion);
        QString body  = QString("**Versión:** %1\n\n**Descripción del problema:**\n"
                                "El launcher se bloqueó inesperadamente.\n\n"
                                "**Reporte de crash:**\n```\n%2\n```")
                            .arg(launcherVersion, reportText.left(4000));

        QUrlQuery q;
        q.addQueryItem("title", title);
        q.addQueryItem("body",  body);
        q.addQueryItem("labels", "crash,bug");

        QUrl url("https://github.com/ElPibeCapo/BetelineyLauncher/issues/new");
        url.setQuery(q);
        QDesktopServices::openUrl(url);
        dlg.accept();
    });

    dlg.exec();

    // Borrar el archivo de crash después de mostrarlo
    QFile::remove(crashFile.absoluteFilePath());
}
