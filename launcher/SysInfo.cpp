
// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *  Copyright (C) 2022 r58Playz <r58playz@gmail.com>
 *  Copyright (C) 2024 timoreo <contact@timoreo.fr>
 *  Copyright (C) 2024 Trial97 <alexandru.tripon97@gmail.com>
 *  Copyright (C) 2025 TheKodeToad <TheKodeToad@proton.me>
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

#include <QFile>
#include <QMap>
#include <QProcess>
#include <QStandardPaths>
#include <QString>
#include "HardwareInfo.h"

#if defined(Q_OS_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <sys/sysinfo.h>
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD)
#include <cstdio>
#elif defined(Q_OS_APPLE)
#include <sys/sysctl.h>
#endif

#ifdef Q_OS_MACOS
bool rosettaDetect()
{
    int ret = 0;
    size_t size = sizeof(ret);
    if (sysctlbyname("sysctl.proc_translated", &ret, &size, nullptr, 0) == -1) {
        return false;
    }
    return ret == 1;
}
#endif

namespace SysInfo {
QString currentSystem()
{
#if defined(Q_OS_LINUX)
    return "linux";
#elif defined(Q_OS_MACOS)
    return "osx";
#elif defined(Q_OS_WINDOWS)
    return "windows";
#elif defined(Q_OS_FREEBSD)
    return "freebsd";
#elif defined(Q_OS_OPENBSD)
    return "openbsd";
#else
    return "unknown";
#endif
}

QString useQTForArch()
{
#if defined(Q_OS_MACOS) && !defined(Q_PROCESSOR_ARM)
    if (rosettaDetect()) {
        return "arm64";
    } else {
        return "x86_64";
    }
#endif
    return QSysInfo::currentCpuArchitecture();
}

uint64_t getSystemRamMiB()
{
#if defined(Q_OS_WINDOWS)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof status;

    if (GlobalMemoryStatusEx(&status)) {
        // transforming bytes -> mib
        return (uint64_t)status.ullTotalPhys / 1024 / 1024;
    }
#elif defined(Q_OS_LINUX)
    struct sysinfo info;

    if (sysinfo(&info) != -1) {
        // transforming bytes -> mib
        return info.totalram / 1024 / 1024;
    }
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD)
    char buff[512];
    FILE* fp = popen("sysctl hw.physmem", "r");
    if (fp != nullptr) {
        if (fgets(buff, 512, fp) != nullptr) {
            std::string str(buff);
            uint64_t mem = std::stoull(str.substr(12, std::string::npos));

            // transforming kib -> mib
            return mem / 1024;
        }
    }
#elif defined(Q_OS_APPLE)
    uint64_t memsize;
    size_t memsizesize = sizeof memsize;

    if (!sysctlbyname("hw.memsize", &memsize, &memsizesize, nullptr, 0)) {
        // transforming bytes -> mib
        return memsize / 1024 / 1024;
    }
#elif defined(__GNUC__) || defined(__clang__)
#warning getSystemRam not implemented on this platform; detecting amount of installed RAM will not work
#endif
    return 0;
}

int suitableMaxMem()
{
    int totalRAM = static_cast<int>(HardwareInfo::totalRamMiB());

    // Beteliney: valor predeterminado conservador para la primera instalación.
    // Reserva margen para:
    //   - El sistema operativo       (~2 GB en Windows 11)
    //   - GPU integrada (iGPU)       (~1-2 GB de RAM del sistema en APUs AMD/Intel)
    //   - Navegador y otras apps     (~1-2 GB)
    // Con GPU dedicada (dGPU) el usuario puede aumentarlo manualmente.
    //
    // Escala (pensada para sistemas con iGPU, margen de ~6 GB reservado):
    //   ≥32 GB → 10 GB para MC  (queda ~22 GB para SO + iGPU + apps)
    //   ≥16 GB →  5 GB para MC  (queda ~11 GB; iGPU toma 2 GB, SO toma 2 GB)
    //   ≥12 GB →  4 GB para MC  (queda ~8 GB; razonable para iGPU + SO + apps)
    //   ≥ 8 GB →  2 GB para MC  (queda ~6 GB; justo para iGPU + SO + una app)
    //   ≥ 4 GB →  1 GB para MC  (sistemas muy limitados)
    //    < 4 GB → 40% de la RAM total (mínimo absoluto)
    if (totalRAM >= 32768)
        return 10240;  // 10 GB
    else if (totalRAM >= 16384)
        return 5120;   // 5 GB (conservador para iGPU en sistemas de 16 GB)
    else if (totalRAM >= 12288)
        return 4096;   // 4 GB (sistemas de 12 GB con iGPU)
    else if (totalRAM >= 8192)
        return 2048;   // 2 GB
    else if (totalRAM >= 4096)
        return 1024;   // 1 GB
    else
        return qMax(512, totalRAM * 2 / 5);  // 40% mínimo 512 MB
}

QString getSupportedJavaArchitecture()
{
    auto sys = currentSystem();
    auto arch = useQTForArch();
    if (sys == "windows") {
        if (arch == "x86_64")
            return "windows-x64";
        if (arch == "i386")
            return "windows-x86";
        // Unknown, maybe arm, appending arch
        return "windows-" + arch;
    }
    if (sys == "osx") {
        if (arch == "arm64")
            return "mac-os-arm64";
        if (arch.contains("64"))
            return "mac-os-x64";
        if (arch.contains("86"))
            return "mac-os-x86";
        // Unknown, maybe something new, appending arch
        return "mac-os-" + arch;
    } else if (sys == "linux") {
        if (arch == "x86_64")
            return "linux-x64";
        if (arch == "i386")
            return "linux-x86";
        // will work for arm32 arm(64)
        return "linux-" + arch;
    }
    return {};
}
}  // namespace SysInfo
