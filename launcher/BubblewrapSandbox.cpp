// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "BubblewrapSandbox.h"

#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>

namespace Beteliney {

bool BubblewrapSandbox::isAvailable()
{
    return !QStandardPaths::findExecutable("bwrap").isEmpty();
}

QStringList BubblewrapSandbox::buildArgs(const QString& instanceRoot, const QString& javaPath, const QString& sandboxHome)
{
    QStringList args;

    auto roBindTry = [&args](const QString& path) {
        if (QFileInfo::exists(path))
            args << "--ro-bind-try" << path << path;
    };

    // Base del sistema, solo lectura: suficiente para que Java, las
    // bibliotecas nativas de Minecraft (LWJGL/GLFW/OpenAL) y los drivers
    // gráficos/de audio encuentren todo lo que necesitan, sin acceso de
    // escritura al sistema real.
    roBindTry("/usr");
    roBindTry("/etc");
    roBindTry("/lib");
    roBindTry("/lib64");
    roBindTry("/bin");
    roBindTry("/sbin");
    roBindTry("/opt");

    // Pseudo-filesystems que la JVM y las bibliotecas gráficas dan por hecho.
    args << "--dev" << "/dev";
    args << "--proc" << "/proc";

    // /tmp aislado: tmpfs propio y vacío, no el /tmp real del host — así el
    // proceso del juego no puede leer archivos temporales de otros procesos
    // del usuario.
    args << "--tmpfs" << "/tmp";

    // Runtime dir de sesión (sockets de Wayland, PipeWire/PulseAudio, D-Bus).
    // Sin esto no hay audio ni ventana en la gran mayoría de setups modernos.
    QString runtimeDir = QProcessEnvironment::systemEnvironment().value("XDG_RUNTIME_DIR");
    if (!runtimeDir.isEmpty() && QFileInfo::exists(runtimeDir))
        args << "--bind" << runtimeDir << runtimeDir;

    // X11 (incluye XWayland): el socket vive bajo /tmp/.X11-unix, que quedó
    // vacío por el tmpfs nuevo de arriba — hay que exponerlo aparte.
    if (QFileInfo::exists("/tmp/.X11-unix"))
        args << "--ro-bind" << "/tmp/.X11-unix" << "/tmp/.X11-unix";

    // Directorio de la instancia: único lugar con escritura real (saves,
    // config, screenshots, logs, mods, natives extraídos).
    if (!instanceRoot.isEmpty())
        args << "--bind" << instanceRoot << instanceRoot;

    // Carpeta contenedora del ejecutable de Java real, por si vive fuera de
    // los directorios del sistema ya montados (JDKs portables bajo el data
    // dir del launcher).
    QFileInfo javaInfo(javaPath);
    QString javaDir = javaInfo.absolutePath();
    if (!javaDir.isEmpty() && QFileInfo::exists(javaDir))
        args << "--ro-bind-try" << javaDir << javaDir;

    // $HOME falso: el punto central del threat model. Un mod malicioso que
    // intente leer tokens de sesión de Discord, cookies de navegador, claves
    // SSH, etc. desde el $HOME real, se encuentra en cambio con este
    // directorio vacío dentro de la propia instancia.
    if (!sandboxHome.isEmpty()) {
        QDir().mkpath(sandboxHome);
        args << "--bind" << sandboxHome << sandboxHome;
        args << "--setenv" << "HOME" << sandboxHome;
    }

    // Namespaces aislados salvo red (necesaria para autenticación
    // Microsoft/Xbox y para multijugador).
    args << "--unshare-user" << "--unshare-pid" << "--unshare-ipc" << "--unshare-uts" << "--unshare-cgroup";
    args << "--share-net";

    // Si el launcher muere, el proceso aislado muere con él.
    args << "--die-with-parent";
    // Sesión de terminal nueva: evita propagación inesperada de señales.
    args << "--new-session";

    return args;
}

}  // namespace Beteliney
