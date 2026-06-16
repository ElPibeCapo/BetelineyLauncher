// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 */
#pragma once

#include <QList>
#include <QString>

namespace Beteliney {

enum class DiagnosisSeverity {
    Critical,  // El juego no pudo iniciar
    Error,     // El juego crasheó en runtime
    Warning,   // Problema detectado, puede causar inestabilidad
    Info       // Información relevante
};

struct Diagnosis {
    DiagnosisSeverity severity;
    QString errorCode;      // ID interno: "OOM", "DUPLICATE_MOD", etc.
    QString title;          // Título corto visible al usuario
    QString explanation;    // Qué pasó, en español simple
    QString solution;       // Qué hacer exactamente
    QString detail;         // Fragmento del log relevante (opcional, máx 3 líneas)
    QString affectedMod;    // Nombre del mod causante (si aplica)
    QString actionLabel;    // Texto del botón de acción (vacío = sin botón)
    QString actionTarget;   // Página de settings a abrir ("java", "mods", etc.)
};

/**
 * Motor de diagnóstico de logs de Minecraft.
 * Analiza el texto completo del log y devuelve una lista ordenada de diagnósticos,
 * del más crítico al menos importante.
 *
 * Uso:
 *   QString log = m_model->toPlainText();
 *   auto diagnoses = Beteliney::LogAnalyzer::analyze(log, exitCode);
 */
class LogAnalyzer {
public:
    static QList<Diagnosis> analyze(const QString& fullLog, int exitCode = 0);

private:
    // Memoria / heap
    static void checkOutOfMemory(const QString& log, QList<Diagnosis>& out);
    static void checkHeapReservation(const QString& log, QList<Diagnosis>& out);

    // Mods
    static void checkDuplicateMod(const QString& log, QList<Diagnosis>& out);
    static void checkMissingDependency(const QString& log, QList<Diagnosis>& out);
    static void checkIncompatibleMods(const QString& log, QList<Diagnosis>& out);
    static void checkMixinConflict(const QString& log, QList<Diagnosis>& out);

    // Java
    static void checkJavaNotFound(const QString& log, QList<Diagnosis>& out);
    static void checkUnsupportedJavaVersion(const QString& log, QList<Diagnosis>& out);
    static void checkForgeJavaRequirement(const QString& log, QList<Diagnosis>& out);

    // GPU / gráficos
    static void checkOpenGLNotAccelerated(const QString& log, QList<Diagnosis>& out);
    static void checkOpenGLError(const QString& log, QList<Diagnosis>& out);

    // Sistema / OS
    static void checkNativesCrash(const QString& log, int exitCode, QList<Diagnosis>& out);
    static void checkWindowsLoadLibrary(const QString& log, QList<Diagnosis>& out);

    // Red
    static void checkNetworkError(const QString& log, QList<Diagnosis>& out);

    // Seguridad
    static void checkFractureiser(const QString& log, QList<Diagnosis>& out);

    // Forge / NeoForge específico
    static void checkForgeEarlyWindow(const QString& log, QList<Diagnosis>& out);
    static void checkForgeCoremods(const QString& log, QList<Diagnosis>& out);

    // Fabric específico
    static void checkFabricIncompatible(const QString& log, QList<Diagnosis>& out);

    // Helpers
    static QString extractContext(const QString& log, const QString& keyword, int linesBefore = 0, int linesAfter = 2);
    static QString extractModName(const QString& contextLine);
};

}  // namespace Beteliney
