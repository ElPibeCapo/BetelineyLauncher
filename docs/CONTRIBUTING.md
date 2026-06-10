# Guía de contribución — Beteliney Launcher

## Política sobre IA Generativa

- No publiques salida directa de LLMs como comentarios o sugerencias sin revisarla.
- Si usaste IA para ayudarte a escribir código o documentación, asegurate de entender
  completamente los cambios y poder explicar por qué son correctos.
- Los commits con asistencia de IA deben incluir una etiqueta `Assisted-by`:

```
Assisted-by: Claude:claude-sonnet-4-6
```

Los agentes IA **NO** deben agregar `Signed-off-by`. Solo los humanos pueden
certificar el Developer Certificate of Origin (DCO).

---

## Estilo de código

Todos los archivos se formatean con `clang-format` usando la configuración en `.clang-format`.
Ejecutarlo sobre los archivos modificados antes de hacer commit.

Convenciones C++:

- Clases y tipos: `PascalCase` — `MyClass`
- Miembros privados/protegidos: `camelCase` con prefijo `m_` — `m_myCounter`
- Miembros estáticos privados/protegidos: `camelCase` con prefijo `s_` — `s_instance`
- Miembros públicos: `camelCase` sin prefijo — `dateOfBirth`
- Constantes estáticas: `SCREAMING_SNAKE_CASE` — `MAX_VALUE`
- Funciones miembro: `camelCase` — `incrementCounter`
- Variables/funciones globales: `camelCase` — `globalData`
- Constantes globales y macros: `SCREAMING_SNAKE_CASE` — `LIGHT_GRAY`
- Constantes de enum: `PascalCase` — `CamelusBactrianus`
- Evitar inventar siglas o abreviaciones para nombres de múltiples palabras.

---

## Firmando tu trabajo (DCO)

Para asegurar que el código es compatible con las licencias del proyecto,
todos los commits deben estar firmados:

```bash
git commit -s -m "Tu mensaje de commit"
```

O agregando manualmente al mensaje:

```
Signed-off-by: Tu Nombre <tu@email.com>
```

Al firmar, aceptas los términos del Developer Certificate of Origin 1.1.

---

## Tests

Los tests unitarios están en `tests/`. Para agregar uno nuevo:

1. Crear `tests/MiFeature_test.cpp` usando Qt Test (`QTest`)
2. Agregarlo a `tests/CMakeLists.txt`:

```cmake
ecm_add_test(MiFeature_test.cpp LINK_LIBRARIES Launcher_logic Qt${QT_VERSION_MAJOR}::Test
    TEST_NAME MiFeature)
```

Para los perfiles JVM, el test de referencia es `tests/BetelineyProfiles_test.cpp`.

---

## Compilar para desarrollo

Ver [`docs/COMPILAR_WINDOWS.md`](COMPILAR_WINDOWS.md) para instrucciones completas.

Build rápido incremental (modo 2 del bat):
```bat
COMPILAR_BETELINEY.bat  ->  [2] Recompilar
```

---

## Notas importantes

- Este proyecto compila con **MinGW (GCC)**, no con MSVC.
  No mezclar DLLs de builds MSVC con este build MinGW — son ABI incompatibles.
- Los flags de CPU (`-march=znver1`) están ajustados para Ryzen 3700U (Zen+/Picasso).
  `COMPILAR_LINUX.sh` detecta la CPU automáticamente — en otro hardware usa `-march=native` sin tocar nada.
  En Windows (`COMPILAR_BETELINEY.bat`), ajustá los flags manualmente si compilás en otra CPU.
- Beteliney ya no es un fork directo de Prism Launcher — las diferencias acumuladas
  hacen que los bugs y soluciones no sean directamente equivalentes.
