// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "BetelineyLogAnalyzer.h"

#include <QRegularExpression>
#include <QStringList>
#include <algorithm>

namespace Beteliney {

// ─── PUNTO DE ENTRADA ────────────────────────────────────────────────────────

QList<Diagnosis> LogAnalyzer::analyze(const QString& log, int exitCode)
{
    if (log.trimmed().isEmpty())
        return {};

    QList<Diagnosis> out;

    // Seguridad — primero (malware)
    checkFractureiser(log, out);

    // Memoria / heap
    checkOutOfMemory(log, out);
    checkHeapReservation(log, out);

    // Java
    checkJavaNotFound(log, out);
    checkUnsupportedJavaVersion(log, out);
    checkForgeJavaRequirement(log, out);

    // Mods
    checkDuplicateMod(log, out);
    checkMissingDependency(log, out);
    checkIncompatibleMods(log, out);
    checkMixinConflict(log, out);
    checkFabricIncompatible(log, out);

    // GPU / gráficos
    checkOpenGLNotAccelerated(log, out);
    checkOpenGLError(log, out);

    // Forge específico
    checkForgeEarlyWindow(log, out);
    checkForgeCoremods(log, out);

    // Sistema / OS
    checkNativesCrash(log, exitCode, out);
    checkWindowsLoadLibrary(log, out);

    // Red
    checkNetworkError(log, out);

    // Ordenar: Critical > Error > Warning > Info
    std::stable_sort(out.begin(), out.end(), [](const Diagnosis& a, const Diagnosis& b) {
        return static_cast<int>(a.severity) < static_cast<int>(b.severity);
    });

    return out;
}

// ─── HELPERS ─────────────────────────────────────────────────────────────────

QString LogAnalyzer::extractContext(const QString& log, const QString& keyword, int linesBefore, int linesAfter)
{
    QStringList lines = log.split('\n');
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].contains(keyword, Qt::CaseInsensitive)) {
            int start = qMax(0, i - linesBefore);
            int end   = qMin(lines.size() - 1, i + linesAfter);
            QStringList ctx;
            for (int j = start; j <= end; ++j)
                ctx << lines[j].trimmed();
            return ctx.join('\n');
        }
    }
    return {};
}

QString LogAnalyzer::extractModName(const QString& contextLine)
{
    // Intenta extraer nombre de mod de patrones comunes en logs
    // "mod 'SomeMod' (somemod)" → SomeMod
    static QRegularExpression reMod(R"(mod\s+'([^']+)')");
    auto m = reMod.match(contextLine);
    if (m.hasMatch()) return m.captured(1);

    // "modid: somemod" → somemod
    static QRegularExpression reModId(R"(modid:\s*(\S+))");
    m = reModId.match(contextLine);
    if (m.hasMatch()) return m.captured(1);

    return {};
}

// ─── MEMORIA / HEAP ──────────────────────────────────────────────────────────

void LogAnalyzer::checkOutOfMemory(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("java.lang.OutOfMemoryError", Qt::CaseSensitive))
        return;

    QString detail = extractContext(log, "java.lang.OutOfMemoryError", 0, 3);

    // Determinar tipo de OOM
    QString explanation, solution;
    if (log.contains("Java heap space")) {
        explanation = "El juego se quedó sin memoria RAM asignada al heap de Java (Java heap space). "
                      "Minecraft intentó reservar más memoria de la que tenías configurada.";
        solution = "Ve a Configuración → Java → Memoria máxima (Xmx) y auméntala. "
                   "Para un juego vanilla, 2048 MB es suficiente. Con mods pesados, prueba 4096 MB o más. "
                   "No asignes más del 60% de tu RAM total.";
    } else if (log.contains("GC overhead limit exceeded")) {
        explanation = "El recolector de basura de Java está gastando demasiado tiempo limpiando memoria. "
                      "Hay muy poca RAM asignada para la cantidad de objetos que el juego genera.";
        solution = "Aumenta la memoria máxima en Configuración → Java. "
                   "También considera usar el perfil JVM 'Balanceado' o superior de BetelineyLauncher, "
                   "que tiene flags G1GC optimizados para reducir la presión del GC.";
    } else if (log.contains("Metaspace")) {
        explanation = "La Metaspace de Java (donde se almacenan las clases de los mods) se llenó. "
                      "Suele ocurrir con modpacks que tienen 200+ mods.";
        solution = "Agrega el flag JVM: -XX:MaxMetaspaceSize=512m (o 768m para modpacks muy grandes). "
                   "Puedes hacerlo en Configuración de instancia → Java → Argumentos JVM adicionales.";
    } else {
        explanation = "El juego se quedó sin memoria. Minecraft necesitaba más RAM de la disponible.";
        solution = "Aumenta la memoria máxima en Configuración → Java → Memoria máxima.";
    }

    out << Diagnosis{
        DiagnosisSeverity::Critical,
        "OOM",
        "Sin memoria suficiente",
        explanation,
        solution,
        detail,
        {},
        "Configurar memoria",
        "java"
    };
}

void LogAnalyzer::checkHeapReservation(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("Could not reserve enough space for", Qt::CaseSensitive) &&
        !log.contains("Unable to create new native thread", Qt::CaseSensitive))
        return;

    out << Diagnosis{
        DiagnosisSeverity::Critical,
        "HEAP_RESERVE",
        "No se pudo reservar memoria",
        "Java no pudo reservar el espacio de heap solicitado. Esto suele ocurrir cuando la "
        "memoria máxima configurada supera la RAM disponible en el sistema, o cuando otras "
        "aplicaciones ya ocupan demasiada RAM.",
        "Cierra otras aplicaciones pesadas (navegador, Discord, etc.) y reduce la memoria máxima "
        "en Configuración → Java. Si tienes 8 GB de RAM, configura máximo 4096 MB para Minecraft.",
        extractContext(log, "Could not reserve", 0, 2),
        {},
        "Configurar Java",
        "java"
    };
}

// ─── MODS ────────────────────────────────────────────────────────────────────

void LogAnalyzer::checkDuplicateMod(const QString& log, QList<Diagnosis>& out)
{
    // Fabric
    static QRegularExpression reFabric(
        R"(Duplicate mod id\s*'([^']+)'|Found duplicate mods:.*?'([^']+)')",
        QRegularExpression::CaseInsensitiveOption
    );
    // Forge / NeoForge
    static QRegularExpression reForge(
        R"(Found duplicate mod\s*:\s*(\S+)|Duplicate mods found:.*?modid='([^']+)')",
        QRegularExpression::CaseInsensitiveOption
    );

    QString modName;
    auto mFabric = reFabric.match(log);
    auto mForge  = reForge.match(log);

    if (mFabric.hasMatch())
        modName = mFabric.captured(1).isEmpty() ? mFabric.captured(2) : mFabric.captured(1);
    else if (mForge.hasMatch())
        modName = mForge.captured(1).isEmpty() ? mForge.captured(2) : mForge.captured(1);
    else if (log.contains("Duplicate mod", Qt::CaseInsensitive) ||
             log.contains("duplicate mods", Qt::CaseInsensitive))
        modName = "desconocido";
    else
        return;

    QString title = modName.isEmpty() || modName == "desconocido"
                    ? "Mod duplicado detectado"
                    : QString("Mod duplicado: %1").arg(modName);

    out << Diagnosis{
        DiagnosisSeverity::Critical,
        "DUPLICATE_MOD",
        title,
        QString("Hay dos archivos del mismo mod ('%1') en la carpeta mods de esta instancia. "
                "El juego no puede iniciar con mods duplicados.").arg(modName),
        "Ve a la carpeta de mods de esta instancia y elimina el archivo duplicado. "
        "Generalmente ocurre cuando descargas una actualización sin borrar la versión anterior.",
        extractContext(log, "Duplicate mod", 0, 3),
        modName,
        "Abrir carpeta de mods",
        "mods-folder"
    };
}

void LogAnalyzer::checkMissingDependency(const QString& log, QList<Diagnosis>& out)
{
    // Fabric: "requires mod 'X' to be loaded"
    static QRegularExpression reFabric(
        R"(requires (?:mod\s+)?'?([a-zA-Z0-9_\-]+)'? to be (?:loaded|present))",
        QRegularExpression::CaseInsensitiveOption
    );
    // Forge: "Missing Mods: ..."
    bool hasForge = log.contains("Missing Mods:", Qt::CaseSensitive) ||
                    log.contains("missing dependency", Qt::CaseInsensitive);

    auto mFabric = reFabric.match(log);

    if (!mFabric.hasMatch() && !hasForge)
        return;

    QString missingMod = mFabric.hasMatch() ? mFabric.captured(1) : QString();
    QString title = missingMod.isEmpty()
                    ? "Dependencia de mod faltante"
                    : QString("Falta el mod: %1").arg(missingMod);

    out << Diagnosis{
        DiagnosisSeverity::Critical,
        "MISSING_DEP",
        title,
        missingMod.isEmpty()
            ? "Uno o más mods requieren que otro mod esté instalado, pero no está en la carpeta de mods."
            : QString("Un mod requiere que '%1' esté instalado, pero no está en la carpeta de mods de esta instancia.").arg(missingMod),
        missingMod.isEmpty()
            ? "Revisa el log completo para ver cuál mod falta e instálalo desde Modrinth o CurseForge."
            : QString("Descarga e instala el mod '%1' desde Modrinth o CurseForge y vuelve a lanzar.").arg(missingMod),
        extractContext(log, "requires", 0, 2),
        missingMod,
        missingMod.isEmpty() ? QString() : QString("Buscar %1").arg(missingMod),
        missingMod.isEmpty() ? QString() : QString("search-modrinth:%1").arg(missingMod)
    };
}

void LogAnalyzer::checkIncompatibleMods(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("Incompatible mods found", Qt::CaseInsensitive) &&
        !log.contains("conflicts with mod", Qt::CaseInsensitive) &&
        !log.contains("is not compatible with", Qt::CaseInsensitive))
        return;

    out << Diagnosis{
        DiagnosisSeverity::Error,
        "INCOMPATIBLE_MODS",
        "Mods incompatibles entre sí",
        "Dos o más mods de esta instancia son incompatibles. El juego puede crashear o comportarse incorrectamente.",
        "Revisa el log para identificar qué mods están en conflicto. "
        "Generalmente una versión actualizada de uno de los mods resuelve el conflicto. "
        "Si no, uno de los dos mods tendrá que ser eliminado.",
        extractContext(log, "incompatible", 0, 4),
        {},
        {},
        {}
    };
}

void LogAnalyzer::checkMixinConflict(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("Mixin transformation failed", Qt::CaseInsensitive) &&
        !log.contains("MixinApplyError", Qt::CaseSensitive) &&
        !log.contains("mixin apply failed", Qt::CaseInsensitive))
        return;

    // Intentar extraer el mod causante del contexto
    QString ctx = extractContext(log, "Mixin transformation failed", 0, 5);
    if (ctx.isEmpty())
        ctx = extractContext(log, "MixinApplyError", 0, 5);

    static QRegularExpression reModMixin(R"(from mod (\S+)|in (\S+)\.mixins\.json)");
    auto m = reModMixin.match(ctx);
    QString modName = m.hasMatch() ? (m.captured(1).isEmpty() ? m.captured(2) : m.captured(1)) : QString();

    out << Diagnosis{
        DiagnosisSeverity::Error,
        "MIXIN_CONFLICT",
        modName.isEmpty() ? "Conflicto de Mixin entre mods" : QString("Mixin fallido: %1").arg(modName),
        "Un mod usa la técnica de 'Mixin' para modificar el código del juego, pero entró en conflicto "
        "con otro mod o con la versión de Minecraft. Este tipo de error es específico de la interacción "
        "entre mods y no tiene una solución directa.",
        "Actualiza todos los mods a sus versiones más recientes compatibles con esta versión de Minecraft. "
        "Si el error persiste, uno de los mods conflictivos tiene que ser eliminado. "
        "Reporta el problema al autor del mod mencionado en el log.",
        ctx,
        modName,
        {},
        {}
    };
}

void LogAnalyzer::checkFabricIncompatible(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("is not compatible with", Qt::CaseInsensitive) &&
        !log.contains("Incompatible mod set", Qt::CaseInsensitive))
        return;

    QString ctx = extractContext(log, "not compatible with", 0, 4);
    if (ctx.isEmpty()) ctx = extractContext(log, "Incompatible mod set", 0, 4);

    out << Diagnosis{
        DiagnosisSeverity::Critical,
        "FABRIC_INCOMPATIBLE",
        "Mods incompatibles con esta versión de Fabric/Minecraft",
        "Uno o más mods no son compatibles con la versión de Fabric o de Minecraft configurada. "
        "Fabric detectó el conflicto antes de iniciar el juego.",
        "Actualiza Fabric a su última versión estable desde la página de instancia. "
        "Luego actualiza todos los mods a versiones compatibles con tu versión de Minecraft. "
        "Si el problema persiste, uno de los mods aún no tiene soporte para esta versión.",
        ctx, {}, {}, {}
    };
}

// ─── JAVA ────────────────────────────────────────────────────────────────────

void LogAnalyzer::checkJavaNotFound(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("Failed to start the minecraft runtime", Qt::CaseInsensitive) &&
        !log.contains("No valid Java installation found", Qt::CaseInsensitive) &&
        !log.contains("java: not found", Qt::CaseSensitive) &&
        !log.contains("Cannot run program", Qt::CaseSensitive))
        return;

    out << Diagnosis{
        DiagnosisSeverity::Critical,
        "JAVA_NOT_FOUND",
        "Java no encontrado o no funciona",
        "El launcher no pudo iniciar Java. Puede que no esté instalado, que la ruta configurada "
        "sea incorrecta, o que el ejecutable esté dañado.",
        "Ve a Configuración → Java y usa 'Detectar Java automáticamente' o instala Java gestionado "
        "desde el launcher. Para Minecraft 1.17+ necesitas Java 17. Para 1.20.5+ necesitas Java 21.",
        extractContext(log, "Failed to start", 0, 2),
        {},
        "Configurar Java",
        "java"
    };
}

void LogAnalyzer::checkUnsupportedJavaVersion(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("UnsupportedClassVersionError", Qt::CaseSensitive) &&
        !log.contains("class file version", Qt::CaseInsensitive) &&
        !log.contains("has been compiled by a more recent version", Qt::CaseInsensitive))
        return;

    static QRegularExpression reVersion(R"(class file version (\d+\.\d+))");
    auto m = reVersion.match(log);
    QString detail, javaNeeded;
    if (m.hasMatch()) {
        double ver = m.captured(1).toDouble();
        if (ver >= 65.0)      javaNeeded = "Java 21";
        else if (ver >= 61.0) javaNeeded = "Java 17";
        else if (ver >= 55.0) javaNeeded = "Java 11";
        else                  javaNeeded = "Java 8";
        detail = QString("Versión de clase: %1 (requiere %2)").arg(m.captured(1), javaNeeded);
    }

    out << Diagnosis{
        DiagnosisSeverity::Critical,
        "JAVA_VERSION_OLD",
        javaNeeded.isEmpty() ? "Versión de Java demasiado antigua" : QString("Necesitas %1 o superior").arg(javaNeeded),
        "La versión de Java configurada es demasiado antigua para esta versión de Minecraft o sus mods.",
        javaNeeded.isEmpty()
            ? "Instala una versión más reciente desde Configuración → Java."
            : QString("Instala %1 desde Configuración → Java → Java gestionado.").arg(javaNeeded),
        detail, {},
        "Actualizar Java",
        "java"
    };
}

void LogAnalyzer::checkForgeJavaRequirement(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("requires Java", Qt::CaseInsensitive) &&
        !log.contains("Forge requires", Qt::CaseInsensitive))
        return;
    if (!log.contains("ERROR", Qt::CaseSensitive) && !log.contains("requires", Qt::CaseInsensitive))
        return;

    static QRegularExpression reReq(R"(requires Java (\d+))");
    auto m = reReq.match(log);
    QString needed = m.hasMatch() ? QString("Java %1").arg(m.captured(1)) : "Java 17+";

    out << Diagnosis{
        DiagnosisSeverity::Critical,
        "FORGE_JAVA_REQ",
        QString("Forge/NeoForge requiere %1").arg(needed),
        QString("Esta versión de Forge o NeoForge requiere %1, pero tienes configurada una versión más antigua.").arg(needed),
        QString("Botón derecho en la instancia → Editar → Java → selecciona %1. "
                "Si no lo tienes, descárgalo desde Configuración → Java → Java gestionado.").arg(needed),
        extractContext(log, "requires Java", 0, 2), {},
        QString("Configurar %1").arg(needed),
        "java"
    };
}

// ─── GPU / GRÁFICOS ──────────────────────────────────────────────────────────

void LogAnalyzer::checkOpenGLNotAccelerated(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("Pixel format not accelerated", Qt::CaseInsensitive) &&
        !log.contains("No OpenGL accelerated renderer", Qt::CaseInsensitive) &&
        !log.contains("software renderer", Qt::CaseInsensitive))
        return;

    out << Diagnosis{
        DiagnosisSeverity::Critical,
        "OPENGL_NOT_ACCEL",
        "OpenGL no acelerado por hardware",
        "Tu GPU no está siendo usada para renderizar Minecraft. El juego corre en modo software "
        "(solo CPU), lo que lo hace extremadamente lento o directamente incapaz de iniciar.",
        "Actualiza los drivers de tu GPU (NVIDIA/AMD/Intel). En Linux con GPU integrada: "
        "actualiza el paquete mesa. En máquina virtual: instala los drivers de guest "
        "(VirtualBox Guest Additions, VMware Tools).",
        extractContext(log, "Pixel format", 0, 2),
        {}, {}, {}
    };
}

void LogAnalyzer::checkOpenGLError(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("OpenGL error", Qt::CaseInsensitive) &&
        !log.contains("GL_ERROR", Qt::CaseSensitive) &&
        !log.contains("org.lwjgl.opengl.OpenGLException", Qt::CaseSensitive))
        return;

    for (const auto& d : out)
        if (d.errorCode == "OPENGL_NOT_ACCEL") return;

    out << Diagnosis{
        DiagnosisSeverity::Warning,
        "OPENGL_ERROR",
        "Error de OpenGL durante el juego",
        "El juego reportó errores de OpenGL. Puede causar crashes gráficos, pantallas negras o artefactos.",
        "1. Añade -Dfml.earlyprogresswindow=false en los argumentos JVM de la instancia.\n"
        "2. Desactiva OptiFine si está instalado.\n"
        "3. Actualiza los drivers de GPU.\n"
        "4. Prueba con otra distribución de Java (Adoptium vs Azul Zulu).",
        extractContext(log, "OpenGL error", 0, 3),
        {}, {}, {}
    };
}

// ─── SISTEMA / OS ────────────────────────────────────────────────────────────

void LogAnalyzer::checkNativesCrash(const QString& log, int exitCode, QList<Diagnosis>& out)
{
    Q_UNUSED(exitCode)
    bool hasSignal = log.contains("SIGSEGV", Qt::CaseSensitive) ||
                     log.contains("SIGABRT", Qt::CaseSensitive) ||
                     log.contains("signal 11", Qt::CaseInsensitive);
    bool hasAccess = log.contains("EXCEPTION_ACCESS_VIOLATION", Qt::CaseSensitive) ||
                     log.contains("A fatal error has been detected by the Java Runtime", Qt::CaseSensitive);
    bool hasCrash  = log.contains("hs_err_pid", Qt::CaseSensitive);

    if (!hasSignal && !hasAccess && !hasCrash) return;

    QString ctx;
    if (hasAccess) ctx = extractContext(log, "EXCEPTION_ACCESS_VIOLATION", 0, 4);
    if (ctx.isEmpty()) ctx = extractContext(log, "SIGSEGV", 0, 4);
    if (ctx.isEmpty()) ctx = extractContext(log, "hs_err_pid", 0, 2);

    out << Diagnosis{
        DiagnosisSeverity::Error,
        "NATIVE_CRASH",
        "Crash fatal en la JVM (error de memoria nativa)",
        "La JVM crasheó con un error de acceso a memoria. Causas comunes: mod con código nativo "
        "defectuoso, driver de GPU inestable, o incompatibilidad de la distribución de Java.",
        "1. Cambia la distribución de Java (Adoptium → Azul Zulu 21, o viceversa).\n"
        "2. Actualiza los drivers de GPU.\n"
        "3. Desactiva OptiFine/shaders de uno en uno para identificar el mod problemático.\n"
        "4. Busca el archivo hs_err_pid*.log en la carpeta de la instancia para más detalles.",
        ctx, {}, {}, {}
    };
}

void LogAnalyzer::checkWindowsLoadLibrary(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("LoadLibrary failed", Qt::CaseSensitive) &&
        !log.contains("The specified module could not be found", Qt::CaseInsensitive) &&
        !log.contains("Unable to load native library", Qt::CaseInsensitive))
        return;
    if (!log.contains(".dll", Qt::CaseInsensitive)) return;

    static QRegularExpression reDll(R"((\w+\.dll))");
    auto m = reDll.match(log);
    QString dllName = m.hasMatch() ? m.captured(1) : "una DLL requerida";

    out << Diagnosis{
        DiagnosisSeverity::Critical,
        "DLL_NOT_FOUND",
        QString("DLL faltante: %1").arg(dllName),
        QString("Windows no puede cargar %1. Puede ser instalación incompleta del launcher, "
                "drivers de GPU desactualizados, o componentes del sistema faltantes.").arg(dllName),
        "1. Reinstala BetelineyLauncher.\n"
        "2. Instala Visual C++ Redistributable (x64) desde microsoft.com.\n"
        "3. Si la DLL es de GPU: actualiza los drivers.\n"
        "4. Ejecuta 'sfc /scannow' como administrador.",
        extractContext(log, "LoadLibrary", 0, 3),
        dllName, {}, {}
    };
}

// ─── RED ─────────────────────────────────────────────────────────────────────

void LogAnalyzer::checkNetworkError(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("java.net.ConnectException", Qt::CaseSensitive) &&
        !log.contains("java.net.UnknownHostException", Qt::CaseSensitive) &&
        !log.contains("Unable to connect to session server", Qt::CaseInsensitive))
        return;

    bool isAuth = log.contains("session server", Qt::CaseInsensitive) ||
                  log.contains("sessionserver.mojang.com", Qt::CaseInsensitive);

    if (isAuth) {
        out << Diagnosis{
            DiagnosisSeverity::Warning,
            "AUTH_NETWORK_ERROR",
            "Error de conexión con los servidores de Mojang",
            "El juego no pudo conectarse a los servidores de autenticación de Mojang. "
            "Puedes jugar offline pero no en servidores que requieran cuenta premium.",
            "Verifica tu conexión a internet. Si el problema persiste, revisa status.mojang.com. "
            "Confirma que ningún antivirus bloquea el acceso de Java a internet.",
            extractContext(log, "session server", 0, 2),
            {}, {}, {}
        };
    } else {
        out << Diagnosis{
            DiagnosisSeverity::Warning,
            "NETWORK_ERROR",
            "Error de red durante el juego",
            "El juego encontró un error de conexión de red.",
            "Verifica tu conexión. Si ocurre al conectarte a un servidor específico, "
            "puede estar caído o tener una versión incompatible.",
            extractContext(log, "ConnectException", 0, 2),
            {}, {}, {}
        };
    }
}

// ─── SEGURIDAD ───────────────────────────────────────────────────────────────

void LogAnalyzer::checkFractureiser(const QString& log, QList<Diagnosis>& out)
{
    static const QStringList malwarePatterns = {
        "fractureiser",
        "85.217.144.130",
        "107.189.3.101",
        "v1.skyrage.de",
        "files.skyrage.de",
        "sigma.pl",
    };

    for (const QString& pattern : malwarePatterns) {
        if (log.contains(pattern, Qt::CaseInsensitive)) {
            out << Diagnosis{
                DiagnosisSeverity::Critical,
                "MALWARE_FRACTUREISER",
                "POSIBLE MALWARE DETECTADO",
                "El log contiene patrones del malware Fractureiser. Este malware se distribuyó "
                "a través de mods de CurseForge en junio de 2023 y puede robar credenciales "
                "y tokens de cuentas de Minecraft.",
                "ACCIÓN INMEDIATA:\n"
                "1. Desconéctate de internet.\n"
                "2. Cambia tu contraseña Microsoft DESDE OTRO DISPOSITIVO.\n"
                "3. Escanea con un antivirus actualizado.\n"
                "4. Elimina todos los mods y descárgalos nuevamente de fuentes verificadas.\n"
                "5. Revisa archivos extraños en ~/.minecraft/ o %AppData%/.minecraft/.",
                extractContext(log, pattern, 1, 1),
                {}, {}, {}
            };
            return;
        }
    }
}

// ─── FORGE ESPECÍFICO ────────────────────────────────────────────────────────

void LogAnalyzer::checkForgeEarlyWindow(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("earlyprogresswindow", Qt::CaseInsensitive) &&
        !log.contains("EarlyLoadingException", Qt::CaseSensitive) &&
        !log.contains("Early window error", Qt::CaseInsensitive))
        return;

    out << Diagnosis{
        DiagnosisSeverity::Error,
        "FORGE_EARLY_WINDOW",
        "Forge: error en ventana de carga temprana",
        "Forge no pudo inicializar su ventana de carga temprana. Suele ocurrir en GPUs integradas, "
        "drivers desactualizados, o ciertos entornos Linux.",
        "Añade este argumento JVM en la configuración de la instancia:\n"
        "-Dfml.earlyprogresswindow=false",
        extractContext(log, "earlyprogresswindow", 1, 2),
        {}, {}, {}
    };
}

void LogAnalyzer::checkForgeCoremods(const QString& log, QList<Diagnosis>& out)
{
    if (!log.contains("Coremod", Qt::CaseSensitive)) return;
    if (!log.contains("ERROR", Qt::CaseSensitive) &&
        !log.contains("failed", Qt::CaseInsensitive)) return;

    QString ctx = extractContext(log, "Coremod", 0, 3);
    static QRegularExpression reMod(R"(Coremod\s+(\S+))");
    auto m = reMod.match(log);
    QString coreMod = m.hasMatch() ? m.captured(1) : QString();

    out << Diagnosis{
        DiagnosisSeverity::Error,
        "FORGE_COREMOD_ERROR",
        coreMod.isEmpty() ? "Error en Coremod de Forge" : QString("Error en Coremod: %1").arg(coreMod),
        "Un Coremod de Forge falló durante la inicialización. Ocurre cuando el coremod "
        "no es compatible con la versión de Minecraft o de Forge que tienes.",
        "Verifica que todos tus mods son compatibles con la versión de Forge que usas. "
        "Actualiza el mod problemático o elimínalo si no tiene versión compatible.",
        ctx, coreMod, {}, {}
    };
}

}  // namespace Beteliney
