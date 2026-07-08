#include "BackgroundModUpdateCheckTask.h"

#include <QDebug>
#include <QTimer>

#include "Application.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "minecraft/mod/ModFolderModel.h"
#include "minecraft/mod/Resource.h"
#include "modplatform/ModIndex.h"
#include "modplatform/flame/FlameCheckUpdate.h"
#include "modplatform/modrinth/ModrinthCheckUpdate.h"
#include "tasks/ConcurrentTask.h"

namespace {
std::vector<Version> mcVersionsFor(MinecraftInstance* inst)
{
    return { inst->getPackProfile()->getComponent("net.minecraft")->getVersion() };
}
}  // namespace

BackgroundModUpdateCheckTask::BackgroundModUpdateCheckTask(MinecraftInstance* instance) : Task(), m_instance(instance) {}

bool BackgroundModUpdateCheckTask::abort()
{
    m_aborted = true;
    disconnect(m_updateFinishedConn);
    emitFailed(tr("Aborted"));
    return true;
}

void BackgroundModUpdateCheckTask::executeTask()
{
    if (!m_instance || APPLICATION->settings()->get("ModMetadataDisabled").toBool()) {
        emitSucceeded();
        return;
    }

    auto* mod_list = m_instance->loaderModList();
    if (!mod_list) {
        emitSucceeded();
        return;
    }

    // Connect before calling update(): update() can return false if a scan is already in
    // progress, but in that case it still eventually emits updateFinished() once the scheduled
    // rescan completes (see ResourceFolderModel::update()), so connecting first is safe either way.
    auto handled = std::make_shared<bool>(false);
    m_updateFinishedConn = connect(mod_list, &ResourceFolderModel::updateFinished, this, [this, handled]() {
        if (*handled)
            return;
        *handled = true;
        disconnect(m_updateFinishedConn);
        if (!m_aborted)
            onModListReady();
    });

    // Safety net: if updateFinished() never arrives for some reason (e.g. createUpdateTask()
    // returns null), don't leave this task hanging in the background forever.
    QTimer::singleShot(30000, this, [this, handled]() {
        if (*handled)
            return;
        *handled = true;
        disconnect(m_updateFinishedConn);
        if (!m_aborted)
            emitSucceeded();
    });

    mod_list->update();
}

void BackgroundModUpdateCheckTask::onModListReady()
{
    if (!m_instance) {
        // Instance was deleted/invalidated while updateFinished() was queued but not yet
        // delivered. m_instance is a QPointer, so this check is reliable (not a dangling read).
        emitSucceeded();
        return;
    }

    auto* mod_list = m_instance->loaderModList();
    auto* profile = m_instance->getPackProfile();
    auto loaders = profile->getModLoadersList();

    if (loaders.isEmpty()) {
        // No mod loader installed, nothing to check.
        emitSucceeded();
        return;
    }

    QList<Resource*> modrinth_mods;
    QList<Resource*> flame_mods;
    for (auto* resource : mod_list->allResources()) {
        // Mods without metadata yet are skipped on purpose: generating metadata requires the
        // user to pick a provider interactively (see ResourceUpdateDialog::ensureMetadata()),
        // which this silent background check must never do.
        if (resource->status() == ResourceStatus::NO_METADATA || !resource->metadata())
            continue;

        switch (resource->metadata()->provider) {
            case ModPlatform::ResourceProvider::MODRINTH:
                modrinth_mods.push_back(resource);
                break;
            case ModPlatform::ResourceProvider::FLAME:
                flame_mods.push_back(resource);
                break;
        }
    }

    if (modrinth_mods.isEmpty() && flame_mods.isEmpty()) {
        emitSucceeded();
        return;
    }

    auto versions = mcVersionsFor(m_instance);
    auto check_task = makeShared<ConcurrentTask>("Background mod update check", 2);
    auto found_update = std::make_shared<bool>(false);

    if (!modrinth_mods.isEmpty()) {
        auto t = makeShared<ModrinthCheckUpdate>(modrinth_mods, versions, loaders, mod_list);
        connect(t.get(), &Task::succeeded, this, [t, found_update]() {
            if (!t->getUpdates().empty())
                *found_update = true;
        });
        connect(t.get(), &Task::failed, this, [](QString reason) {
            qDebug() << "Background mod update check (Modrinth) failed silently:" << reason;
        });
        check_task->addTask(t);
    }

    if (!flame_mods.isEmpty()) {
        auto t = makeShared<FlameCheckUpdate>(flame_mods, versions, loaders, mod_list);
        connect(t.get(), &Task::succeeded, this, [t, found_update]() {
            if (!t->getUpdates().empty())
                *found_update = true;
        });
        connect(t.get(), &Task::failed, this,
                [](QString reason) { qDebug() << "Background mod update check (Flame) failed silently:" << reason; });
        check_task->addTask(t);
    }

    auto instance = m_instance;
    connect(check_task.get(), &Task::finished, this, [this, check_task, found_update, instance]() {
        if (instance)
            instance->setUpdateAvailable(*found_update);
        emitSucceeded();
    });

    check_task->start();
}
