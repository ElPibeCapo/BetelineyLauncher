#pragma once
#include <cstdint>
#include <QString>

namespace SysInfo {
QString currentSystem();
QString useQTForArch();
QString getSupportedJavaArchitecture();
uint64_t getSystemRamMiB();
int suitableMaxMem();
}  // namespace SysInfo
