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
// Rotada 2026-07-15 (sesion 40, segunda rotacion): la clave anterior
// (generada 2026-07-14 en la misma sesion) tambien se perdio - el .pem
// privado quedo en un directorio temporal que no sobrevivio a un corte de
// conexion antes de que se subiera a GitHub Secrets. Ningun release real
// corrio con firma desde que existe el mecanismo, asi que no hay impacto en
// produccion. Se genero un par nuevo con 'openssl genpkey -algorithm
// Ed25519', verificado con un roundtrip real firma/verificacion contra
// libsodium (crypto_sign_verify_detached, resultado 0) antes de commitear
// esta clave. Esta vez la privada SI se mostro una unica vez en el chat de
// la sesion (nunca escrita a un archivo de este repo) para que el usuario
// la copiara de inmediato a GitHub Secrets (Settings > Secrets and
// variables > Actions > RELEASE_SIGNING_KEY) sin depender de que un archivo
// temporal sobreviviera.
//
// Si algun dia se rota la clave otra vez, hay que actualizar esta constante Y
// el secret de CI a la vez, o el updater rechazara todos los releases nuevos.
constexpr unsigned char kReleasePublicKey[crypto_sign_PUBLICKEYBYTES] = {
    0x8a, 0x87, 0xbb, 0xce, 0xfa, 0xad, 0xe2, 0x08, 0x81, 0xb6, 0x20, 0x2c,
    0x2a, 0xd5, 0xf0, 0xbb, 0x77, 0x17, 0xf1, 0x60, 0x5a, 0xaa, 0x41, 0x3f,
    0x82, 0x2e, 0xf0, 0x63, 0x6b, 0xc0, 0x06, 0x49
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
