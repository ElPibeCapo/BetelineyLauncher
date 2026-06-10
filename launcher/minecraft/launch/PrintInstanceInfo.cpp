// SPDX-License-Identifier: Apache-2.0
/* Copyright 2013-2021 MultiMC Contributors
 * Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <launch/LaunchTask.h>
#include "HardwareInfo.h"
#include "PrintInstanceInfo.h"

void PrintInstanceInfo::executeTask()
{
    auto instance = m_parent->instance();
    QStringList log;

    log << "CPU: " + HardwareInfo::cpuInfo();
    log << QString("RAM: %1 MiB (available: %2 MiB)").arg(HardwareInfo::totalRamMiB()).arg(HardwareInfo::availableRamMiB());
    log.append(HardwareInfo::gpuInfo());

    logLines(log, MessageLevel::Launcher);
    logLines(instance->verboseDescription(m_session, m_targetToJoin), MessageLevel::Launcher);
    emitSucceeded();
}
