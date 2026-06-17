// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include <QList>
#include <QString>
#include <QStringList>

namespace Beteliney {

enum class PackProvider { Modrinth, CurseForge, Direct };

struct PackMod {
    PackProvider provider;
    QString projectId;   // ID de proyecto en Modrinth o CurseForge
    QString version;     // ID de versión específica (vacío = última compatible)
    QString url;         // Para provider = Direct: URL de descarga directa
    QString sha512;      // Hash SHA-512 para verificación (recomendado)
    QString filename;    // Nombre de archivo final en la carpeta mods
};

struct Pack {
    int formatVersion = 1;
    QString id;              // Identificador único: "beteliney-survival-1"
    QString name;            // Nombre visible: "Beteliney Survival"
    QString description;     // Descripción corta (1-2 frases)
    QString longDescription; // Descripción larga (Markdown)
    QString version;         // Versión del pack: "1.0.0"
    QString minecraft;       // Versión de Minecraft: "1.21.1"
    QString loader;          // "fabric" | "forge" | "neoforge" | "quilt" | "vanilla"
    QString loaderVersion;   // Versión del loader: "0.16.9"
    QString iconUrl;         // URL del icono del pack (PNG, 128x128 recomendado)
    QStringList screenshots; // URLs de screenshots
    QStringList tags;        // ["survival", "vanilla-plus", "pvp", etc.]
    QList<PackMod> mods;     // Lista de mods a instalar
    bool featured = false;   // Si aparece destacado en la UI
};

// Resultado del índice (index.json)
struct PackIndex {
    int formatVersion = 1;
    QStringList ids;         // IDs de todos los packs disponibles
};

}  // namespace Beteliney
