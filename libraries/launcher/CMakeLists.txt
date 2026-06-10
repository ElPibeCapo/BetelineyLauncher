cmake_minimum_required(VERSION 3.20)
project(launcher)
find_package(Java 1.8 REQUIRED COMPONENTS Development)

# NO usamos UseJava — CMake 4.x + Java 21+ genera @argfile vacío en jar -cfe.
# Usamos add_custom_command con sintaxis moderna de jar (Java 9+):
#   jar --create --file out.jar --main-class Main -C classdir .
# Esto incluye todos los .class del directorio directamente, sin argfiles.

set(CLASSES_DIR "${CMAKE_CURRENT_BINARY_DIR}/classes")

# ─── Fuentes absolutas ────────────────────────────────────────────────────────
set(SRC_FILES
    com/beteliney/launcher/EntryPoint.java
    com/beteliney/launcher/launcher/Launcher.java
    com/beteliney/launcher/launcher/impl/AbstractLauncher.java
    com/beteliney/launcher/launcher/impl/StandardLauncher.java
    com/beteliney/launcher/exception/ParameterNotFoundException.java
    com/beteliney/launcher/exception/ParseException.java
    com/beteliney/launcher/utils/Parameters.java
    com/beteliney/launcher/utils/ReflectionUtils.java
    com/beteliney/launcher/utils/logging/Level.java
    com/beteliney/launcher/utils/logging/Log.java
    com/beteliney/launcher/legacy/LegacyProxy.java
    com/beteliney/launcher/SystemProperties.java
)
set(SRC_ABS "")
foreach(f ${SRC_FILES})
    list(APPEND SRC_ABS "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
endforeach()

set(LEGACY_SRC_FILES
    legacy/com/beteliney/launcher/legacy/LegacyFrame.java
    legacy/com/beteliney/launcher/legacy/LegacyLauncher.java
    legacy/com/beteliney/launcher/legacy/fix/online/Handler.java
    legacy/com/beteliney/launcher/legacy/fix/online/OnlineFixes.java
    legacy/com/beteliney/launcher/legacy/fix/online/OnlineModeFix.java
    legacy/com/beteliney/launcher/legacy/fix/online/SkinFix.java
    legacy/com/beteliney/launcher/legacy/utils/Base64.java
    legacy/com/beteliney/launcher/legacy/utils/api/MojangApi.java
    legacy/com/beteliney/launcher/legacy/utils/api/Texture.java
    legacy/com/beteliney/launcher/legacy/utils/json/JsonParseException.java
    legacy/com/beteliney/launcher/legacy/utils/json/JsonParser.java
    legacy/com/beteliney/launcher/legacy/utils/url/ByteArrayUrlConnection.java
    legacy/com/beteliney/launcher/legacy/utils/url/UrlUtils.java
    legacy/net/minecraft/Launcher.java
    legacy/com/beteliney/launcher/legacy/LegacyProxy.java
)
set(LEGACY_SRC_ABS "")
foreach(f ${LEGACY_SRC_FILES})
    list(APPEND LEGACY_SRC_ABS "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
endforeach()

# ─── NewLaunch.jar ────────────────────────────────────────────────────────────
set(NEWLAUNCH_JAR "${JARS_DEST_DIR}/NewLaunch.jar")
add_custom_command(
    OUTPUT "${NEWLAUNCH_JAR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CLASSES_DIR}/NewLaunch"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${JARS_DEST_DIR}"
    COMMAND "${Java_JAVAC_EXECUTABLE}" --release 8
            -d "${CLASSES_DIR}/NewLaunch"
            ${SRC_ABS}
    COMMAND "${Java_JAR_EXECUTABLE}" --create
            --file "${NEWLAUNCH_JAR}"
            --main-class com.beteliney.launcher.EntryPoint
            -C "${CLASSES_DIR}/NewLaunch" .
    DEPENDS ${SRC_ABS}
    COMMENT "Compilando NewLaunch.jar (--release 8)"
    VERBATIM
)
add_custom_target(NewLaunch ALL DEPENDS "${NEWLAUNCH_JAR}")

# ─── NewLaunchLegacy.jar ──────────────────────────────────────────────────────
set(NEWLAUNCHLEGACY_JAR "${JARS_DEST_DIR}/NewLaunchLegacy.jar")
add_custom_command(
    OUTPUT "${NEWLAUNCHLEGACY_JAR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CLASSES_DIR}/NewLaunchLegacy"
    COMMAND "${Java_JAVAC_EXECUTABLE}" --release 8
            -cp "${NEWLAUNCH_JAR}"
            -d "${CLASSES_DIR}/NewLaunchLegacy"
            ${LEGACY_SRC_ABS}
    COMMAND "${Java_JAR_EXECUTABLE}" --create
            --file "${NEWLAUNCHLEGACY_JAR}"
            --main-class com.beteliney.launcher.EntryPoint
            -C "${CLASSES_DIR}/NewLaunchLegacy" .
    DEPENDS "${NEWLAUNCH_JAR}" ${LEGACY_SRC_ABS}
    COMMENT "Compilando NewLaunchLegacy.jar (--release 8)"
    VERBATIM
)
add_custom_target(NewLaunchLegacy ALL DEPENDS "${NEWLAUNCHLEGACY_JAR}")

# install_jar reemplazado — los JARs ya van directo a JARS_DEST_DIR
