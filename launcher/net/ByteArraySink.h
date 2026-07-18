// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *  Copyright (c) 2022 flowln <flowlnlnln@gmail.com>
 *  Copyright (c) 2023 Trial97 <alexandru.tripon97@gmail.com>
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

#pragma once

#include "Sink.h"

namespace Net {

/*
 * Sink object for downloads that uses an owned QByteArray as a target.
 */
class ByteArraySink : public Sink {
   public:
    virtual ~ByteArraySink() = default;

   public:
    auto init(QNetworkRequest& request) -> Task::State override
    {
        m_output.clear();
        if (initAllValidators(request))
            return Task::State::Running;
        m_fail_reason = "Failed to initialize validators";
        return Task::State::Failed;
    };

    auto write(QByteArray& data) -> Task::State override
    {
        // Auditoría de seguridad (sesión 45, meta server / NetRequest): sin este tope,
        // cualquier respuesta HTTP dirigida a un ByteArraySink (auth MSA, búsquedas de
        // mods CurseForge/Modrinth/ATLauncher/Technic/FTB, manifests de versión Minecraft,
        // BetelineyPacks) se acumulaba entera en memoria sin límite -> agotamiento de RAM
        // ante un servidor comprometido o una respuesta inesperadamente grande. No cubre
        // descargas de archivos de mods: esas usan FileSink (streaming a disco), no este.
        // 64 MB deja margen de sobra sobre cualquier JSON de listado/manifest real visto
        // en este codebase (el más pesado documentado, meta/index.json, pesa ~1.7 MB).
        static constexpr qint64 MAX_BYTEARRAY_SINK_BYTES = 64 * 1024 * 1024;
        if (m_output.size() + data.size() > MAX_BYTEARRAY_SINK_BYTES) {
            m_fail_reason = QString("Response exceeds %1 MB limit, aborting").arg(MAX_BYTEARRAY_SINK_BYTES / (1024 * 1024));
            return Task::State::Failed;
        }
        m_output.append(data);
        if (writeAllValidators(data))
            return Task::State::Running;
        m_fail_reason = "Failed to write validators";
        return Task::State::Failed;
    }

    auto abort() -> Task::State override
    {
        failAllValidators();
        m_fail_reason = "Aborted";
        return Task::State::Failed;
    }

    auto finalize(QNetworkReply& reply) -> Task::State override
    {
        if (finalizeAllValidators(reply))
            return Task::State::Succeeded;
        m_fail_reason = "Failed to finalize validators";
        return Task::State::Failed;
    }

    auto hasLocalData() -> bool override { return false; }

    QByteArray* output() { return &m_output; }

   protected:
    QByteArray m_output;
};
}  // namespace Net
