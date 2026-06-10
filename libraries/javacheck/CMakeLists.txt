cmake_minimum_required(VERSION 3.20)
project(javacheck)
find_package(Java 1.8 REQUIRED COMPONENTS Development)

# Mismo fix que launcher: sin UseJava para evitar bug @argfile con Java 21+/26
set(JAVACHECK_JAR "${JARS_DEST_DIR}/JavaCheck.jar")
add_custom_command(
    OUTPUT "${JAVACHECK_JAR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/classes"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${JARS_DEST_DIR}"
    COMMAND "${Java_JAVAC_EXECUTABLE}" --release 8
            -d "${CMAKE_CURRENT_BINARY_DIR}/classes"
            "${CMAKE_CURRENT_SOURCE_DIR}/JavaCheck.java"
    COMMAND "${Java_JAR_EXECUTABLE}" --create
            --file "${JAVACHECK_JAR}"
            --main-class JavaCheck
            -C "${CMAKE_CURRENT_BINARY_DIR}/classes" .
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/JavaCheck.java"
    COMMENT "Compilando JavaCheck.jar (--release 8)"
    VERBATIM
)
add_custom_target(JavaCheck ALL DEPENDS "${JAVACHECK_JAR}")

# FIX: en builds de desarrollo el launcher busca JavaCheck.jar en
# <build>/jars/ pero javacheck lo genera en
# <build>/libraries/javacheck/share/<LauncherName>/.
# Creamos el directorio y copiamos el jar post-build para que funcione
# sin necesidad de hacer cmake --install.
add_custom_command(TARGET JavaCheck POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/jars"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/${JARS_DEST_DIR}/JavaCheck.jar"
        "${CMAKE_BINARY_DIR}/jars/JavaCheck.jar"
    COMMENT "Copiando JavaCheck.jar -> build/jars/ (fix ruta dev)"
    VERBATIM
)
