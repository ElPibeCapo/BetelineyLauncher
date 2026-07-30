# Scripts de desarrollo

Utilidades para compilar y verificar el proyecto localmente.

## Compilación

- **`build_fast.sh`** — compila con la configuración de desarrollo.
- **`check_build.sh`** — verifica que el build esté sano y el binario arranque.
- **`verify_presets.py`** — valida que `CMakePresets.json` sea coherente
  (que los presets referenciados existan y no haya rutas rotas).

## Seguridad

- **`secret_scan.sh`** — busca credenciales, tokens y claves en los archivos
  que estás por commitear. Funciona como hook de `pre-commit`.
- **`install_hooks.sh`** — instala el hook anterior en `.git/hooks/`.

Recomendado después de clonar:

```bash
./tools/dev/install_hooks.sh
```

Eso evita subir por accidente una API key o un `.pem`. Vale la pena aunque
trabajes solo.

## Uso típico

```bash
./tools/dev/build_fast.sh          # compilar
./tools/dev/check_build.sh         # verificar
python3 tools/dev/verify_presets.py  # validar presets
```

Ver `CONTRIBUTING.md` en la raíz para el flujo completo de contribución.
