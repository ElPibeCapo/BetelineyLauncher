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

#pragma once

#include <QString>
#include <QStringList>

namespace Beteliney {

/**
 * Sandboxing opcional del proceso de Minecraft con Bubblewrap (bwrap).
 *
 * Objetivo (threat model de sesión 30/35): un mod malicioso corriendo dentro
 * del proceso Java del juego no debe poder leer archivos sensibles del $HOME
 * real del usuario (tokens de sesión de Discord, cookies de navegador, claves
 * SSH, etc.). Solo Linux — bwrap no existe en Windows/macOS.
 *
 * Diseño: namespaces aislados (usuario, PID, IPC, UTS, cgroup) compartiendo
 * únicamente red (necesaria para autenticación Microsoft/Xbox y multijugador).
 * $HOME se reemplaza por un directorio vacío dentro de la propia instancia.
 * El sistema base (/usr, /etc, /lib...) se monta solo-lectura para que Java y
 * las bibliotecas nativas (LWJGL/GLFW/OpenAL) encuentren lo que necesitan sin
 * poder escribir fuera del directorio de la instancia.
 */
class BubblewrapSandbox {
   public:
    /** true si el binario 'bwrap' está disponible en el PATH del sistema. */
    static bool isAvailable();

    /**
     * Construye los argumentos de bwrap (sin incluir "bwrap" en sí, ni el
     * comando a ejecutar dentro — eso se agrega aparte).
     *
     * @param instanceRoot directorio raíz de la instancia (saves, mods,
     *        config, resourcepacks, screenshots, logs, natives extraídos).
     *        Se monta con acceso de escritura real.
     * @param javaPath ejecutable de Java ya resuelto; se usa para montar
     *        también su carpeta contenedora, por si vive fuera de los
     *        directorios del sistema (JDKs portables bajo el data dir).
     * @param sandboxHome directorio (se crea si no existe) usado como $HOME
     *        falso dentro del sandbox.
     */
    static QStringList buildArgs(const QString& instanceRoot, const QString& javaPath, const QString& sandboxHome);
};

}  // namespace Beteliney
