// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *  Copyright (c) 2025 Trial97 <alexandru.tripon97@gmail.com>
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
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");

 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */
#include "AnonymizeLog.h"

#include <QRegularExpression>

struct RegReplace {
    RegReplace(QRegularExpression r, QString w) : reg(std::move(r)), with(std::move(w)) { reg.optimize(); }
    QRegularExpression reg;
    QString with;
};

// Reglas aplicadas en orden. Las más específicas van primero para no solaparse.
static const QVector<RegReplace> anonymizeRules = {

    // ── Rutas del sistema operativo ─────────────────────────────────────────
    RegReplace(QRegularExpression("C:\\\\Users\\\\([^\\\\]+)\\\\", QRegularExpression::CaseInsensitiveOption),
               "C:\\Users\\********\\"),  // Windows backslash
    RegReplace(QRegularExpression("C:\\/Users\\/([^\\/]+)\\/", QRegularExpression::CaseInsensitiveOption),
               "C:/Users/********/"),  // Windows forward slash
    RegReplace(QRegularExpression("(?<!\\w)\\/home\\/[^\\/]+\\/", QRegularExpression::CaseInsensitiveOption),
               "/home/********/"),  // Linux
    RegReplace(QRegularExpression("(?<!\\w)\\/Users\\/[^\\/]+\\/", QRegularExpression::CaseInsensitiveOption),
               "/Users/********/"),  // macOS
    // AppData: grupos de captura para preservar el separador original (/ o \\)
    RegReplace(QRegularExpression("AppData([/\\\\])([^/\\\\]+)([/\\\\])",
                                  QRegularExpression::CaseInsensitiveOption),
               "AppData\\1<SUBDIR>\\3"),

    // ── Tokens de autenticación de Microsoft/Mojang ──────────────────────────
    RegReplace(QRegularExpression("\\(Session ID is [^\\)]+\\)", QRegularExpression::CaseInsensitiveOption),
               "(Session ID is <SESSION_TOKEN>)"),
    RegReplace(QRegularExpression("new refresh token: \"[^\"]+\"", QRegularExpression::CaseInsensitiveOption),
               "new refresh token: \"<TOKEN>\""),
    RegReplace(QRegularExpression("\"device_code\" :  \"[^\"]+\"", QRegularExpression::CaseInsensitiveOption),
               "\"device_code\" :  \"<DEVICE_CODE>\""),
    RegReplace(QRegularExpression("(?:accessToken|access_token)[\"\"\\s:=]+[A-Za-z0-9+/=_\\-]{20,}",
                                  QRegularExpression::CaseInsensitiveOption),
               "accessToken: <ACCESS_TOKEN>"),
    // Bearer tokens en cabeceras HTTP (aparecen en logs de red)
    RegReplace(QRegularExpression("Bearer [A-Za-z0-9+/=_\\-\\.]{20,}",
                                  QRegularExpression::CaseInsensitiveOption),
               "Bearer <BEARER_TOKEN>"),
    // client_secret de OAuth (nunca debería estar en logs, pero por si acaso)
    RegReplace(QRegularExpression("(?:client_secret)[\"\"\\s:=]+[A-Za-z0-9+/=_\\-]{8,}",
                                  QRegularExpression::CaseInsensitiveOption),
               "client_secret: <CLIENT_SECRET>"),

    // ── UUIDs contextualizados (solo auth — no seeds, mod IDs, server IDs) ──
    // "id" suelto es demasiado genérico ("seed id=", "world id=", etc.).
    // Solo se anonimiza cuando el keyword está en contexto JSON/auth claro.
    RegReplace(QRegularExpression(
                   "(?:player|uuid|playerId|selectedProfile|\"id\")[\"\"\\s:=]+([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})",
                   QRegularExpression::CaseInsensitiveOption),
               "uuid: <UUID>"),

    // ── Datos de red privados ────────────────────────────────────────────────
    // IPs privadas (192.168.x.x, 10.x.x.x, 172.16-31.x.x) — identifican la red local
    RegReplace(QRegularExpression(
                   "\\b(?:192\\.168\\.[0-9]{1,3}|10\\.[0-9]{1,3}\\.[0-9]{1,3}|172\\.(?:1[6-9]|2[0-9]|3[01])\\.[0-9]{1,3})\\.[0-9]{1,3}\\b"),
               "<LOCAL_IP>"),

    // ── Credenciales en argumentos de línea de comandos ─────────────────────
    // --username cubre espacio, = y tabulador como separador
    RegReplace(QRegularExpression("--username[\\s=]+[^\\s]+",
                                  QRegularExpression::CaseInsensitiveOption),
               "--username <USERNAME>"),
    // --accessToken en args de launch
    RegReplace(QRegularExpression("--accessToken[\\s=]+[^\\s]+",
                                  QRegularExpression::CaseInsensitiveOption),
               "--accessToken <ACCESS_TOKEN>"),

    // ── Emails (pueden aparecer en respuestas OAuth o en logs de error) ──────
    RegReplace(QRegularExpression("[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}"),
               "<EMAIL>"),
};

void anonymizeLog(QString& log)
{
    for (const auto& rule : anonymizeRules) {
        log.replace(rule.reg, rule.with);
    }
}
