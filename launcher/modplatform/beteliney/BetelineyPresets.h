// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include "BetelineyPack.h"
#include <QList>

namespace Beteliney {

/**
 * Presets de instancia locales — disponibles sin conexión a internet.
 * Aparecen en BetelineyPackPage junto con los packs remotos, pero
 * marcados como "built-in" (no requieren descarga de índice).
 *
 * Los mods se descargan igual desde Modrinth CDN al instalar.
 * URL y hash: rellenar antes de publicar (dejar vacío = se salta la verificación).
 */
inline QList<Pack> builtinPresets()
{
    QList<Pack> presets;

    // ── 1. Vanilla Optimizado (Fabric 1.21.1) ─────────────────────────────
    {
        Pack p;
        p.id              = "builtin-vanilla-optimized";
        p.name            = tr("Vanilla Optimizado");
        p.description     = tr("Fabric 1.21.1 con Sodium, Lithium, Iris y ModernFix.\n"
                               "Más FPS, menos lag, mismo gameplay.");
        p.longDescription = tr("**Vanilla Optimizado** instala los mods de rendimiento "
                               "más probados para Fabric:\n\n"
                               "- **Sodium** — renderizado optimizado (5-10× más FPS)\n"
                               "- **Lithium** — optimiza game logic, IA de mobs, chunks\n"
                               "- **Iris** — shaders compatibles con Sodium\n"
                               "- **ModernFix** — reduce uso de RAM y tiempo de carga\n\n"
                               "Ideal para PCs de gama media o baja.");
        p.version         = "1.0.0";
        p.minecraft       = "1.21.1";
        p.loader          = "fabric";
        p.loaderVersion   = "0.16.9";
        p.tags            = { "rendimiento", "fabric", "vanilla-plus", "builtin" };
        p.featured        = true;

        // Sodium — AANobbMI en Modrinth
        p.mods << PackMod{ PackProvider::Modrinth,
                           "AANobbMI",
                           "mc1.21.1-0.6.4+build.5",
                           "https://cdn.modrinth.com/data/AANobbMI/versions/PfHePkD4/"
                           "sodium-fabric-0.6.4%2Bmc1.21.1.jar",
                           "", // sha512 — rellenar con hash real
                           "sodium-fabric-0.6.4+mc1.21.1.jar" };

        // Lithium — gvQqBUqZ en Modrinth
        p.mods << PackMod{ PackProvider::Modrinth,
                           "gvQqBUqZ",
                           "mc1.21.1-0.13.0",
                           "https://cdn.modrinth.com/data/gvQqBUqZ/versions/le4NLU8p/"
                           "lithium-fabric-0.13.0%2Bmc1.21.1.jar",
                           "",
                           "lithium-fabric-0.13.0+mc1.21.1.jar" };

        // Iris — YL57xq9U en Modrinth
        p.mods << PackMod{ PackProvider::Modrinth,
                           "YL57xq9U",
                           "1.8.0+mc1.21.1",
                           "https://cdn.modrinth.com/data/YL57xq9U/versions/6Rq7oRXq/"
                           "iris-1.8.0%2Bmc1.21.1.jar",
                           "",
                           "iris-1.8.0+mc1.21.1.jar" };

        // ModernFix — nmDcB62a en Modrinth
        p.mods << PackMod{ PackProvider::Modrinth,
                           "nmDcB62a",
                           "5.19.4+mc1.21.1",
                           "https://cdn.modrinth.com/data/nmDcB62a/versions/hEOlIRIi/"
                           "modernfix-fabric-5.19.4%2Bmc1.21.1.jar",
                           "",
                           "modernfix-fabric-5.19.4+mc1.21.1.jar" };

        presets << p;
    }

    // ── 2. PvP Competitivo (Fabric 1.21.1) ────────────────────────────────
    {
        Pack p;
        p.id              = "builtin-pvp-competitive";
        p.name            = tr("PvP Competitivo");
        p.description     = tr("Fabric 1.21.1 con mods de PvP esenciales: FPS alto, hitboxes precisas.");
        p.longDescription = tr("**PvP Competitivo** incluye:\n\n"
                               "- **Sodium** — máximo FPS\n"
                               "- **Lithium** — menor latencia de tick\n"
                               "- **Ksyxis** — carga de spawn más rápida\n"
                               "- **FerriteCore** — menos uso de RAM\n\n"
                               "Perfil JVM recomendado: **Balanceado** (2048–4096 MB).");
        p.version         = "1.0.0";
        p.minecraft       = "1.21.1";
        p.loader          = "fabric";
        p.loaderVersion   = "0.16.9";
        p.tags            = { "pvp", "fabric", "competitivo", "builtin" };
        p.featured        = false;

        // Sodium
        p.mods << PackMod{ PackProvider::Modrinth, "AANobbMI", "mc1.21.1-0.6.4+build.5",
                           "https://cdn.modrinth.com/data/AANobbMI/versions/PfHePkD4/"
                           "sodium-fabric-0.6.4%2Bmc1.21.1.jar", "", "sodium-fabric-0.6.4+mc1.21.1.jar" };
        // Lithium
        p.mods << PackMod{ PackProvider::Modrinth, "gvQqBUqZ", "mc1.21.1-0.13.0",
                           "https://cdn.modrinth.com/data/gvQqBUqZ/versions/le4NLU8p/"
                           "lithium-fabric-0.13.0%2Bmc1.21.1.jar", "", "lithium-fabric-0.13.0+mc1.21.1.jar" };
        // FerriteCore — uXXizFIs en Modrinth
        p.mods << PackMod{ PackProvider::Modrinth, "uXXizFIs", "7.0.0",
                           "https://cdn.modrinth.com/data/uXXizFIs/versions/7Yq5lQ7i/"
                           "ferritecore-7.0.0-fabric.jar", "", "ferritecore-7.0.0-fabric.jar" };

        presets << p;
    }

    // ── 3. Modpack Pesado (NeoForge 1.21.1) ───────────────────────────────
    {
        Pack p;
        p.id              = "builtin-heavy-modpack";
        p.name            = tr("Modpack Pesado (NeoForge)");
        p.description     = tr("Base NeoForge 1.21.1 sin mods — optimizada para instalar 100+ mods manualmente.");
        p.longDescription = tr("**Modpack Pesado** es una instancia NeoForge 1.21.1 limpia "
                               "con el perfil JVM **Pesado (100–300 mods)** preconfigurado.\n\n"
                               "No incluye mods — añade los tuyos. El perfil JVM asigna "
                               "2048–6144 MB y usa `AlwaysPreTouch` para evitar pauses.\n\n"
                               "Recomendado: **8 GB de RAM** o más disponibles para el sistema.");
        p.version         = "1.0.0";
        p.minecraft       = "1.21.1";
        p.loader          = "neoforge";
        p.loaderVersion   = "21.1.172"; // usar versión recomendada de NeoForge para 1.21.1
        p.tags            = { "neoforge", "pesado", "base", "builtin" };
        p.featured        = false;
        // Sin mods — la instancia es una base limpia
        presets << p;
    }

    return presets;
}

}  // namespace Beteliney
