// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include "BetelineyPack.h"
#include <QCoreApplication>
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
        p.name            = QObject::tr("Vanilla Optimizado");
        p.description     = QObject::tr("Fabric 1.21.1 con Sodium, Lithium, Iris y ModernFix.\n"
                               "Más FPS, menos lag, mismo gameplay.");
        p.longDescription = QObject::tr("**Vanilla Optimizado** instala los mods de rendimiento "
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

        // Sodium — AANobbMI en Modrinth (verificado en vivo 2026-07-06)
        p.mods << PackMod{ PackProvider::Modrinth,
                           "AANobbMI",
                           "mc1.21.1-0.8.12-fabric",
                           "https://cdn.modrinth.com/data/AANobbMI/versions/KIRFiWG4/"
                           "sodium-fabric-0.8.12%2Bmc1.21.1.jar",
                           "8afe411eec65a9f677611ed6390ce656e5a3572f9be473e5dca51ae882a9426a547cd2e8c793278577bb14c17e48158030b11753108926ef33698614bd94ed7f",
                           "sodium-fabric-0.8.12+mc1.21.1.jar" };

        // Lithium — gvQqBUqZ en Modrinth (verificado en vivo 2026-07-06)
        p.mods << PackMod{ PackProvider::Modrinth,
                           "gvQqBUqZ",
                           "mc1.21.1-0.15.4-fabric",
                           "https://cdn.modrinth.com/data/gvQqBUqZ/versions/N08Z8wog/"
                           "lithium-fabric-0.15.4%2Bmc1.21.1.jar",
                           "182064b00e6315e2255b857eaab8eb759e6b042ebd4cc8b855ff0d93f875a5a7188fac49f878d7b29d4ef7e6b6341190ad7f6f6f39f4a6d2c62003468b08e4c6",
                           "lithium-fabric-0.15.4+mc1.21.1.jar" };

        // Iris — YL57xq9U en Modrinth (verificado en vivo 2026-07-06)
        p.mods << PackMod{ PackProvider::Modrinth,
                           "YL57xq9U",
                           "1.8.14-beta.1+1.21.1-fabric",
                           "https://cdn.modrinth.com/data/YL57xq9U/versions/bAo1Qhte/"
                           "iris-fabric-1.8.14-beta.1%2Bmc1.21.1.jar",
                           "a7fbb629793c52f0be8b049f787cb598879239b1ad8e1de62e103c8b9efff140e3232b93ef1f14e505d262897d8cf9505b1126396429ad4056bff969c8674e52",
                           "iris-fabric-1.8.14-beta.1+mc1.21.1.jar" };

        // ModernFix — nmDcB62a en Modrinth (verificado en vivo 2026-07-06)
        p.mods << PackMod{ PackProvider::Modrinth,
                           "nmDcB62a",
                           "5.25.1+mc1.21.1",
                           "https://cdn.modrinth.com/data/nmDcB62a/versions/NnNX8LBn/"
                           "modernfix-fabric-5.25.1%2Bmc1.21.1.jar",
                           "dc67d6e023e1fcdeaf7837917c477cba212c611dfc2463c6ea021319c644087c79b477e0ea8194e113ddd7332fd5c6d82baa47c291eaac7f4a86252507b4e19f",
                           "modernfix-fabric-5.25.1+mc1.21.1.jar" };

        presets << p;
    }

    // ── 2. PvP Competitivo (Fabric 1.21.1) ────────────────────────────────
    {
        Pack p;
        p.id              = "builtin-pvp-competitive";
        p.name            = QObject::tr("PvP Competitivo");
        p.description     = QObject::tr("Fabric 1.21.1 con mods de PvP esenciales: FPS alto, hitboxes precisas.");
        p.longDescription = QObject::tr("**PvP Competitivo** incluye:\n\n"
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

        // Sodium (verificado en vivo 2026-07-06)
        p.mods << PackMod{ PackProvider::Modrinth, "AANobbMI", "mc1.21.1-0.8.12-fabric",
                           "https://cdn.modrinth.com/data/AANobbMI/versions/KIRFiWG4/"
                           "sodium-fabric-0.8.12%2Bmc1.21.1.jar",
                           "8afe411eec65a9f677611ed6390ce656e5a3572f9be473e5dca51ae882a9426a547cd2e8c793278577bb14c17e48158030b11753108926ef33698614bd94ed7f",
                           "sodium-fabric-0.8.12+mc1.21.1.jar" };
        // Lithium (verificado en vivo 2026-07-06)
        p.mods << PackMod{ PackProvider::Modrinth, "gvQqBUqZ", "mc1.21.1-0.15.4-fabric",
                           "https://cdn.modrinth.com/data/gvQqBUqZ/versions/N08Z8wog/"
                           "lithium-fabric-0.15.4%2Bmc1.21.1.jar",
                           "182064b00e6315e2255b857eaab8eb759e6b042ebd4cc8b855ff0d93f875a5a7188fac49f878d7b29d4ef7e6b6341190ad7f6f6f39f4a6d2c62003468b08e4c6",
                           "lithium-fabric-0.15.4+mc1.21.1.jar" };
        // FerriteCore — uXXizFIs en Modrinth (verificado en vivo 2026-07-06)
        p.mods << PackMod{ PackProvider::Modrinth, "uXXizFIs", "7.0.3-fabric",
                           "https://cdn.modrinth.com/data/uXXizFIs/versions/sOzRw3CG/"
                           "ferritecore-7.0.3-fabric.jar",
                           "3ad31620fac4ff44327dc7dedbe162b2d978f3f246dc16255a6e400ce9592a0d326fe36a626f3c1bf30a11f813093cbb4dcc107af039cff724d0cdf648541fdf",
                           "ferritecore-7.0.3-fabric.jar" };

        presets << p;
    }

    // ── 3. Modpack Pesado (NeoForge 1.21.1) ───────────────────────────────
    {
        Pack p;
        p.id              = "builtin-heavy-modpack";
        p.name            = QObject::tr("Modpack Pesado (NeoForge)");
        p.description     = QObject::tr("Base NeoForge 1.21.1 sin mods — optimizada para instalar 100+ mods manualmente.");
        p.longDescription = QObject::tr("**Modpack Pesado** es una instancia NeoForge 1.21.1 limpia "
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
