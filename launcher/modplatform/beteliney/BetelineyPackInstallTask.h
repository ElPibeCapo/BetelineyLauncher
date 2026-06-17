// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include "BetelineyPack.h"
#include "InstanceCreationTask.h"
#include "net/NetJob.h"

#include <QWidget>

namespace Beteliney {

/**
 * Instala un BetelineyPack creando una instancia de Minecraft con el
 * loader correcto y descargando cada mod en la carpeta mods.
 *
 * Hereda de InstanceCreationTask (no InstanceTask) — sigue el mismo
 * patrón que ModrinthCreationTask y FlameCreationTask.
 */
class PackInstallTask : public InstanceCreationTask {
    Q_OBJECT
public:
    explicit PackInstallTask(const Pack& pack,
                             const QString& stagingPath,
                             SettingsObject* globalSettings,
                             QWidget* parent = nullptr);

    bool abort() override;

protected:
    std::unique_ptr<MinecraftInstance> createInstance() override;

private:
    Pack m_pack;
    QWidget* m_parent = nullptr;
    Task::Ptr m_dlTask;
};

}  // namespace Beteliney
