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

# El copiado a build/jars/ para dev builds lo hace el CopyJars central
# en el CMakeLists.txt raiz (unico consumidor de JARS_DEST_DIR entre
# directorios). Tener aca ademas un POST_BUILD duplicado hacia el
# mismo destino compite con esa dependencia cruzada de directorio
# (ver nota en el CMakeLists.txt raiz) y en Windows/Ninja provoca una
# carrera: CopyJars puede correr antes de que este paso termine.

