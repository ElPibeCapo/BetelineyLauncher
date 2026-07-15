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

#include "UpdateVerify.h"

#include <sodium.h>

#include <QDebug>

namespace UpdateVerify {

// Clave publica Ed25519 (32 bytes crudos) del par usado para firmar los
// releases de BetelineyLauncher. NO es secreta - su unico rol es verificar.
// La clave PRIVADA correspondiente vive unicamente como secret de GitHub
// Actions (RELEASE_SIGNING_KEY) y no debe aparecer jamas en este repo.
//
// Rotada 2026-07-14 (sesion 40): la clave anterior (generada 2026-07-08) se
// perdio - el .pem privado que debia subirse a GitHub Secrets ya no existia
// en disco y no hay evidencia de que se haya llegado a cargar (ningun
// release real corrio con firma desde que existe el mecanismo). Se genero un
// par nuevo con 'openssl genpkey -algorithm Ed25519', verificado con un
// roundtrip real firma/verificacion contra libsodium antes de commitear esta
// clave. La privada NUNCA paso por este chat ni por ningun archivo de este
// repo - vive solo en el filesystem local del usuario, pendiente de que el
// la cargue el mismo en GitHub Secrets (Settings > Secrets and variables >
// Actions > RELEASE_SIGNING_KEY).
//
// Si algun dia se rota la clave otra vez, hay que actualizar esta constante Y
// el secret de CI a la vez, o el updater rechazara todos los releases nuevos.
constexpr unsigned char kReleasePublicKey[crypto_sign_PUBLICKEYBYTES] = {
    0xb1, 0xde, 0x15, 0x8f, 0xc5, 0xf2, 0x5e, 0xae, 0x9e, 0x84, 0x0d, 0xd1,
    0x3f, 0x05, 0xc6, 0x80, 0x68, 0x08, 0x26, 0x25, 0xba, 0x5d, 0x94, 0xd6,
    0x65, 0x36, 0x33, 0x32, 0x83, 0x88, 0x53, 0xbd
};

static_assert(sizeof(kReleasePublicKey) == 32, "La clave publica Ed25519 debe tener exactamente 32 bytes");

bool verifyReleaseSignature(const QByteArray& fileData, const QByteArray& signature)
{
    if (sodium_init() < 0) {
        qCritical() << "libsodium no pudo inicializarse - rechazando verificacion de firma por seguridad";
        return false;
    }

    if (signature.size() != kSignatureBytes) {
        qCritical() << "Firma de tamano incorrecto:" << signature.size() << "bytes, se esperaban" << kSignatureBytes;
        return false;
    }

    if (fileData.isEmpty()) {
        qCritical() << "Archivo vacio, no hay nada que verificar - rechazando";
        return false;
    }

    auto sig = reinterpret_cast<const unsigned char*>(signature.constData());
    auto msg = reinterpret_cast<const unsigned char*>(fileData.constData());

    int result = crypto_sign_verify_detached(sig, msg, static_cast<unsigned long long>(fileData.size()), kReleasePublicKey);

    if (result != 0) {
        qCritical() << "Verificacion de firma Ed25519 fallida - el archivo no coincide con la firma publicada";
        return false;
    }

    qDebug() << "Firma Ed25519 verificada correctamente (" << fileData.size() << "bytes )";
    return true;
}

}  // namespace UpdateVerify
