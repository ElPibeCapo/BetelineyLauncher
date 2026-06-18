// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include <QString>

/**
 * Beteliney Crash Reporter — Fase 4.3
 *
 * Flujo:
 *   1. main() llama installPanicHandler() antes de crear Application.
 *      Instala handlers para SIGSEGV/SIGABRT/SIGFPE/SIGILL (Linux/macOS)
 *      o SetUnhandledExceptionFilter (Windows).
 *      En el crash: escribe /tmp/beteliney_crash_<pid>.txt con el backtrace.
 *
 *   2. Application::init() llama checkAndShowCrashReport().
 *      Si encuentra archivos de crash → muestra diálogo con reporte
 *      + botón para abrir issue en GitHub con template pre-llenado.
 */
namespace Beteliney {

void installPanicHandler();
void checkAndShowCrashReport(const QString& launcherVersion);

}  // namespace Beteliney
