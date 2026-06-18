// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include <QList>
#include <QString>
#include <optional>

namespace Beteliney {

/**
 * Fase 4.4 — Importador de instancias GDLauncher Carbon.
 *
 * GDLauncher Carbon guarda sus datos en:
 *   Linux:   ~/.local/share/gdlauncher_next/
 *   Windows: %APPDATA%/gdlauncher_next/   o   %LOCALAPPDATA%/gdlauncher_next/
 *
 * Base de datos: data.sqlite (QSqlDatabase / QSQLITE)
 *   Tabla `instance`: id, name, mc_version, modloader, modloader_version, shortpath, status
 *   Tabla `mod`: id, instance_id, name, filename, enabled
 *
 * Salida (formato Prism/Beteliney):
 *   <destDir>/<instanceName>/instance.cfg  — metadatos de la instancia
 *   <destDir>/<instanceName>/mmc-pack.json — componentes (MC + loader)
 *   <destDir>/<instanceName>/.minecraft/   — contenido del juego (copiado)
 */

struct GDInstance {
    QString id;         // id en la BD (para buscar mods y ruta)
    QString name;       // nombre de la instancia
    QString mcVersion;  // ej. "1.21.1"
    QString loader;     // "fabric" | "forge" | "neoforge" | "quilt" | "vanilla"
    QString loaderVersion;
    QString shortpath;  // subdirectorio dentro de gdlauncher_next/instances/
    QString sourcePath; // ruta completa al directorio de la instancia en GDL
};

/** Detecta el directorio de GDLauncher Carbon. Vacío si no existe. */
QString gdlauncherDataDir();

/** Lee las instancias de la BD SQLite. Requiere que Qt tenga driver QSQLITE. */
QList<GDInstance> readGDInstances(const QString& dataDir, QString* errorOut = nullptr);

/**
 * Importa una instancia al directorio de instancias de Beteliney.
 * Retorna vacío en éxito, o descripción del error.
 */
QString importGDInstance(const GDInstance& inst,
                         const QString&    destInstancesDir);

}  // namespace Beteliney
