// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "CheckModConflicts.h"

#include <QDir>
#include <QFileInfo>
#include <QMap>

#include "launch/LaunchTask.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/mod/Mod.h"
#include "minecraft/mod/tasks/LocalModParseTask.h"
#include "MessageLevel.h"

void CheckModConflicts::executeTask()
{
    auto* mc = qobject_cast<MinecraftInstance*>(m_parent->instance());
    if (!mc) {
        emitSucceeded();
        return;
    }

    QString modsPath = mc->modsRoot();
    QDir modsDir(modsPath);
    if (!modsDir.exists()) {
        emitSucceeded();
        return;
    }

    // Recopilar todos los .jar habilitados
    QStringList jarFilter { "*.jar", "*.litemod" };
    auto jarFiles = modsDir.entryInfoList(jarFilter, QDir::Files);

    if (jarFiles.isEmpty()) {
        emitSucceeded();
        return;
    }

    // mod_id → lista de archivos que lo definen
    QMap<QString, QStringList> idToFiles;

    for (const QFileInfo& fi : jarFiles) {
        // Ignorar mods deshabilitados (.jar.disabled)
        if (fi.suffix() == "disabled")
            continue;

        Mod mod(fi);
        if (!ModUtils::process(mod, ModUtils::ProcessingLevel::BasicInfoOnly))
            continue;

        const QString& modId = mod.details().mod_id;
        if (modId.isEmpty())
            continue;

        idToFiles[modId] << fi.fileName();
    }

    // Buscar duplicados
    QStringList conflicts;
    for (auto it = idToFiles.constBegin(); it != idToFiles.constEnd(); ++it) {
        if (it.value().size() > 1) {
            conflicts << QString("  • %1: %2").arg(it.key(), it.value().join(", "));
        }
    }

    if (conflicts.isEmpty()) {
        emit logLine("✔ Beteliney: Sin conflictos de mods detectados.\n", MessageLevel::Launcher);
        emitSucceeded();
        return;
    }

    // Registrar advertencia clara en el log de lanzamiento
    emit logLine("\n╔══════════════════════════════════════════════════════════╗", MessageLevel::Warning);
    emit logLine("║  ⚠  BETELINEY — MODS DUPLICADOS DETECTADOS              ║", MessageLevel::Warning);
    emit logLine("╠══════════════════════════════════════════════════════════╣", MessageLevel::Warning);
    emit logLine("║  Los siguientes mod IDs tienen más de un archivo JAR:   ║", MessageLevel::Warning);
    emit logLine("╚══════════════════════════════════════════════════════════╝", MessageLevel::Warning);

    for (const QString& conflict : conflicts)
        emit logLine(conflict, MessageLevel::Warning);

    emit logLine("\n→ Solución: ve a la carpeta mods y elimina el JAR duplicado.", MessageLevel::Warning);
    emit logLine("→ Minecraft puede crashear o comportarse de forma inesperada.\n", MessageLevel::Warning);

    // No abortar — el usuario decide si continúa
    emitSucceeded();
}
