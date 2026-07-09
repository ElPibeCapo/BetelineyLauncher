// SPDX-FileCopyrightText: 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
//
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
 *
 */

#pragma once

#include <QByteArray>

// Verificacion criptografica de releases descargados por el auto-updater.
//
// Diseno: cada asset publicado en GitHub Releases va acompanado de un archivo
// "<asset>.sig" (firma Ed25519 detached, 64 bytes crudos) generado en el
// workflow de CI con la clave privada (secret RELEASE_SIGNING_KEY, jamas en el
// repo). Este modulo verifica esa firma contra la clave publica embebida abajo
// antes de permitir que el updater ejecute o desempaquete el asset.
//
// Politica fail-closed a proposito: si el asset no trae ".sig", o la firma no
// verifica, la actualizacion se rechaza. No hay bypass ni modo "advertir y
// continuar" - un asset sin firma valida no se instala, punto.
namespace UpdateVerify {

// Longitud de una firma Ed25519 detached (crypto_sign_BYTES en libsodium).
constexpr int kSignatureBytes = 64;

// Verifica que `signature` sea una firma Ed25519 valida de `fileData` bajo la
// clave publica embebida en UpdateVerify.cpp. Devuelve false ante cualquier
// problema (firma de tamano incorrecto, libsodium sin inicializar, firma que
// no matchea) - nunca lanza, nunca asume "valido" por default.
bool verifyReleaseSignature(const QByteArray& fileData, const QByteArray& signature);

}  // namespace UpdateVerify
