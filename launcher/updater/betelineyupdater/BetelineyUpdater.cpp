// SPDX-FileCopyrightText: 2022 Rachel Powers <508861+Ryex@users.noreply.github.com>
//
// SPDX-License-Identifier: GPL-3.0-only

/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *  Copyright (C) 2022 Rachel Powers <508861+Ryex@users.noreply.github.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "BetelineyUpdater.h"
#include "BuildConfig.h"
#include "ui/dialogs/ProgressDialog.h"

#include <cstdlib>
#include <iostream>

#include <QDebug>

#include <QAccessible>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProgressDialog>
#include <memory>

// sys.h eliminado (librería systeminfo reemplazada por SysInfo.h)

#if defined Q_OS_WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "console/WindowsConsole.h"
#endif

#include <filesystem>
namespace fs = std::filesystem;

#include "DesktopServices.h"

#include "updater/betelineyupdater/UpdaterDialogs.h"

#include "FileSystem.h"
#include "Json.h"
#include "StringUtils.h"

#include "net/Download.h"
#include "net/RawHeaderProxy.h"

#include "BetelineyZip.h"

/** output to the log file */
void appDebugOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    static std::mutex loggerMutex;
    const std::lock_guard<std::mutex> lock(loggerMutex);  // synchronized, QFile logFile is not thread-safe

    QString out = qFormatLogMessage(type, context, msg);
    out += QChar::LineFeed;

    BetelineyUpdaterApp* app = static_cast<BetelineyUpdaterApp*>(QCoreApplication::instance());
    app->logFile->write(out.toUtf8());
    app->logFile->flush();
    if (app->logToConsole) {
        QTextStream(stderr) << out.toLocal8Bit();
        fflush(stderr);
    }
}

BetelineyUpdaterApp::BetelineyUpdaterApp(int& argc, char** argv) : QApplication(argc, argv)
{
#if defined Q_OS_WIN32
    // attach the parent console if stdout not already captured
    if (AttachWindowsConsole()) {
        consoleAttached = true;
    }
#endif
    setOrganizationName(BuildConfig.LAUNCHER_NAME);
    setOrganizationDomain(BuildConfig.LAUNCHER_DOMAIN);
    setApplicationName(BuildConfig.LAUNCHER_NAME + "Updater");
    setApplicationVersion(BuildConfig.printableVersionString() + "\n" + BuildConfig.GIT_COMMIT);

    // Command line parsing
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Actualizador automático de Beteliney Launcher"));

    parser.addOptions(
        { { { "d", "dir" }, tr("Usar una ruta personalizada como raíz de la aplicación (usa '.' para el directorio actual)."), tr("directorio") },
          { { "V", "beteliney-version" },
            tr("Usar esta versión como la versión instalada del launcher. (necesario porque stdout no se puede capturar de forma confiable en windows)"),
            tr("versión instalada del launcher") },
          { { "I", "install-version" }, tr("Instalar una versión específica."), tr("nombre de versión") },
          { { "U", "update-url" }, tr("Actualizar desde el repositorio especificado."), tr("url del repo de github") },
          { { "c", "check-only" },
            tr("Solo verificar si se necesita una actualización. Código de salida 100 si es necesario, 0 si no (o distinto de 0 si hubo un error).") },
          { { "p", "pre-release" }, tr("Permitir actualizar a versiones pre-release") },
          { { "F", "force" }, tr("Forzar una actualización, incluso si no es necesaria.") },
          { { "l", "list" }, tr("Listar versiones disponibles.") },
          { "debug", tr("Registrar depuración en la consola.") },
          { { "S", "select-ui" }, tr("Seleccionar la versión a instalar con una interfaz gráfica.") },
          { { "D", "allow-downgrade" }, tr("Permitir que el actualizador retroceda a versiones anteriores.") } });

    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(arguments());

    logToConsole = parser.isSet("debug");

    QString origCwdPath = QDir::currentPath();
    QString binPath = applicationDirPath();

    {  // find data director
       // Root path is used for updates and portable data
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD)
        QDir foo(FS::PathCombine(binPath, ".."));  // typically portable-root or /usr
        m_rootPath = foo.absolutePath();
#elif defined(Q_OS_WIN32)
        m_rootPath = binPath;
#elif defined(Q_OS_MAC)
        QDir foo(FS::PathCombine(binPath, "../.."));
        m_rootPath = foo.absolutePath();
        // on macOS, touch the root to force Finder to reload the .app metadata (and fix any icon change issues)
        FS::updateTimestamp(m_rootPath);
#endif
    }

    QString adjustedBy;
    // change folder
    QString dirParam = parser.value("dir");
    if (!dirParam.isEmpty()) {
        // the dir param. it makes beteliney launcher data path point to whatever the user specified
        // on command line
        adjustedBy = "Command line";
        m_dataPath = dirParam;
#ifndef Q_OS_MACOS
        if (QDir(FS::PathCombine(m_rootPath, "UserData")).exists()) {
            m_isPortable = true;
        }
        if (QFile::exists(FS::PathCombine(m_rootPath, "portable.txt"))) {
            m_isPortable = true;
        }
#endif
    } else if (auto dataDirEnv =
                   QProcessEnvironment::systemEnvironment().value(QString("%1_DATA_DIR").arg(BuildConfig.LAUNCHER_NAME.toUpper()));
               !dataDirEnv.isEmpty()) {
        adjustedBy = "System environment";
        m_dataPath = dataDirEnv;
#ifndef Q_OS_MACOS
        if (QFile::exists(FS::PathCombine(m_rootPath, "portable.txt"))) {
            m_isPortable = true;
        }
#endif
    } else {
        QDir foo(FS::PathCombine(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation), ".."));
        m_dataPath = foo.absolutePath();
        adjustedBy = "Persistent data path";

#ifndef Q_OS_MACOS
        if (auto portableUserData = FS::PathCombine(m_rootPath, "UserData"); QDir(portableUserData).exists()) {
            m_dataPath = portableUserData;
            adjustedBy = "Portable user data path";
            m_isPortable = true;
        } else if (QFile::exists(FS::PathCombine(m_rootPath, "portable.txt"))) {
            m_dataPath = m_rootPath;
            adjustedBy = "Portable data path";
            m_isPortable = true;
        }
#endif
    }

    m_updateLogPath = FS::PathCombine(m_dataPath, "logs", "beteliney_update.log");

    {  // setup logging
        FS::ensureFolderPathExists(FS::PathCombine(m_dataPath, "logs"));
        static const QString baseLogFile = BuildConfig.LAUNCHER_NAME + "Updater" + (m_checkOnly ? "-CheckOnly" : "") + "-%0.log";
        static const QString logBase = FS::PathCombine(m_dataPath, "logs", baseLogFile);

        if (FS::ensureFolderPathExists("logs")) {  // enough history to track both launches of the updater during a portable install
            FS::move(logBase.arg(1), logBase.arg(2));
            FS::move(logBase.arg(0), logBase.arg(1));
        }

        logFile = std::unique_ptr<QFile>(new QFile(logBase.arg(0)));
        if (!logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            showFatalErrorMessage(tr("¡La carpeta de datos del launcher no es escribible!"),
                                  tr("El actualizador no pudo crear un archivo de log: la carpeta de datos no es escribible.\n"
                                     "\n"
                                     "Asegúrate de tener permisos de escritura en la carpeta de datos.\n"
                                     "(%1)\n"
                                     "\n"
                                     "El actualizador no puede continuar hasta que arregles este problema.")
                                      .arg(m_dataPath));
            return;
        }
        qInstallMessageHandler(appDebugOutput);

        qSetMessagePattern(
            "%{time process}"
            " "
            "%{if-debug}D%{endif}"
            "%{if-info}I%{endif}"
            "%{if-warning}W%{endif}"
            "%{if-critical}C%{endif}"
            "%{if-fatal}F%{endif}"
            " "
            "|"
            " "
            "%{if-category}[%{category}]: %{endif}"
            "%{message}");

        bool foundLoggingRules = false;

        auto logRulesFile = QStringLiteral("qtlogging.ini");
        auto logRulesPath = FS::PathCombine(m_dataPath, logRulesFile);

        qDebug() << "Testing" << logRulesPath << "...";
        foundLoggingRules = QFile::exists(logRulesPath);

        // search the dataPath()
        // seach app data standard path
        if (!foundLoggingRules && !isPortable() && dirParam.isEmpty()) {
            logRulesPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, FS::PathCombine("..", logRulesFile));
            if (!logRulesPath.isEmpty()) {
                qDebug() << "Found" << logRulesPath << "...";
                foundLoggingRules = true;
            }
        }
        // seach root path
        if (!foundLoggingRules) {
            logRulesPath = FS::PathCombine(m_rootPath, logRulesFile);
            qDebug() << "Testing" << logRulesPath << "...";
            foundLoggingRules = QFile::exists(logRulesPath);
        }

        if (foundLoggingRules) {
            // load and set logging rules
            qDebug() << "Loading logging rules from:" << logRulesPath;
            QSettings loggingRules(logRulesPath, QSettings::IniFormat);
            loggingRules.beginGroup("Rules");
            QStringList rule_names = loggingRules.childKeys();
            QStringList rules;
            qDebug() << "Setting log rules:";
            for (auto rule_name : rule_names) {
                auto rule = QString("%1=%2").arg(rule_name).arg(loggingRules.value(rule_name).toString());
                rules.append(rule);
                qDebug() << "    " << rule;
            }
            auto rules_str = rules.join("\n");
            QLoggingCategory::setFilterRules(rules_str);
        }

        qDebug() << "<> Log initialized.";
    }

    {  // log debug program info
        qDebug() << qPrintable(BuildConfig.LAUNCHER_DISPLAYNAME + " Updater, " +
                               QString(BuildConfig.LAUNCHER_COPYRIGHT).replace("\n", ", "));
        qDebug() << "Version                    :" << BuildConfig.printableVersionString();
        qDebug() << "Git commit                 :" << BuildConfig.GIT_COMMIT;
        qDebug() << "Git refspec                :" << BuildConfig.GIT_REFSPEC;
        qDebug() << "Compiled for               :" << BuildConfig.systemID();
        qDebug() << "Compiled by                :" << BuildConfig.compilerID();
        qDebug() << "Build Artifact             :" << BuildConfig.BUILD_ARTIFACT;
        if (adjustedBy.size()) {
            qDebug() << "Data dir before adjustment :" << origCwdPath;
            qDebug() << "Data dir after adjustment  :" << m_dataPath;
            qDebug() << "Adjusted by                :" << adjustedBy;
        } else {
            qDebug() << "Data dir                   :" << QDir::currentPath();
        }
        qDebug() << "Work dir                   :" << QDir::currentPath();
        qDebug() << "Binary path                :" << binPath;
        qDebug() << "Application root path      :" << m_rootPath;
        qDebug() << "Portable install           :" << m_isPortable;
        qDebug() << "<> Paths set.";
    }

    {  // network
        m_network = makeShared<QNetworkAccessManager>(new QNetworkAccessManager());
        qDebug() << "Detecting proxy settings...";
        QNetworkProxy proxy = QNetworkProxy::applicationProxy();
        m_network->setProxy(proxy);
    }

#ifdef Q_OS_MACOS
    showFatalErrorMessage(tr("MacOS no soportado"), tr("El actualizador no soporta instalaciones en MacOS"));
#endif

    if (binPath.startsWith("/tmp/.mount_")) {
        m_isAppimage = true;
        m_appimagePath = QProcessEnvironment::systemEnvironment().value(QStringLiteral("APPIMAGE"));
        if (m_appimagePath.isEmpty()) {
            showFatalErrorMessage(tr("Instalación no soportada"),
                                  tr("¿El actualizador se está ejecutando como un AppImage mal configurado? (falta la variable de entorno $APPIMAGE)"));
        }
    }

    m_isFlatpak = DesktopServices::isFlatpak();

    QString beteliney_executable = FS::PathCombine(binPath, BuildConfig.LAUNCHER_APP_BINARY_NAME);
#if defined Q_OS_WIN32
    beteliney_executable.append(".exe");
#endif

    if (!QFileInfo(beteliney_executable).isFile()) {
        showFatalErrorMessage(tr("Instalación no soportada"), tr("El actualizador no puede encontrar el ejecutable principal."));
    }

    m_betelineyExecutable = beteliney_executable;

    auto beteliney_update_url = parser.value("update-url");
    if (beteliney_update_url.isEmpty())
        beteliney_update_url = BuildConfig.UPDATER_GITHUB_REPO;

    m_betelineyRepoUrl = QUrl::fromUserInput(beteliney_update_url);

    m_checkOnly = parser.isSet("check-only");
    m_forceUpdate = parser.isSet("force");
    m_printOnly = parser.isSet("list");
    auto user_version = parser.value("install-version");
    if (!user_version.isEmpty()) {
        m_userSelectedVersion = Version(user_version);
    }
    m_selectUI = parser.isSet("select-ui");
    m_allowDowngrade = parser.isSet("allow-downgrade");

    auto version = parser.value("beteliney-version");
    if (!version.isEmpty()) {
        if (version.contains('-')) {
            auto index = version.indexOf('-');
            m_betelineyVersionChannel = version.mid(index + 1);
            version = version.left(index);
        } else {
            m_betelineyVersionChannel = "stable";
        }
        auto version_parts = version.split('.');
        m_betelineyVersionMajor = version_parts.takeFirst().toInt();
        m_betelineyVersionMinor = version_parts.takeFirst().toInt();
        if (!version_parts.isEmpty())
            m_betelineyVersionPatch = version_parts.takeFirst().toInt();
        else
            m_betelineyVersionPatch = 0;
    }

    m_allowPreRelease = parser.isSet("pre-release");

    auto marker_file_path = QDir(m_rootPath).absoluteFilePath(".beteliney_updater_unpack.marker");
    auto marker_file = QFileInfo(marker_file_path);
    if (marker_file.exists()) {
        auto target_dir = QString(FS::read(marker_file_path)).trimmed();
        if (target_dir.isEmpty()) {
            qWarning() << "Empty updater marker file contains no install target. making best guess of parent dir";
            target_dir = QDir(m_rootPath).absoluteFilePath("..");
        }

        QMetaObject::invokeMethod(this, [this, target_dir]() { moveAndFinishUpdate(target_dir); }, Qt::QueuedConnection);

    } else {
        QMetaObject::invokeMethod(this, &BetelineyUpdaterApp::loadReleaseList, Qt::QueuedConnection);
    }
}

BetelineyUpdaterApp::~BetelineyUpdaterApp()
{
    qDebug() << "updater shutting down";
    // Shut down logger by setting the logger function to nothing
    qInstallMessageHandler(nullptr);

#if defined Q_OS_WIN32
    // Detach from Windows console
    if (consoleAttached) {
        fclose(stdout);
        fclose(stdin);
        fclose(stderr);
        FreeConsole();
    }
#endif
}

void BetelineyUpdaterApp::fail(const QString& reason)
{
    qCritical() << qPrintable(reason);
    m_status = Failed;
    exit(1);
}

void BetelineyUpdaterApp::abort(const QString& reason)
{
    qCritical() << qPrintable(reason);
    m_status = Aborted;
    exit(2);
}

void BetelineyUpdaterApp::showFatalErrorMessage(const QString& title, const QString& content)
{
    m_status = Failed;
    auto msgBox = new QMessageBox();
    msgBox->setWindowTitle(title);
    msgBox->setText(content);
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->setDefaultButton(QMessageBox::Ok);
    msgBox->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextBrowserInteraction);
    msgBox->setIcon(QMessageBox::Critical);
    msgBox->setMinimumWidth(460);
    msgBox->adjustSize();
    msgBox->exec();
    exit(1);
}

void BetelineyUpdaterApp::run()
{
    qDebug() << "found" << m_releases.length() << "releases on github";
    qDebug() << "loading exe at" << m_betelineyExecutable;

    if (m_printOnly) {
        printReleases();
        m_status = Succeeded;
        return exit(0);
    }

    if (!loadBetelineyVersionFromExe(m_betelineyExecutable)) {
        m_betelineyVersion = BuildConfig.printableVersionString();
        m_betelineyVersionMajor = BuildConfig.VERSION_MAJOR;
        m_betelineyVersionMinor = BuildConfig.VERSION_MINOR;
        m_betelineyVersionPatch = BuildConfig.VERSION_PATCH;
        m_betelineyVersionChannel = BuildConfig.VERSION_CHANNEL;
        m_betelineyGitCommit = BuildConfig.GIT_COMMIT;
    }
    m_status = Succeeded;

    qDebug() << "Executable reports as:" << m_betelineyBinaryName << "version:" << m_betelineyVersion;
    qDebug() << "Version major:" << m_betelineyVersionMajor;
    qDebug() << "Version minor:" << m_betelineyVersionMinor;
    qDebug() << "Version minor:" << m_betelineyVersionPatch;
    qDebug() << "Version channel:" << m_betelineyVersionChannel;
    qDebug() << "Git Commit:" << m_betelineyGitCommit;

    auto latest = getLatestRelease();
    qDebug() << "Latest release" << latest.version;
    auto need_update = needUpdate(latest);

    if (m_checkOnly) {
        if (need_update) {
            QTextStream stdOutStream(stdout);
            stdOutStream << "Name: " << latest.name << "\n";
            stdOutStream << "Version: " << latest.tag_name << "\n";
            stdOutStream << "TimeStamp: " << latest.created_at.toString(Qt::ISODate) << "\n";
            stdOutStream << latest.body << "\n";
            stdOutStream.flush();

            return exit(100);
        } else {
            return exit(0);
        }
    }

    if (m_isFlatpak) {
        showFatalErrorMessage(tr("Actualización de flatpak no soportada"), tr("Las acciones más allá de verificar si hay una actualización disponible no "
                                                                        "están soportadas en la versión flatpak de Beteliney Launcher."));
        return;
    }
    if (m_isAppimage) {
        bool result = true;
        if (need_update)
            result = callAppImageUpdate();
        return exit(result ? 0 : 1);
    }

    if (need_update || m_forceUpdate || !m_userSelectedVersion.isEmpty()) {
        GitHubRelease update_release = latest;
        if (!m_userSelectedVersion.isEmpty()) {
            bool found = false;
            for (auto rls : m_releases) {
                if (rls.version == m_userSelectedVersion) {
                    found = true;
                    update_release = rls;
                    break;
                }
            }
            if (!found) {
                showFatalErrorMessage(
                    tr("No hay release para esa versión"),
                    tr("No se encontró un release de GitHub para la versión especificada %1").arg(m_userSelectedVersion.toString()));
                return;
            }
        } else if (m_selectUI) {
            update_release = selectRelease();
            if (!update_release.isValid()) {
                showFatalErrorMessage(tr("No se seleccionó versión."), tr("No se seleccionó ninguna versión."));
                return;
            }
        }

        performUpdate(update_release);
    }

    exit(0);
}

void BetelineyUpdaterApp::moveAndFinishUpdate(QDir target)
{
    logUpdate(tr("Finalizando el proceso de actualización"));

    logUpdate(tr("Esperando 2 segundos para liberar recursos"));
    this->thread()->sleep(2);

    auto manifest_path = FS::PathCombine(m_rootPath, "manifest.txt");
    QFileInfo manifest(manifest_path);

    auto app_dir = QDir(m_rootPath);

    QStringList file_list;
    if (manifest.isFile()) {
        // load manifest from file
        logUpdate(tr("Leyendo manifiesto desde %1").arg(manifest.absoluteFilePath()));
        try {
            auto contents = QString::fromUtf8(FS::read(manifest.absoluteFilePath()));
            auto files = contents.split('\n');
            for (auto file : files) {
                file_list.append(file.trimmed());
            }
        } catch (FS::FileSystemException&) {
        }
    }

    if (file_list.isEmpty()) {
        logUpdate(tr("Manifiesto vacío, adivinando el contenido del directorio de %1").arg(m_rootPath));
        auto entries = target.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
        for (auto entry : entries) {
            file_list.append(entry.fileName());
        }
    }
    logUpdate(tr("Instalando lo siguiente en %1 :\n %2").arg(target.absolutePath()).arg(file_list.join(",\n  ")));

    bool error = false;

    QProgressDialog progress(tr("Instalando desde %1").arg(m_rootPath), "", 0, file_list.length());
    progress.setCancelButton(nullptr);
    progress.setMinimumWidth(400);
    progress.adjustSize();
    progress.show();
    QCoreApplication::processEvents();

    logUpdate(tr("Instalando desde %1").arg(m_rootPath));

    auto copy = [this, app_dir, target](QString to_install_file) {
        auto rel_path = app_dir.relativeFilePath(to_install_file);
        auto install_path = FS::PathCombine(target.absolutePath(), rel_path);
        logUpdate(tr("Instalando %1 desde %2").arg(install_path).arg(to_install_file));
        FS::ensureFilePathExists(install_path);
        auto result = FS::copy(to_install_file, install_path).overwrite(true)();
        if (!result) {
            logUpdate(tr("Falló la copia de %1 a %2").arg(to_install_file).arg(install_path));
            return true;
        }
        return false;
    };

    int i = 0;
    for (auto glob : file_list) {
        QDirIterator iter(m_rootPath, QStringList({ glob }), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        progress.setValue(i);
        QCoreApplication::processEvents();
        if (!iter.hasNext() && !glob.isEmpty()) {
            if (auto file_info = QFileInfo(FS::PathCombine(m_rootPath, glob)); file_info.exists()) {
                error |= copy(file_info.absoluteFilePath());
            } else {
                logUpdate(tr("El archivo no existe, ignorando: %1").arg(FS::PathCombine(m_rootPath, glob)));
            }
        } else {
            while (iter.hasNext()) {
                error |= copy(iter.next());
            }
        }
        i++;
    }
    progress.setValue(i);
    QCoreApplication::processEvents();

    if (error) {
        logUpdate(tr("Hubo errores al instalar la actualización."));
        auto fail_marker = FS::PathCombine(m_dataPath, ".beteliney_update.fail");
        FS::copy(m_updateLogPath, fail_marker).overwrite(true)();
    } else {
        logUpdate(tr("Actualización exitosa."));
        auto success_marker = FS::PathCombine(m_dataPath, ".beteliney_update.success");
        FS::copy(m_updateLogPath, success_marker).overwrite(true)();
    }
    auto update_lock_path = FS::PathCombine(m_dataPath, ".beteliney_update.lock");
    FS::deletePath(update_lock_path);

    QProcess proc;
    auto app_exe_name = BuildConfig.LAUNCHER_APP_BINARY_NAME;
#if defined Q_OS_WIN32
    app_exe_name.append(".exe");

    auto env = QProcessEnvironment::systemEnvironment();
    env.insert("__COMPAT_LAYER", "RUNASINVOKER");
    proc.setProcessEnvironment(env);
#else
    app_exe_name.prepend("bin/");
#endif

    auto app_exe_path = target.absoluteFilePath(app_exe_name);
    proc.startDetached(app_exe_path);

    exit(error ? 1 : 0);
}

void BetelineyUpdaterApp::printReleases()
{
    for (auto release : m_releases) {
        std::cout << release.name.toStdString() << " Version: " << release.tag_name.toStdString() << std::endl;
    }
}

QList<GitHubRelease> BetelineyUpdaterApp::nonDraftReleases()
{
    QList<GitHubRelease> nonDraft;
    for (auto rls : m_releases) {
        if (rls.isValid() && !rls.draft)
            nonDraft.append(rls);
    }
    return nonDraft;
}

QList<GitHubRelease> BetelineyUpdaterApp::newerReleases()
{
    QList<GitHubRelease> newer;
    for (auto rls : nonDraftReleases()) {
        if (rls.version > m_betelineyVersion)
            newer.append(rls);
    }
    return newer;
}

GitHubRelease BetelineyUpdaterApp::selectRelease()
{
    QList<GitHubRelease> releases;

    if (m_allowDowngrade) {
        releases = nonDraftReleases();
    } else {
        releases = newerReleases();
    }

    if (releases.isEmpty())
        return {};

    SelectReleaseDialog dlg(Version(m_betelineyVersion), releases);
    auto result = dlg.exec();

    if (result == QDialog::Rejected) {
        return {};
    }
    GitHubRelease release = dlg.selectedRelease();

    return release;
}

QList<GitHubReleaseAsset> BetelineyUpdaterApp::validReleaseArtifacts(const GitHubRelease& release)
{
    QList<GitHubReleaseAsset> valid;

    qDebug() << "Selecting best asset from" << release.tag_name << "for platform" << BuildConfig.BUILD_ARTIFACT
             << "portable:" << m_isPortable;
    if (BuildConfig.BUILD_ARTIFACT.isEmpty())
        qWarning() << "Build platform is not set!";
    for (auto asset : release.assets) {
        if (asset.name.endsWith(".zsync")) {
            qDebug() << "Rejecting zsync file" << asset.name;
            continue;
        }
        if (!m_isAppimage && asset.name.toLower().endsWith("appimage")) {
            qDebug() << "Rejecting" << asset.name << "because it is an AppImage";
            continue;
        } else if (m_isAppimage && !asset.name.toLower().endsWith("appimage")) {
            qDebug() << "Rejecting" << asset.name << "because it is not an AppImage";
            continue;
        }
        auto asset_name = asset.name.toLower();
        auto [platform, platform_qt_ver] = StringUtils::splitFirst(BuildConfig.BUILD_ARTIFACT.toLower(), "-qt");
        auto system_is_arm = QSysInfo::buildCpuArchitecture().contains("arm64");
        auto asset_is_arm = asset_name.contains("arm64");
        auto asset_is_archive = asset_name.endsWith(".zip") || asset_name.endsWith(".tar.gz");

        bool for_platform = !platform.isEmpty() && asset_name.contains(platform);
        if (!for_platform) {
            qDebug() << "Rejecting" << asset.name << "because platforms do not match";
        }
        bool for_portable = asset_name.contains("portable");
        if (for_platform && asset_name.contains("legacy") && !platform.contains("legacy")) {
            qDebug() << "Rejecting" << asset.name << "because platforms do not match";
            for_platform = false;
        }
        if (for_platform && ((asset_is_arm && !system_is_arm) || (!asset_is_arm && system_is_arm))) {
            qDebug() << "Rejecting" << asset.name << "because architecture does not match";
            for_platform = false;
        }
        if (for_platform && platform.contains("windows") && !m_isPortable && asset_is_archive) {
            qDebug() << "Rejecting" << asset.name << "because it is not an installer";
            for_platform = false;
        }

        static const QRegularExpression s_qtPattern("-qt(\\d+)");
        auto qt_match = s_qtPattern.match(asset_name);
        if (for_platform && qt_match.hasMatch()) {
            if (platform_qt_ver.isEmpty() || platform_qt_ver.toInt() != qt_match.captured(1).toInt()) {
                qDebug() << "Rejecting" << asset.name << "because it is not for the correct qt version" << platform_qt_ver.toInt() << "vs"
                         << qt_match.captured(1).toInt();
                for_platform = false;
            }
        }

        if (((m_isPortable && for_portable) || (!m_isPortable && !for_portable)) && for_platform) {
            qDebug() << "Accepting" << asset.name;
            valid.append(asset);
        }
    }
    return valid;
}

GitHubReleaseAsset BetelineyUpdaterApp::selectAsset(const QList<GitHubReleaseAsset>& assets)
{
    SelectReleaseAssetDialog dlg(assets);
    auto result = dlg.exec();

    if (result == QDialog::Rejected) {
        return {};
    }

    GitHubReleaseAsset asset = dlg.selectedAsset();
    return asset;
}

void BetelineyUpdaterApp::performUpdate(const GitHubRelease& release)
{
    m_install_release = release;
    qDebug() << "Updating to" << release.tag_name;
    auto valid_assets = validReleaseArtifacts(release);
    qDebug() << "valid release assets:" << valid_assets;

    GitHubReleaseAsset selected_asset;
    if (valid_assets.isEmpty()) {
        return showFatalErrorMessage(
            tr("Sin assets de release válidos"),
            tr("El release %1 de GitHub no tiene assets válidos para esta plataforma: %2")
                .arg(release.tag_name)
                .arg(tr("%1 portable: %2").arg(BuildConfig.BUILD_ARTIFACT).arg(m_isPortable ? tr("sí") : tr("no"))));
    } else if (valid_assets.length() > 1) {
        selected_asset = selectAsset(valid_assets);
    } else {
        selected_asset = valid_assets.takeFirst();
    }

    if (!selected_asset.isValid()) {
        return showFatalErrorMessage(tr("No se seleccionó versión."), tr("No se seleccionó ninguna versión."));
    }

    qDebug() << "will install" << selected_asset;
    auto file = downloadAsset(selected_asset);

    if (!file.exists()) {
        return showFatalErrorMessage(tr("Error al descargar"), tr("No se pudo descargar el asset seleccionado."));
    }

    performInstall(file);
}

QFileInfo BetelineyUpdaterApp::downloadAsset(const GitHubReleaseAsset& asset)
{
    auto temp_dir = QDir::tempPath();
    auto file_url = QUrl(asset.browser_download_url);
    auto out_file_path = FS::PathCombine(temp_dir, file_url.fileName());

    qDebug() << "downloading" << file_url << "to" << out_file_path;
    auto download = Net::Download::makeFile(file_url, out_file_path);
    download->setNetwork(m_network.get());
    auto progress_dialog = ProgressDialog();
    progress_dialog.adjustSize();

    progress_dialog.execWithTask(download.get());

    qDebug() << "download complete";

    QFileInfo out_file(out_file_path);
    return out_file;
}

bool BetelineyUpdaterApp::callAppImageUpdate()
{
    auto appimage_path = QProcessEnvironment::systemEnvironment().value(QStringLiteral("APPIMAGE"));
    QProcess proc = QProcess();
    qDebug() << "Calling: AppImageUpdate" << appimage_path;
    proc.setProgram(FS::PathCombine(m_rootPath, "bin", "AppImageUpdate.AppImage"));
    proc.setArguments({ appimage_path });
    auto result = proc.startDetached();
    if (!result)
        qDebug() << "Failed to start AppImageUpdate reason:" << proc.errorString();
    return result;
}

void BetelineyUpdaterApp::clearUpdateLog()
{
    FS::deletePath(m_updateLogPath);
}

void BetelineyUpdaterApp::logUpdate(const QString& msg)
{
    qDebug() << qUtf8Printable(msg);
    FS::append(m_updateLogPath, QStringLiteral("%1\n").arg(msg).toUtf8());
}

std::tuple<QDateTime, QString, QString, QString, QString> read_lock_File(const QString& path)
{
    auto contents = QString(FS::read(path));
    auto lines = contents.split('\n');

    QDateTime timestamp;
    QString from, to, target, data_path;
    for (auto line : lines) {
        auto index = line.indexOf("=");
        if (index < 0)
            continue;
        auto left = line.left(index);
        auto right = line.mid(index + 1);
        if (left.toLower() == "timestamp") {
            timestamp = QDateTime::fromString(right, Qt::ISODate);
        } else if (left.toLower() == "from") {
            from = right;
        } else if (left.toLower() == "to") {
            to = right;
        } else if (left.toLower() == "target") {
            target = right;
        } else if (left.toLower() == "data_path") {
            data_path = right;
        }
    }
    return std::make_tuple(timestamp, from, to, target, data_path);
}

bool write_lock_file(const QString& path, QDateTime timestamp, QString from, QString to, QString target, QString data_path)
{
    try {
        FS::write(path, QStringLiteral("TIMESTAMP=%1\nFROM=%2\nTO=%3\nTARGET=%4\nDATA_PATH=%5\n")
                            .arg(timestamp.toString(Qt::ISODate))
                            .arg(from)
                            .arg(to)
                            .arg(target)
                            .arg(data_path)
                            .toUtf8());
    } catch (FS::FileSystemException& err) {
        qWarning() << "Error writing lockfile:" << err.what() << "\n" << err.cause();
        return false;
    }
    return true;
}

void BetelineyUpdaterApp::performInstall(QFileInfo file)
{
    qDebug() << "starting install";
    auto update_lock_path = FS::PathCombine(m_dataPath, ".beteliney_update.lock");
    QFileInfo update_lock(update_lock_path);
    if (update_lock.exists()) {
        auto [timestamp, from, to, target, data_path] = read_lock_File(update_lock_path);
        auto msg = tr("Ya hay una actualización en progreso\n");
        auto infoMsg =
            tr("Esta instalación tiene un archivo de bloqueo de actualización presente en: %1\n"
               "\n"
               "Marca de tiempo: %2\n"
               "Actualizando de la versión %3 a %4\n"
               "Ruta de instalación destino: %5\n"
               "Ruta de datos: %6"
               "\n"
               "Esto probablemente significa que un intento de actualización anterior falló. Asegúrate de que tu instalación esté en buen estado antes de "
               "continuar.\n"
               "Revisa el log del actualizador de Beteliney Launcher en: \n"
               "%7\n"
               "para más detalles sobre el último intento de actualización.\n"
               "\n"
               "Para sobrescribir este bloqueo y continuar con esta actualización de todas formas, selecciona \"Ignorar\" abajo.")
                .arg(update_lock_path)
                .arg(timestamp.toString(Qt::ISODate), from, to, target, data_path)
                .arg(m_updateLogPath);
        QMessageBox msgBox;
        msgBox.setText(msg);
        msgBox.setInformativeText(infoMsg);
        msgBox.setStandardButtons(QMessageBox::Ignore | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setMinimumWidth(460);
        msgBox.adjustSize();
        switch (msgBox.exec()) {
            case QMessageBox::AcceptRole:
                break;
            case QMessageBox::RejectRole:
                [[fallthrough]];
            default:
                return showFatalErrorMessage(tr("Actualización cancelada"), tr("El intento de actualización fue cancelado"));
        }
    }
    clearUpdateLog();

    auto changelog_path = FS::PathCombine(m_dataPath, ".beteliney_update.changelog");
    FS::write(changelog_path, m_install_release.body.toUtf8());

    logUpdate(tr("Actualizando de %1 a %2").arg(m_betelineyVersion).arg(m_install_release.tag_name));
    if (m_isPortable || file.fileName().endsWith(".zip") || file.fileName().endsWith(".tar.gz")) {
        write_lock_file(update_lock_path, QDateTime::currentDateTime(), m_betelineyVersion, m_install_release.tag_name, m_rootPath, m_dataPath);
        logUpdate(tr("Actualizando instalación portátil en %1").arg(m_rootPath));
        unpackAndInstall(file);
    } else {
        logUpdate(tr("Ejecutando archivo instalador en %1").arg(file.absoluteFilePath()));
        QProcess proc = QProcess();
#if defined Q_OS_WIN
        auto env = QProcessEnvironment::systemEnvironment();
        env.insert("__COMPAT_LAYER", "RUNASINVOKER");
        proc.setProcessEnvironment(env);
#endif
        proc.setProgram(file.absoluteFilePath());
        bool result = proc.startDetached();
        logUpdate(tr("Resultado del inicio del proceso: %1").arg(result ? tr("sí") : tr("no")));
        exit(result ? 0 : 1);
    }
}

void BetelineyUpdaterApp::unpackAndInstall(QFileInfo archive)
{
    logUpdate(tr("Respaldando instalación"));
    backupAppDir();

    if (auto loc = unpackArchive(archive)) {
        auto marker_file_path = loc.value().absoluteFilePath(".beteliney_updater_unpack.marker");
        FS::write(marker_file_path, m_rootPath.toUtf8());

        QProcess proc = QProcess();

        auto exe_name = QStringLiteral("%1_updater").arg(BuildConfig.LAUNCHER_APP_BINARY_NAME);
#if defined Q_OS_WIN32
        exe_name.append(".exe");

        auto env = QProcessEnvironment::systemEnvironment();
        env.insert("__COMPAT_LAYER", "RUNASINVOKER");
        proc.setProcessEnvironment(env);
#else
        exe_name.prepend("bin/");
#endif

        auto new_updater_path = loc.value().absoluteFilePath(exe_name);
        logUpdate(tr("Iniciando el nuevo actualizador en '%1'").arg(new_updater_path));
        if (!proc.startDetached(new_updater_path, { "-d", m_dataPath }, loc.value().absolutePath())) {
            logUpdate(tr("No se pudo iniciar '%1' %2").arg(new_updater_path).arg(proc.errorString()));
            return exit(10);
        }
        return exit();  // up to the new updater now
    }
    return exit(1);  // unpack failure
}

void BetelineyUpdaterApp::backupAppDir()
{
    auto manifest_path = FS::PathCombine(m_rootPath, "manifest.txt");
    QFileInfo manifest(manifest_path);

    QStringList file_list;
    if (manifest.isFile()) {
        // load manifest from file

        logUpdate(tr("Leyendo manifiesto desde %1").arg(manifest.absoluteFilePath()));
        try {
            auto contents = QString::fromUtf8(FS::read(manifest.absoluteFilePath()));
            auto files = contents.split('\n');
            for (auto file : files) {
                file_list.append(file.trimmed());
            }
        } catch (FS::FileSystemException&) {
        }
    }

    if (file_list.isEmpty()) {
        // best guess
        if (BuildConfig.BUILD_ARTIFACT.toLower().contains("linux")) {
            file_list.append({ "BetelineyLauncher", "bin", "share", "lib" });
        } else {  // windows by process of elimination
            file_list.append({
                "jars",
                "beteliney.exe",
                "beteliney_filelink.exe",
                "beteliney_updater.exe",
                "qtlogging.ini",
                "imageformats",
                "iconengines",
                "platforms",
                "styles",
                "tls",
                "qt.conf",
                "Qt*.dll",
            });
        }
        logUpdate(tr("manifest.txt vacío o ausente. Adivinando los archivos a respaldar."));
    }
    logUpdate(tr("Respaldando:\n  %1").arg(file_list.join(",\n  ")));
    static const QRegularExpression s_replaceRegex("[" + QRegularExpression::escape("\\/:*?\"<>|") + "]");
    auto app_dir = QDir(m_rootPath);
    auto backup_dir =
        FS::PathCombine(app_dir.absolutePath(),
                        QStringLiteral("backup_") + QString(m_betelineyVersion).replace(s_replaceRegex, QString("_")) + "-" + m_betelineyGitCommit);
    FS::ensureFolderPathExists(backup_dir);
    auto backup_marker_path = FS::PathCombine(m_dataPath, ".beteliney_update_backup_path.txt");
    FS::write(backup_marker_path, backup_dir.toUtf8());

    QProgressDialog progress(tr("Respaldando instalación en %1").arg(m_rootPath), "", 0, file_list.length());
    progress.setCancelButton(nullptr);
    progress.setMinimumWidth(400);
    progress.adjustSize();
    progress.show();
    QCoreApplication::processEvents();

    logUpdate(tr("Respaldando instalación en %1").arg(m_rootPath));

    auto copy = [this, app_dir, backup_dir](QString to_bak_file) {
        auto rel_path = app_dir.relativeFilePath(to_bak_file);
        auto bak_path = FS::PathCombine(backup_dir, rel_path);
        logUpdate(tr("Respaldando y luego eliminando %1").arg(to_bak_file));
        FS::ensureFilePathExists(bak_path);
        auto result = FS::copy(to_bak_file, bak_path).overwrite(true)();
        if (!result) {
            logUpdate(tr("Falló el respaldo de %1 a %2").arg(to_bak_file).arg(bak_path));
        } else {
            if (!FS::deletePath(to_bak_file))
                logUpdate(tr("No se pudo eliminar %1").arg(to_bak_file));
        }
    };

    int i = 0;
    for (auto glob : file_list) {
        QDirIterator iter(app_dir.absolutePath(), QStringList({ glob }), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        progress.setValue(i);
        QCoreApplication::processEvents();
        if (!iter.hasNext() && !glob.isEmpty()) {
            if (auto file_info = QFileInfo(FS::PathCombine(app_dir.absolutePath(), glob)); file_info.exists()) {
                copy(file_info.absoluteFilePath());
            } else {
                logUpdate(tr("El archivo no existe, ignorando: %1").arg(FS::PathCombine(app_dir.absolutePath(), glob)));
            }
        } else {
            while (iter.hasNext()) {
                copy(iter.next());
            }
        }
        i++;
    }
    progress.setValue(i);
    QCoreApplication::processEvents();
}

std::optional<QDir> BetelineyUpdaterApp::unpackArchive(QFileInfo archive)
{
    auto temp_extract_path = FS::PathCombine(m_dataPath, "beteliney_update_release");
    FS::ensureFolderPathExists(temp_extract_path);
    auto tmp_extract_dir = QDir(temp_extract_path);

    auto result = BetelineyZip::extractDir(archive.absoluteFilePath(), tmp_extract_dir.absolutePath());
    if (result) {
        logUpdate(tr("Se extrajo lo siguiente a \"%1\":\n  %2").arg(tmp_extract_dir.absolutePath()).arg(result->join("\n  ")));
    } else {
        logUpdate(tr("No se pudo extraer %1 a %2").arg(archive.absoluteFilePath()).arg(tmp_extract_dir.absolutePath()));
        showFatalErrorMessage(tr("Error al extraer el archivo"),
                              tr("No se pudo extraer %1 a %2").arg(archive.absoluteFilePath()).arg(tmp_extract_dir.absolutePath()));
        return std::nullopt;
    }

    return tmp_extract_dir;
}

bool BetelineyUpdaterApp::loadBetelineyVersionFromExe(const QString& exe_path)
{
    QProcess proc = QProcess();
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.setReadChannel(QProcess::StandardOutput);
    proc.start(exe_path, { "--version" });
    if (!proc.waitForStarted(5000)) {
        showFatalErrorMessage(tr("Error al verificar versión"), tr("No se pudo iniciar el proceso hijo para leer la versión."));
        return false;
    }  // wait 5 seconds to start
    if (!proc.waitForFinished(5000)) {
        showFatalErrorMessage(tr("Error al verificar versión"), tr("El proceso hijo del launcher falló."));
        return false;
    }
    auto out = proc.readAllStandardOutput();
    auto lines = out.split('\n');
    lines.removeAll("");
    if (lines.length() < 2)
        return false;
    else if (lines.length() > 2) {
        auto line1 = lines.takeLast();
        auto line2 = lines.takeLast();
        lines = { line2, line1 };
    }
    auto first = lines.takeFirst();
    auto first_parts = first.split(' ');
    if (first_parts.length() < 2)
        return false;
    m_betelineyBinaryName = first_parts.takeFirst();
    auto version = first_parts.takeFirst().trimmed();
    m_betelineyVersion = version;
    if (version.contains('-')) {
        auto index = version.indexOf('-');
        m_betelineyVersionChannel = version.mid(index + 1);
        version = version.left(index);
    } else {
        m_betelineyVersionChannel = "stable";
    }
    auto version_parts = version.split('.');
    if (version_parts.length() < 2)
        return false;
    m_betelineyVersionMajor = version_parts.takeFirst().toInt();
    m_betelineyVersionMinor = version_parts.takeFirst().toInt();
    if (!version_parts.isEmpty())
        m_betelineyVersionPatch = version_parts.takeFirst().toInt();
    else
        m_betelineyVersionPatch = 0;
    m_betelineyGitCommit = lines.takeFirst().simplified();
    return true;
}

void BetelineyUpdaterApp::loadReleaseList()
{
    auto github_repo = m_betelineyRepoUrl;
    if (github_repo.host() != "github.com")
        return fail("updating from a non github url is not supported");

    auto path_parts = github_repo.path().split('/');
    path_parts.removeFirst();  // empty segment from leading /
    auto repo_owner = path_parts.takeFirst();
    auto repo_name = path_parts.takeFirst();
    auto api_url = QString("https://api.github.com/repos/%1/%2/releases").arg(repo_owner, repo_name);

    qDebug() << "Fetching release list from" << api_url;

    downloadReleasePage(api_url, 1);
}

void BetelineyUpdaterApp::downloadReleasePage(const QString& api_url, int page)
{
    int per_page = 30;
    auto page_url = QString("%1?per_page=%2&page=%3").arg(api_url).arg(QString::number(per_page)).arg(QString::number(page));
    auto [download, response] = Net::Download::makeByteArray(page_url);
    download->setNetwork(m_network.get());
    m_current_url = page_url;

    auto github_api_headers = std::make_unique<Net::RawHeaderProxy>();
    github_api_headers->addHeaders({
        { "Accept", "application/vnd.github+json" },
        { "X-GitHub-Api-Version", "2022-11-28" },
    });
    download->addHeaderProxy(std::move(github_api_headers));

    connect(download.get(), &Net::Download::succeeded, this, [this, response, per_page, api_url, page]() {
        int num_found = parseReleasePage(response);
        if (!(num_found < per_page)) {  // there may be more, fetch next page
            downloadReleasePage(api_url, page + 1);
        } else {
            run();
        }
    });
    connect(download.get(), &Net::Download::failed, this, &BetelineyUpdaterApp::downloadError);

    m_current_task.reset(download);
    connect(download.get(), &Net::Download::finished, this, [this]() {
        qDebug() << "Download" << m_current_task->getUid().toString() << "finished";
        m_current_task.reset();
        m_current_url = "";
    });

    QCoreApplication::processEvents();

    QMetaObject::invokeMethod(download.get(), &Task::start, Qt::QueuedConnection);
}

int BetelineyUpdaterApp::parseReleasePage(const QByteArray* response)
{
    if (response->isEmpty())  // empty page
        return 0;
    int num_releases = 0;
    try {
        auto doc = Json::requireDocument(*response);
        auto release_list = Json::requireArray(doc);
        for (auto release_json : release_list) {
            auto release_obj = Json::requireObject(release_json);

            GitHubRelease release = {};
            release.id = Json::requireInteger(release_obj, "id");
            release.name = release_obj["name"].toString();
            release.tag_name = Json::requireString(release_obj, "tag_name");
            release.created_at = QDateTime::fromString(Json::requireString(release_obj, "created_at"), Qt::ISODate);
            release.published_at = QDateTime::fromString(release_obj["published_at"].toString(), Qt::ISODate);
            release.draft = Json::requireBoolean(release_obj, "draft");
            release.prerelease = Json::requireBoolean(release_obj, "prerelease");
            release.body = release_obj["body"].toString();
            release.version = Version(release.tag_name);

            auto release_assets_obj = Json::requireArray(release_obj, "assets");
            for (auto asset_json : release_assets_obj) {
                auto asset_obj = Json::requireObject(asset_json);
                GitHubReleaseAsset asset = {};
                asset.id = Json::requireInteger(asset_obj, "id");
                asset.name = Json::requireString(asset_obj, "name");
                asset.label = asset_obj["label"].toString();
                asset.content_type = Json::requireString(asset_obj, "content_type");
                asset.size = Json::requireInteger(asset_obj, "size");
                asset.created_at = QDateTime::fromString(Json::requireString(asset_obj, "created_at"), Qt::ISODate);
                asset.updated_at = QDateTime::fromString(Json::requireString(asset_obj, "updated_at"), Qt::ISODate);
                asset.browser_download_url = Json::requireString(asset_obj, "browser_download_url");
                release.assets.append(asset);
            }
            m_releases.append(release);
            num_releases++;
        }
    } catch (Json::JsonException& e) {
        auto err_msg =
            QString("Failed to parse releases from github: %1\n%2").arg(e.what()).arg(QString::fromStdString(response->toStdString()));
        fail(err_msg);
    }
    return num_releases;
}

GitHubRelease BetelineyUpdaterApp::getLatestRelease()
{
    GitHubRelease latest;
    for (auto release : m_releases) {
        if (release.draft)
            continue;
        if (release.prerelease && !m_allowPreRelease)
            continue;
        if (!latest.isValid() || (release.version > latest.version)) {
            latest = release;
        }
    }
    return latest;
}

bool BetelineyUpdaterApp::needUpdate(const GitHubRelease& release)
{
    auto current_ver = Version(QString("%1.%2.%3").arg(m_betelineyVersionMajor).arg(m_betelineyVersionMinor).arg(m_betelineyVersionPatch));
    return current_ver < release.version;
}

void BetelineyUpdaterApp::downloadError(QString reason)
{
    fail(QString("Network request Failed: %1 with reason %2").arg(m_current_url).arg(reason));
}
