// SPDX-FileCopyrightText: 2023 Rachel Powers <508861+Ryex@users.noreply.github.com>
//
// SPDX-License-Identifier: GPL-3.0-only

/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *  Copyright (C) 2023 Rachel Powers <508861+Ryex@users.noreply.github.com>
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

#include "BetelineyExternalUpdater.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QSettings>
#include <QTimer>
#include <QThread>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <memory>

#include "StringUtils.h"

#include "BuildConfig.h"

#include "ui/dialogs/UpdateAvailableDialog.h"

class BetelineyExternalUpdater::Private {
   public:
    QDir appDir;
    QDir dataDir;
    QTimer updateTimer;
    bool allowBeta;
    bool autoCheck;
    double updateInterval;
    QDateTime lastCheck;
    std::unique_ptr<QSettings> settings;

    QWidget* parent;

    // BUG 1 fix: guardia contra doble llamada simultánea
    bool m_checking = false;
    // Resultado del proceso de chequeo de actualizaciones
    struct UpdateResult {
        bool started = false;
        bool finished = false;
        int exitCode = -1;
        QByteArray stdOut;
        QByteArray stdErr;
        QString errorString;
    };
    // BUG 2 fix: guardar el future para poder esperar en el destructor
    QFuture<UpdateResult> m_currentFuture;
    // Para cancelar el proceso de chequeo desde el botón Cancel del progress dialog
    std::atomic<bool> m_cancelRequested { false };
};

BetelineyExternalUpdater::BetelineyExternalUpdater(QWidget* parent, const QString& appDir, const QString& dataDir)
{
    priv = new BetelineyExternalUpdater::Private();
    priv->appDir = QDir(appDir);
    priv->dataDir = QDir(dataDir);
    auto settings_file = priv->dataDir.absoluteFilePath("beteliney_update.cfg");
    priv->settings = std::make_unique<QSettings>(settings_file, QSettings::Format::IniFormat);
    priv->allowBeta = priv->settings->value("allow_beta", false).toBool();
    priv->autoCheck = priv->settings->value("auto_check", false).toBool();
    bool interval_ok;
    // default once per day
    priv->updateInterval = priv->settings->value("update_interval", 86400).toInt(&interval_ok);
    if (!interval_ok)
        priv->updateInterval = 86400;
    auto last_check = priv->settings->value("last_check");
    if (!last_check.isNull() && last_check.isValid()) {
        priv->lastCheck = QDateTime::fromString(last_check.toString(), Qt::ISODate);
    }
    priv->parent = parent;
    connectTimer();
    resetAutoCheckTimer();
}

BetelineyExternalUpdater::~BetelineyExternalUpdater()
{
    if (priv->updateTimer.isActive())
        priv->updateTimer.stop();
    disconnectTimer();
    // BUG 2 fix: esperar que el hilo de chequeo termine antes de destruir
    if (priv->m_currentFuture.isRunning())
        priv->m_currentFuture.waitForFinished();
    priv->settings->sync();
    delete priv;
}

void BetelineyExternalUpdater::checkForUpdates()
{
    checkForUpdates(true);
}

void BetelineyExternalUpdater::checkForUpdates(bool triggeredByUser)
{
    // BUG 1 fix: evitar dos chequeos simultáneos
    if (priv->m_checking)
        return;
    priv->m_checking = true;

    // Mostrar diálogo de progreso con botón Cancel funcional
    priv->m_cancelRequested.store(false);
    auto* progress = new QProgressDialog(tr("Checking for updates..."), tr("Cancel"), 0, 0, priv->parent);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(500);
    progress->adjustSize();
    // Conectar Cancel: señala al hilo que debe abortar el proceso
    connect(progress, &QProgressDialog::canceled, this, [this]() {
        priv->m_cancelRequested.store(true);
    });

    QString exe_name = QStringLiteral("%1_updater").arg(BuildConfig.LAUNCHER_APP_BINARY_NAME);
    QString exePath;
#if defined Q_OS_WIN32
    exe_name.append(".exe");
    exePath = priv->appDir.absoluteFilePath(exe_name);
#else
    exePath = priv->appDir.absoluteFilePath(QString("bin/%1").arg(exe_name));
#endif

    QStringList args = { "--check-only", "--dir", priv->dataDir.absolutePath(), "--debug" };
    if (priv->allowBeta)
        args.append("--pre-release");

    auto* watcher = new QFutureWatcher<Private::UpdateResult>(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
#if defined Q_OS_WIN32
    env.insert("__COMPAT_LAYER", "RUNASINVOKER");
#endif
    QString capturedExePath = exePath;
    QStringList capturedArgs = args;

    std::atomic<bool>& cancelRef = priv->m_cancelRequested;
    QFuture<Private::UpdateResult> future = QtConcurrent::run([capturedExePath, capturedArgs, env, &cancelRef]() -> Private::UpdateResult {
        Private::UpdateResult result;
        QProcess proc;
        proc.setProcessEnvironment(env);
        proc.start(capturedExePath, capturedArgs);

        result.started = proc.waitForStarted(5000);
        if (!result.started) {
            result.errorString = proc.errorString();
            return result;
        }

        // Esperar en chunks de 200ms para poder detectar cancelación
        result.finished = false;
        for (int elapsed = 0; elapsed < 60000; elapsed += 200) {
            if (cancelRef.load()) {
                proc.kill();
                result.errorString = "cancelled";
                return result;
            }
            if (proc.waitForFinished(200)) {
                result.finished = true;
                break;
            }
        }
        if (!result.finished) {
            proc.kill();
            result.errorString = proc.errorString();
            result.stdOut = proc.readAll();
            return result;
        }

        result.exitCode = proc.exitCode();
        result.stdOut = proc.readAllStandardOutput();
        result.stdErr = proc.readAllStandardError();
        return result;
    });

    // setFuture va DESPUÉS del connect para evitar la race condition
    // donde el future termina antes de que la señal esté conectada
    connect(watcher, &QFutureWatcher<Private::UpdateResult>::finished, this,
            [this, watcher, progress, triggeredByUser]() {
        progress->hide();
        progress->deleteLater();
        watcher->deleteLater();
        // BUG 1 fix: liberar guardia al terminar
        priv->m_checking = false;

        auto result = watcher->result();

        // Si el usuario canceló, cerrar silenciosamente
        if (result.errorString == "cancelled") {
            priv->lastCheck = QDateTime::currentDateTime();
            priv->settings->setValue("last_check", priv->lastCheck.toString(Qt::ISODate));
            priv->settings->sync();
            resetAutoCheckTimer();
            return;
        }

        if (!result.started) {
            qDebug() << "Failed to start updater after 5 seconds. reason:" << result.errorString;
            auto msgBox = QMessageBox(QMessageBox::Information, tr("Update Check Failed"),
                                     tr("Failed to start after 5 seconds\nReason: %1.").arg(result.errorString),
                                     QMessageBox::Ok, priv->parent);
            msgBox.setMinimumWidth(460);
            msgBox.adjustSize();
            msgBox.exec();
            priv->lastCheck = QDateTime::currentDateTime();
            priv->settings->setValue("last_check", priv->lastCheck.toString(Qt::ISODate));
            priv->settings->sync();
            resetAutoCheckTimer();
            return;
        }

        if (!result.finished) {
            qDebug() << "Updater failed to close after 60 seconds. reason:" << result.errorString;
            auto msgBox = QMessageBox(QMessageBox::Information, tr("Update Check Failed"),
                                     tr("Updater failed to close 60 seconds\nReason: %1.").arg(result.errorString),
                                     QMessageBox::Ok, priv->parent);
            msgBox.setDetailedText(result.stdOut);
            msgBox.setMinimumWidth(460);
            msgBox.adjustSize();
            msgBox.exec();
            priv->lastCheck = QDateTime::currentDateTime();
            priv->settings->setValue("last_check", priv->lastCheck.toString(Qt::ISODate));
            priv->settings->sync();
            resetAutoCheckTimer();
            return;
        }

        auto std_output = result.stdOut;
        auto std_error = result.stdErr;

        switch (result.exitCode) {
            case 0:
                if (triggeredByUser) {
                    qDebug() << "No update available";
                    auto msgBox = QMessageBox(QMessageBox::Information, tr("No Update Available"),
                                             tr("You are running the latest version."),
                                             QMessageBox::Ok, priv->parent);
                    msgBox.setMinimumWidth(460);
                    msgBox.adjustSize();
                    msgBox.exec();
                }
                break;
            case 1:
                {
                    qDebug() << "Updater subprocess error" << qPrintable(std_error);
                    auto msgBox = QMessageBox(QMessageBox::Warning, tr("Update Check Error"),
                                             tr("There was an error running the update check."),
                                             QMessageBox::Ok, priv->parent);
                    msgBox.setDetailedText(QString(std_error));
                    msgBox.setMinimumWidth(460);
                    msgBox.adjustSize();
                    msgBox.exec();
                }
                break;
            case 100:
                {
                    auto [first_line, remainder1] = StringUtils::splitFirst(std_output, '\n');
                    auto [second_line, remainder2] = StringUtils::splitFirst(remainder1, '\n');
                    auto [third_line, release_notes] = StringUtils::splitFirst(remainder2, '\n');
                    auto version_name = StringUtils::splitFirst(first_line, ": ").second.trimmed();
                    auto version_tag = StringUtils::splitFirst(second_line, ": ").second.trimmed();
                    qDebug() << "Update available:" << version_name << version_tag;
                    qDebug() << "Update release notes:" << release_notes;
                    offerUpdate(version_name, version_tag, release_notes);
                }
                break;
            default:
                {
                    qDebug() << "Updater exited with unknown code" << result.exitCode;
                    auto msgBox = QMessageBox(QMessageBox::Information, tr("Unknown Update Error"),
                                             tr("The updater exited with an unknown condition.\nExit Code: %1")
                                                 .arg(QString::number(result.exitCode)),
                                             QMessageBox::Ok, priv->parent);
                    auto detail_txt = tr("StdOut: %1\nStdErr: %2").arg(QString(std_output)).arg(QString(std_error));
                    msgBox.setDetailedText(detail_txt);
                    msgBox.setMinimumWidth(460);
                    msgBox.adjustSize();
                    msgBox.exec();
                }
        }

        priv->lastCheck = QDateTime::currentDateTime();
        priv->settings->setValue("last_check", priv->lastCheck.toString(Qt::ISODate));
        priv->settings->sync();
        resetAutoCheckTimer();
    });

    watcher->setFuture(future);
    // BUG 2 fix: guardar el future para poder esperar en el destructor
    priv->m_currentFuture = future;
}

bool BetelineyExternalUpdater::getAutomaticallyChecksForUpdates()
{
    return priv->autoCheck;
}

double BetelineyExternalUpdater::getUpdateCheckInterval()
{
    return priv->updateInterval;
}

bool BetelineyExternalUpdater::getBetaAllowed()
{
    return priv->allowBeta;
}

void BetelineyExternalUpdater::setAutomaticallyChecksForUpdates(bool check)
{
    priv->autoCheck = check;
    priv->settings->setValue("auto_check", check);
    priv->settings->sync();
    resetAutoCheckTimer();
}

void BetelineyExternalUpdater::setUpdateCheckInterval(double seconds)
{
    priv->updateInterval = seconds;
    priv->settings->setValue("update_interval", seconds);
    priv->settings->sync();
    resetAutoCheckTimer();
}

void BetelineyExternalUpdater::setBetaAllowed(bool allowed)
{
    priv->allowBeta = allowed;
    priv->settings->setValue("allow_beta", allowed);
    priv->settings->sync();
}

void BetelineyExternalUpdater::resetAutoCheckTimer()
{
    if (priv->autoCheck && priv->updateInterval > 0) {
        int timeoutDuration = 0;
        auto now = QDateTime::currentDateTime();
        if (priv->lastCheck.isValid()) {
            auto diff = priv->lastCheck.secsTo(now);
            auto secs_left = priv->updateInterval - diff;
            if (secs_left < 0)
                secs_left = 0;
            timeoutDuration = secs_left * 1000;  // to msec
        }
        qDebug() << "Auto update timer starting," << timeoutDuration / 1000 << "seconds left";
        priv->updateTimer.start(timeoutDuration);
    } else {
        if (priv->updateTimer.isActive())
            priv->updateTimer.stop();
    }
}

void BetelineyExternalUpdater::connectTimer()
{
    connect(&priv->updateTimer, &QTimer::timeout, this, &BetelineyExternalUpdater::autoCheckTimerFired);
}

void BetelineyExternalUpdater::disconnectTimer()
{
    disconnect(&priv->updateTimer, &QTimer::timeout, this, &BetelineyExternalUpdater::autoCheckTimerFired);
}

void BetelineyExternalUpdater::autoCheckTimerFired()
{
    qDebug() << "Auto update Timer fired";
    checkForUpdates(false);
}

void BetelineyExternalUpdater::offerUpdate(const QString& version_name, const QString& version_tag, const QString& release_notes)
{
    priv->settings->beginGroup("skip");
    auto should_skip = priv->settings->value(version_tag, false).toBool();
    priv->settings->endGroup();

    if (should_skip) {
        auto msgBox = QMessageBox(QMessageBox::Information, tr("No Update Available"), tr("There are no new updates available."),
                                  QMessageBox::Ok, priv->parent);
        msgBox.setMinimumWidth(460);
        msgBox.adjustSize();
        msgBox.exec();
        return;
    }

    UpdateAvailableDialog dlg(BuildConfig.printableVersionString(), version_name, release_notes);

    auto result = dlg.exec();
    qDebug() << "offer dlg result" << result;
    switch (result) {
        case UpdateAvailableDialog::Install: {
            performUpdate(version_tag);
            return;
        }
        case UpdateAvailableDialog::Skip: {
            priv->settings->beginGroup("skip");
            priv->settings->setValue(version_tag, true);
            priv->settings->endGroup();
            priv->settings->sync();
            return;
        }
        case UpdateAvailableDialog::DontInstall: {
            return;
        }
    }
}

void BetelineyExternalUpdater::performUpdate(const QString& version_tag)
{
    QProcess proc;
    auto exe_name = QStringLiteral("%1_updater").arg(BuildConfig.LAUNCHER_APP_BINARY_NAME);
#if defined Q_OS_WIN32
    exe_name.append(".exe");

    auto env = QProcessEnvironment::systemEnvironment();
    env.insert("__COMPAT_LAYER", "RUNASINVOKER");
    proc.setProcessEnvironment(env);
#else
    exe_name = QString("bin/%1").arg(exe_name);
#endif

    QStringList args = { "--dir", priv->dataDir.absolutePath(), "--install-version", version_tag };
    if (priv->allowBeta)
        args.append("--pre-release");

    // BUG 3 fix: verificar que el exe existe antes de cerrar la app
    QString exeFullPath = priv->appDir.absoluteFilePath(exe_name);
    if (!QFileInfo::exists(exeFullPath)) {
        qCritical() << "performUpdate: updater exe not found:" << exeFullPath;
        QMessageBox::critical(priv->parent, tr("Update Failed"),
                              tr("The updater executable was not found:\n%1\n\nCannot apply update.").arg(exeFullPath));
        return;
    }

    auto result = proc.startDetached(exeFullPath, args);
    if (!result) {
        qDebug() << "Failed to start updater:" << proc.error() << proc.errorString();
    }
    QCoreApplication::exit();
}
