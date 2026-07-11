// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
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
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "LauncherPartLaunch.h"

#include <QRegularExpression>
#include <QStandardPaths>

#include "Application.h"
#include "BubblewrapSandbox.h"
#include "Commandline.h"
#include "FileSystem.h"
#include "launch/LaunchTask.h"
#include "minecraft/MinecraftInstance.h"

#ifdef Q_OS_LINUX
#if __has_include("gamemode_client.h")
#include "gamemode_client.h"
#define BETELINEY_HAVE_GAMEMODE 1
#endif
#endif

LauncherPartLaunch::LauncherPartLaunch(LaunchTask* parent)
    : LaunchStep(parent)
    , m_process(parent->instance()->getJavaVersion().defaultsToUtf8() ? QStringConverter::Utf8 : QStringConverter::System)
{
    if (parent->instance()->settings()->get("CloseAfterLaunch").toBool()) {
        static const QRegularExpression s_settingUser(".*Setting user.+", QRegularExpression::CaseInsensitiveOption);
        std::shared_ptr<QMetaObject::Connection> connection{ new QMetaObject::Connection };
        *connection =
            connect(&m_process, &LoggedProcess::log, this, [connection](const QStringList& lines, [[maybe_unused]] MessageLevel level) {
                qDebug() << lines;
                if (lines.filter(s_settingUser).length() != 0) {
                    APPLICATION->closeAllWindows();
                    disconnect(*connection);
                }
            });
    }

    connect(&m_process, &LoggedProcess::log, this, &LauncherPartLaunch::logLines);
    connect(&m_process, &LoggedProcess::stateChanged, this, &LauncherPartLaunch::on_state);
}

void LauncherPartLaunch::executeTask()
{
    QString jarPath = APPLICATION->getJarPath("NewLaunch.jar");
    if (jarPath.isEmpty()) {
        const char* reason = QT_TR_NOOP("Launcher library could not be found. Please check your installation.");
        emit logLine(tr(reason), MessageLevel::Fatal);
        emitFailed(tr(reason));
        return;
    }

    auto instance = m_parent->instance();

    QString legacyJarPath;
    if (instance->getLauncher() == "legacy" || instance->shouldApplyOnlineFixes()) {
        legacyJarPath = APPLICATION->getJarPath("NewLaunchLegacy.jar");
        if (legacyJarPath.isEmpty()) {
            const char* reason = QT_TR_NOOP("Legacy launcher library could not be found. Please check your installation.");
            emit logLine(tr(reason), MessageLevel::Fatal);
            emitFailed(tr(reason));
            return;
        }
    }

    m_launchScript = instance->createLaunchScript(m_session, m_targetToJoin);
    QStringList args = instance->javaArguments();
    QString allArgs = args.join(", ");
    emit logLine("Java Arguments:\n[" + m_parent->censorPrivateInfo(allArgs) + "]\n\n", MessageLevel::Launcher);

    auto javaPath = FS::ResolveExecutable(instance->settings()->get("JavaPath").toString());

    m_process.setProcessEnvironment(instance->createLaunchEnvironment());

    // make detachable - this will keep the process running even if the object is destroyed
    m_process.setDetachable(true);

    auto classPath = instance->getClassPath();
    classPath.prepend(jarPath);

    if (!legacyJarPath.isEmpty())
        classPath.prepend(legacyJarPath);

    auto natPath = instance->getNativePath();
#ifdef Q_OS_WIN
    natPath = FS::getPathNameInLocal8bit(natPath);
#endif
    args << "-Djava.library.path=" + natPath;

    args << "-cp";
#ifdef Q_OS_WIN
    QStringList processed;
    for (auto& item : classPath) {
        processed << FS::getPathNameInLocal8bit(item);
    }
    args << processed.join(';');
#else
    args << classPath.join(':');
#endif
    args << "com.beteliney.launcher.EntryPoint";

    qDebug() << args.join(' ');

    QString wrapperCommandStr = instance->getWrapperCommand().trimmed();

    QString launchCommand;
    QStringList launchArgs;
    if (!wrapperCommandStr.isEmpty()) {
        wrapperCommandStr = m_parent->substituteVariables(wrapperCommandStr);
        auto wrapperArgs = Commandline::splitArgs(wrapperCommandStr);
        auto wrapperCommand = wrapperArgs.takeFirst();
        auto realWrapperCommand = QStandardPaths::findExecutable(wrapperCommand);
        if (realWrapperCommand.isEmpty()) {
            const char* reason = QT_TR_NOOP("The wrapper command \"%1\" couldn't be found.");
            emit logLine(QString(reason).arg(wrapperCommand), MessageLevel::Fatal);
            emitFailed(tr(reason).arg(wrapperCommand));
            return;
        }
        emit logLine("Wrapper command is:\n" + wrapperCommandStr + "\n\n", MessageLevel::Launcher);
        args.prepend(javaPath);
        launchCommand = wrapperCommand;
        launchArgs = wrapperArgs + args;
    } else {
        launchCommand = javaPath;
        launchArgs = args;
    }

#ifdef Q_OS_LINUX
    // Beteliney: sandboxing opcional con Bubblewrap. Envuelve el comando final
    // (java o el wrapper del usuario, lo que corresponda) en un namespace
    // aislado que no expone el $HOME real, para que un mod malicioso no pueda
    // leer tokens de sesión de Discord/navegador. Ver BubblewrapSandbox.h.
    if (instance->settings()->get("EnableBubblewrapSandbox").toBool()) {
        if (Beteliney::BubblewrapSandbox::isAvailable()) {
            QString sandboxHome = FS::PathCombine(instance->gameRoot(), ".bwrap_home");
            QStringList bwrapArgs = Beteliney::BubblewrapSandbox::buildArgs(instance->gameRoot(), javaPath, sandboxHome);
            bwrapArgs << launchCommand << launchArgs;
            emit logLine("Sandboxing con Bubblewrap activado para esta instancia.\n\n", MessageLevel::Launcher);
            m_process.start("bwrap", bwrapArgs);
        } else {
            emit logLine(
                "El aislamiento con Bubblewrap está activado pero 'bwrap' no se encontró en el sistema. Lanzando sin aislar.\n\n",
                MessageLevel::Warning);
            m_process.start(launchCommand, launchArgs);
        }
    } else {
        m_process.start(launchCommand, launchArgs);
    }
#else
    m_process.start(launchCommand, launchArgs);
#endif

#ifdef BETELINEY_HAVE_GAMEMODE
    if (instance->settings()->get("EnableFeralGamemode").toBool() && APPLICATION->capabilities() & Application::SupportsGameMode) {
        auto pid = m_process.processId();
        if (pid) {
            gamemode_request_start_for(pid);
        }
    }
#endif
}

void LauncherPartLaunch::on_state(LoggedProcess::State state)
{
    switch (state) {
        case LoggedProcess::FailedToStart: {
            //: Error message displayed if instace can't start
            const char* reason = QT_TR_NOOP("Could not launch Minecraft!");
            emit logLine(reason, MessageLevel::Fatal);
            emitFailed(tr(reason));
            return;
        }
        case LoggedProcess::Aborted:
        case LoggedProcess::Crashed: {
            m_parent->setPid(-1);
            m_parent->instance()->setMinecraftRunning(false);
            {
                QString logText = m_parent->getLogModel()->toPlainText();
                QString reason = tr("Game crashed.");
                if (logText.contains("java.lang.OutOfMemoryError", Qt::CaseInsensitive)) {
                    reason = tr("Game crashed: Out of Memory (OutOfMemoryError). Solución: aumenta la RAM máxima en Configuración > Java.");
                } else if (logText.contains("Could not create the Java Virtual Machine", Qt::CaseInsensitive)) {
                    reason = tr("Game crashed: Java no pudo iniciarse. Probable causa: la RAM asignada supera la disponible.");
                } else if (logText.contains("incompatible magic value", Qt::CaseInsensitive) ||
                           logText.contains("class file has wrong version", Qt::CaseInsensitive)) {
                    reason = tr("Game crashed: Versión de Java incompatible con este mod/versión de Minecraft.");
                } else if (logText.contains("Pixel format not accelerated", Qt::CaseInsensitive) ||
                           logText.contains("No OpenGL context found", Qt::CaseInsensitive)) {
                    reason = tr("Game crashed: Error de OpenGL/GPU. Actualiza los drivers de AMD Radeon o fuerza software rendering.");
                } else if (logText.contains("Unable to access jarfile", Qt::CaseInsensitive)) {
                    reason = tr("Game crashed: Archivo .jar faltante o corrupto. Intenta reinstalar la instancia.");
                } else if (logText.contains("net.minecraftforge", Qt::CaseInsensitive) &&
                           logText.contains("mixin", Qt::CaseInsensitive)) {
                    reason = tr("Game crashed: Conflicto de mods (Mixin/Forge). Revisa la lista de mods instalados.");
                } else if (logText.contains("hs_err_pid", Qt::CaseInsensitive) ||
                           logText.contains("A fatal error has been detected by the Java Runtime", Qt::CaseInsensitive)) {
                    reason = tr("Game crashed: Error fatal de JVM (hs_err). Posible incompatibilidad de drivers o RAM insuficiente.");
                }
                emitFailed(reason);
            }
            return;
        }
        case LoggedProcess::Finished: {
            auto instance = m_parent->instance();
            if (instance->settings()->get("CloseAfterLaunch").toBool())
                APPLICATION->showMainWindow();

            m_parent->setPid(-1);
            m_parent->instance()->setMinecraftRunning(false);
            // if the exit code wasn't 0, report this as a crash
            auto exitCode = m_process.exitCode();
            if (exitCode != 0) {
                QString logText = m_parent->getLogModel()->toPlainText();
                QString reason = tr("Game crashed (exit code %1).").arg(exitCode);
                if (logText.contains("java.lang.OutOfMemoryError", Qt::CaseInsensitive)) {
                    reason = tr("Game crashed: Out of Memory. Aumenta la RAM en Configuración > Java.");
                }
                emitFailed(reason);
                return;
            }
            // Store game exit code so PostLaunchCommand can access it via INST_EXITCODE
            m_parent->setGameExitCode(exitCode);
            emitSucceeded();
            break;
        }
        case LoggedProcess::Running:
            emit logLine(QString("Minecraft process ID: %1\n\n").arg(m_process.processId()), MessageLevel::Launcher);
            m_parent->setPid(m_process.processId());
            // send the launch script to the launcher part
            m_process.write(m_launchScript.toUtf8());

            mayProceed = true;
            emit readyForLaunch();
            break;
        default:
            break;
    }
}

void LauncherPartLaunch::setWorkingDirectory(const QString& wd)
{
    m_process.setWorkingDirectory(wd);
}

void LauncherPartLaunch::proceed()
{
    if (mayProceed) {
        m_parent->instance()->setMinecraftRunning(true);
        QString launchString("launch\n");
        m_process.write(launchString.toUtf8());
        mayProceed = false;
    }
}

bool LauncherPartLaunch::abort()
{
    if (mayProceed) {
        mayProceed = false;
        QString launchString("abort\n");
        m_process.write(launchString.toUtf8());
    } else {
        auto state = m_process.state();
        if (state == LoggedProcess::Running || state == LoggedProcess::Starting) {
            m_process.kill();
        }
    }
    return true;
}
