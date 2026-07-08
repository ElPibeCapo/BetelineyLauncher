#pragma once

#include <QPointer>

#include "tasks/Task.h"

class MinecraftInstance;

/** Silent, no-UI check for available mod updates (Modrinth/CurseForge) on a single instance.
 *
 *  Unlike ResourceUpdateDialog, this never shows a dialog and never tries to generate metadata
 *  for mods that don't have it yet (that requires the user to pick a provider interactively).
 *  It only checks mods that already have metadata (Resource::status() != NO_METADATA), which is
 *  read synchronously from disk (.index/) during the folder scan, before this task ever queries it.
 *
 *  On completion (success, failure, or timeout), calls MinecraftInstance::setUpdateAvailable()
 *  so the existing "checkupdate" badge (see InstanceDelegate.cpp) reflects reality without the
 *  user having to open the Mods page and the update dialog manually.
 */
class BackgroundModUpdateCheckTask : public Task {
    Q_OBJECT
   public:
    explicit BackgroundModUpdateCheckTask(MinecraftInstance* instance);

   public slots:
    bool abort() override;

   protected:
    void executeTask() override;

   private:
    void onModListReady();

    // QPointer (not a raw pointer): if the user deletes/removes the instance while the
    // background network check is still in flight, this must become null automatically
    // instead of dangling, since instances are unique_ptr-owned in InstanceList (no shared
    // refcounting to keep them alive for us).
    QPointer<MinecraftInstance> m_instance;
    QMetaObject::Connection m_updateFinishedConn;
    bool m_aborted = false;
};
