# Política de seguridad

## Versiones soportadas

Solo la última versión publicada (ver [Releases](https://github.com/ElPibeCapo/BetelineyLauncher/releases)) recibe parches de seguridad. No se mantienen ramas LTS.

## Reportar una vulnerabilidad

**No abras un issue público para vulnerabilidades reales** (ejecución remota de código, bypass de la verificación de firma del updater, fuga de credenciales, path traversal, etc.). Un issue público le da a un atacante la misma información que al mantenedor antes de que exista un parche.

En su lugar, reportalo por privado:

- GitHub Security Advisories: [reportar acá](https://github.com/ElPibeCapo/BetelineyLauncher/security/advisories/new) (preferido — queda privado hasta que se resuelve).
- Correo: [betelineylauncher@gmail.com](mailto:betelineylauncher@gmail.com), asunto `[SECURITY]`.

Incluí, en lo posible: versión afectada, sistema operativo, pasos para reproducir, y el impacto que le ves (qué podría hacer alguien explotándolo).

Tiempo de respuesta esperado: confirmación en 72 horas, parche o mitigación según severidad — sin plazo fijo prometido, es un proyecto de un solo mantenedor, pero las vulnerabilidades reales tienen prioridad sobre features nuevas.

## Qué SÍ cubre esta política

- El binario del launcher y el código de este repositorio.
- El proceso de actualización (`updater/betelineyupdater/`, verificación de firma Ed25519).
- El escáner de malware (`MalwareScanner`) y la fuente de hashes que consulta.
- BetelineyPacks y cualquier endpoint propio (`betelineylauncher.workers.dev` y similares) que el launcher consulte.

## Qué NO cubre

- Vulnerabilidades en Minecraft mismo, en Mojang/Microsoft, o en mods/modpacks de terceros que el usuario decida instalar — reportalas al autor correspondiente.
- Código heredado sin modificar de [Prism Launcher](https://github.com/PrismLauncher/PrismLauncher) que no haya sido tocado por este fork — reportalo río arriba también, ahí lo van a poder arreglar para todos los forks a la vez.

## Divulgación

Preferimos coordinar el anuncio con quien reporta antes de publicar detalles técnicos completos. Si el reporte resulta en un fix, se documenta en [`docs/CHANGELOG.md`](docs/CHANGELOG.md) sin necesariamente detallar el vector de ataque hasta que la mayoría de instalaciones haya actualizado.
