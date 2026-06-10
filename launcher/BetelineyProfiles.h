// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Perfiles JVM preconfigurados
 *
 *  Flags basados en las recomendaciones de Aikar (https://mcflags.emc.gs)
 *  adaptadas para distintos rangos de memoria y cargas de trabajo.
 *
 *  Notas generales:
 *  - G1GC es el recolector recomendado para Minecraft desde Java 8u191+
 *  - -XX:+PerfDisableSharedMem evita que la JVM cree archivos en /tmp para hsprof,
 *    lo que puede causar bloqueos en algunos sistemas Linux.
 *  - UseTransparentHugePages: solo en perfiles con heap >=2 GB. En heaps menores
 *    los hugepages de 2 MB desperdician más RAM de la que ahorran en TLB misses.
 *  - UseStringDeduplication: útil a partir de ~2 GB de heap máximo (modpacks con miles
 *    de strings repetidos). En heaps pequeños el thread de dedup consume más de lo que ahorra.
 *  - AlwaysPreTouch (Pesado/Extremo): pre-toca todas las páginas del heap al arrancar.
 *    Elimina los page-fault stutters durante el juego a cambio de ~2 s más de startup.
 *  - Xss controla el tamaño del stack por hilo; valores bajos pueden causar
 *    StackOverflowError en mods con recursión profunda.
 *  - UseCompressedOops y UseCompressedClassPointers son activos por defecto
 *    en JVM 64-bit con heap < 32 GB; no se incluyen explícitamente.
 *  - ConcGCThreads y ParallelGCThreads: NO se especifican. La JVM los calcula
 *    según los núcleos del sistema. Hardcodearlos perjudica en CPUs de 2 o de 32 núcleos.
 *  - GCTimeRatio=99: garantiza que la JVM dedique como máximo el 1% del tiempo a GC.
 *    Complementa MaxGCPauseMillis reduciendo la frecuencia total de colecciones.
 */

#pragma once
#include <QList>
#include <QString>

struct BetelineyJVMProfile {
    QString name;
    QString description;
    int minMemMB;
    int maxMemMB;
    QStringList jvmArgs;
};

// inline: una sola instancia aunque el header se incluya en múltiples .cpp
inline const QList<BetelineyJVMProfile> BETELINEY_PROFILES = {
    {
        /* Perfil vacío: permite limpiar los campos sin reemplazar por flags */
        "Personalizado (sin flags)",
        "Borra los argumentos JVM y los valores de memoria. Úsalo para configurar todo manualmente.",
        0,
        0,
        {}
    },
    {
        "iGPU / RAM compartida",
        "Para PCs con GPU integrada que comparte RAM con el sistema (iGPU AMD o Intel). "
        "Limita Minecraft a 1.5 GB para no privar de RAM a la iGPU ni al SO. Solo para Vanilla o mods muy ligeros.",
        384,
        1536,
        {
            "-XX:+UseG1GC",
            "-XX:+ParallelRefProcEnabled",
            "-XX:MaxGCPauseMillis=100",            // 100ms: más suave en iGPU — 200ms es tolerancia de servidor
            "-XX:+UnlockExperimentalVMOptions",
            "-XX:+DisableExplicitGC",
            "-XX:G1NewSizePercent=30",
            "-XX:G1MaxNewSizePercent=40",
            "-XX:G1HeapRegionSize=1M",             // 1536/1 = 1536 regiones — adecuado para heaps pequeños
            "-XX:G1ReservePercent=20",
            "-XX:G1MixedGCCountTarget=4",
            "-XX:InitiatingHeapOccupancyPercent=15",
            "-XX:G1MixedGCLiveThresholdPercent=90",
            "-XX:G1RSetUpdatingPauseTimePercent=5",
            "-XX:GCTimeRatio=99",
            "-XX:SurvivorRatio=32",
            "-XX:+PerfDisableSharedMem",
            "-XX:MaxTenuringThreshold=1",
            "-Xss768k",                            // Stack más pequeño — en iGPU la RAM es crítica y Vanilla no necesita 1M
            "-XX:MinHeapFreeRatio=5",              // Libera RAM al OS muy agresivamente
            "-XX:MaxHeapFreeRatio=10",
            "-Djava.rmi.server.disableHttp=true",  // Desactiva listener HTTP interno del JVM (innecesario en cliente)
            "-Dusing.aikars.flags=https://mcflags.emc.gs",
            "-Daikars.new.flags=true",
            // UseTransparentHugePages NO aplica aquí: heap de 1.5 GB con hugepages de 2 MB
            // desperdicia más RAM de la que ahorra. Solo activar en heaps >=2 GB.
        }
    },
    {
        "Ligero (Vanilla / sin mods)",
        "Vanilla o modpacks muy ligeros (< 10 mods). Conserva RAM para la GPU integrada y el SO.",
        512,
        2048,
        {
            "-XX:+UseG1GC",
            "-XX:+ParallelRefProcEnabled",
            "-XX:MaxGCPauseMillis=100",            // 100ms: gaming — 200ms es tolerancia de servidor
            "-XX:+UnlockExperimentalVMOptions",
            "-XX:+DisableExplicitGC",
            "-XX:G1NewSizePercent=30",             // Aikar: 30% también en heaps pequeños — reduce tenuring y full GC
            "-XX:G1MaxNewSizePercent=40",          // Aikar: 40% es el valor óptimo para G1
            "-XX:G1HeapRegionSize=1M",             // 2048/1M = 2048 regiones G1 — óptimo para heaps pequeños
            "-XX:G1ReservePercent=20",
            "-XX:G1MixedGCCountTarget=4",
            "-XX:InitiatingHeapOccupancyPercent=15",
            "-XX:G1MixedGCLiveThresholdPercent=90",
            "-XX:G1RSetUpdatingPauseTimePercent=5",
            "-XX:GCTimeRatio=99",                  // Max 1% del tiempo en GC
            "-XX:SurvivorRatio=32",
            "-XX:+PerfDisableSharedMem",
            "-XX:MaxTenuringThreshold=1",
            // Sin UseStringDeduplication: requiere heap max ≥2 GB para que valga la pena
            "-Xss1M",   // Mods ligeros pueden tener recursión; 1M es seguro y el overhead es mínimo
            "-XX:AutoBoxCacheMax=20000",            // Minecraft crea miles de Integer autoboxeados por tick
            "-Djava.rmi.server.disableHttp=true",  // Desactiva listener HTTP interno del JVM
            "-Dusing.aikars.flags=https://mcflags.emc.gs",
            "-Daikars.new.flags=true",
            "-XX:MinHeapFreeRatio=10",        // Libera RAM al OS agresivamente cuando GC reduce el heap
            "-XX:MaxHeapFreeRatio=20",        // Mantiene máx 20% libre antes de liberar al OS
            "-XX:+UseTransparentHugePages",   // Linux: reduce TLB misses y stutters de GC en heaps medianos
        }
    },
    {
        "Balanceado (OptiFine / Fabric / Forge ligero)",
        "Perfil principal. Modpacks de 10–100 mods. Buen equilibrio rendimiento / memoria.",
        2048,
        4096,
        {
            "-XX:+UseG1GC",
            "-XX:+ParallelRefProcEnabled",
            "-XX:MaxGCPauseMillis=100",            // 100ms: gaming — 200ms es tolerancia de servidor
            "-XX:+UnlockExperimentalVMOptions",
            "-XX:+DisableExplicitGC",
            "-XX:G1NewSizePercent=30",
            "-XX:G1MaxNewSizePercent=40",
            "-XX:G1HeapRegionSize=2M",             // 4096/2M = 2048 regiones G1 — óptimo
            "-XX:G1ReservePercent=20",
            "-XX:G1MixedGCCountTarget=4",
            "-XX:InitiatingHeapOccupancyPercent=15",
            "-XX:G1MixedGCLiveThresholdPercent=90",
            "-XX:G1RSetUpdatingPauseTimePercent=5",
            "-XX:GCTimeRatio=99",
            "-XX:SurvivorRatio=32",
            "-XX:+PerfDisableSharedMem",
            "-XX:MaxTenuringThreshold=1",
            "-XX:+UseStringDeduplication",         // Activado desde min 2 GB de max heap
            "-XX:StringDeduplicationAgeThreshold=3",
            "-XX:AutoBoxCacheMax=20000",            // Minecraft/mods crean miles de Integer autoboxeados por tick
            "-Xss1M",
            "-Djava.rmi.server.disableHttp=true",  // Desactiva listener HTTP interno del JVM
            "-Dusing.aikars.flags=https://mcflags.emc.gs",
            "-Daikars.new.flags=true",
            "-XX:MinHeapFreeRatio=10",
            "-XX:MaxHeapFreeRatio=20",
            "-XX:+UseTransparentHugePages",         // Linux: reduce TLB misses y stutters de GC
        }
    },
    {
        "Pesado (modpacks 100–300 mods)",
        "Modpacks grandes. No asignar más del 60% de la RAM total del sistema.",
        2048,
        6144,
        {
            "-XX:+UseG1GC",
            "-XX:+ParallelRefProcEnabled",
            "-XX:MaxGCPauseMillis=150",            // 150ms: heaps grandes necesitan más tiempo para limpiar
            "-XX:+UnlockExperimentalVMOptions",
            "-XX:+DisableExplicitGC",
            "-XX:G1NewSizePercent=30",
            "-XX:G1MaxNewSizePercent=50",
            "-XX:G1HeapRegionSize=8M",             // 6144/8 = 768 regiones G1
            "-XX:G1ReservePercent=20",
            "-XX:G1MixedGCCountTarget=4",
            "-XX:InitiatingHeapOccupancyPercent=20", // 20% más conservador en heaps grandes
            "-XX:G1MixedGCLiveThresholdPercent=90",
            "-XX:G1RSetUpdatingPauseTimePercent=5",
            "-XX:GCTimeRatio=99",
            "-XX:SurvivorRatio=32",
            "-XX:+PerfDisableSharedMem",
            "-XX:MaxTenuringThreshold=1",
            "-XX:+UseStringDeduplication",
            "-XX:StringDeduplicationAgeThreshold=3",
            "-XX:AutoBoxCacheMax=20000",            // Minecraft/mods crean miles de Integer autoboxeados por tick
            "-XX:+AlwaysPreTouch",                 // Pre-toca el heap al arrancar: elimina stutters
            "-Xss1M",
            "-XX:MinHeapFreeRatio=5",
            "-XX:MaxHeapFreeRatio=10",
            "-XX:+UseTransparentHugePages",         // Linux: reduce TLB misses y stutters de GC
            "-Djava.rmi.server.disableHttp=true",  // Desactiva listener HTTP interno del JVM
            "-Dusing.aikars.flags=https://mcflags.emc.gs",
            "-Daikars.new.flags=true",
        }
    },
    {
        "Extremo (servers / modpacks \u2265300 mods)",
        "Servidores locales o modpacks con m\u00e1s de 300 mods. No asignar m\u00e1s del 50% de RAM del sistema.",
        6144,
        12288,
        {
            "-XX:+UseG1GC",
            "-XX:+ParallelRefProcEnabled",
            "-XX:MaxGCPauseMillis=200",            // 200ms: heaps muy grandes (>6 GB) necesitan margen para mixed GC
            "-XX:+UnlockExperimentalVMOptions",
            "-XX:+DisableExplicitGC",
            "-XX:G1NewSizePercent=40",
            "-XX:G1MaxNewSizePercent=50",
            "-XX:G1HeapRegionSize=16M",            // 12288/16 = 768 regiones G1
            "-XX:G1ReservePercent=15",
            "-XX:G1MixedGCCountTarget=4",
            "-XX:InitiatingHeapOccupancyPercent=20",
            "-XX:G1MixedGCLiveThresholdPercent=90",
            "-XX:G1RSetUpdatingPauseTimePercent=5",
            "-XX:GCTimeRatio=99",
            "-XX:SurvivorRatio=32",
            "-XX:+PerfDisableSharedMem",
            "-XX:MaxTenuringThreshold=1",
            "-XX:+UseStringDeduplication",
            "-XX:StringDeduplicationAgeThreshold=3",
            "-XX:AutoBoxCacheMax=20000",            // Minecraft/mods crean miles de Integer autoboxeados por tick
            "-XX:+AlwaysPreTouch",                 // Obligatorio con heaps >6 GB
            "-Xss1M",
            "-XX:MinHeapFreeRatio=5",
            "-XX:MaxHeapFreeRatio=10",
            "-XX:+UseTransparentHugePages",         // Linux: reduce TLB misses y stutters de GC
            "-Djava.rmi.server.disableHttp=true",  // Desactiva listener HTTP interno del JVM
            "-Dusing.aikars.flags=https://mcflags.emc.gs",
            "-Daikars.new.flags=true",
        }
    },
    {
        /* MEJ-6: Perfil ZGC para Java 21+ — alternativa al perfil iGPU con pausas <1ms.
         * ZGC en Java 21 es GC de producción (ya no experimental).
         * Ventaja: pausas <1ms frente a ~100ms de G1GC en heaps de 1536MB.
         * SoftMaxHeapSize le dice a ZGC que prefiera mantenerse bajo 1280m,
         * pero puede subir hasta Xmx=1536m si Minecraft lo necesita.
         * NO usar con Java 8/11/17. Verificar compatibilidad con mods muy viejos. */
        "iGPU ZGC (Java 21+)",
        "Alternativa al perfil iGPU usando ZGC. Pausas de GC <1ms vs 100ms de G1GC. "
        "Solo compatible con Java 21 o superior. Verificar compatibilidad con mods antiguos antes de usar.",
        384,
        1536,
        {
            "-XX:+UseZGC",
            "-XX:+ZGenerational",                  // Java 21+: ZGC generacional — mejor para objetos de vida corta
            "-XX:ZUncommitDelay=60",               // libera memoria no usada al OS tras 60s de inactividad
            "-XX:SoftMaxHeapSize=1280m",            // heap preferido; puede subir hasta Xmx si Minecraft lo necesita
            "-XX:ZCollectionInterval=0.1",         // GC proactivo cada 100ms — evita stalls durante carga de chunks
            "-XX:ZAllocationSpikeTolerance=3.0",   // maneja picos de 3x en allocations (carga de mundo/chunks)
            "-XX:+DisableExplicitGC",              // ignora llamadas System.gc() de mods
            "-XX:+PerfDisableSharedMem",           // evita archivos hsprof en /tmp (necesario en VMs/Hyper-V)
            "-Xss768k",                            // stack por hilo — Vanilla no necesita más de 768k
            "-XX:MinHeapFreeRatio=5",              // agresivo liberando heap al OS — crítico en iGPU
            "-XX:MaxHeapFreeRatio=10",
            "-Djava.rmi.server.disableHttp=true",  // desactiva listener HTTP interno del JVM
            "-Dusing.aikars.flags=https://mcflags.emc.gs",
            "-Daikars.new.flags=true",
            // ZGC no usa flags G1GC (G1HeapRegionSize, G1NewSizePercent, etc.)
            // -XX:+UnlockExperimentalVMOptions NO necesario en Java 21+ (ZGC es producción)
            // UseTransparentHugePages NO aplica a ZGC (gestiona su propia memoria)
        }
    }
};

