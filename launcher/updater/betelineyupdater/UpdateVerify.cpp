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
// Generada 2026-07-08 (sesion de implementacion de firma del updater).
// Si algun dia se rota la clave, hay que actualizar esta constante Y el
// secret de CI a la vez, o el updater rechazara todos los releases nuevos.
constexpr unsigned char kReleasePublicKey[crypto_sign_PUBLICKEYBYTES] = {
    0x02, 0xfd, 0x06, 0x3b, 0xec, 0x0b, 0xf9, 0xd1, 0x1c, 0x4d, 0x88, 0x69, 0xdc, 0x23, 0x0e, 0xd6,
    0x19, 0x47, 0x1d, 0x9a, 0xbe, 0x9d, 0x1e, 0xf0, 0x23, 0x65, 0xa0, 0x09, 0xb8, 0x46, 0x28, 0xff
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
