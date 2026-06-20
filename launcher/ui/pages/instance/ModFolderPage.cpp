// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *  Copyright (C) 2022 TheKodeToad <TheKodeToad@proton.me>
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

#include "ModFolderPage.h"
#include "minecraft/mod/Resource.h"
#include "ui/dialogs/ExportToModListDialog.h"
#include "ui/dialogs/InstallLoaderDialog.h"
#include "ui_ExternalResourcesPage.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <algorithm>
#include <memory>

#include "Application.h"

#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/ResourceDownloadDialog.h"
#include "ui/dialogs/ResourceUpdateDialog.h"

#include "minecraft/PackProfile.h"
#include "minecraft/VersionFilterData.h"
#include "minecraft/mod/Mod.h"
#include "minecraft/mod/ModFolderModel.h"

#include "tasks/ConcurrentTask.h"
#include "tasks/Task.h"
#include "ui/dialogs/ProgressDialog.h"

ModFolderPage::ModFolderPage(BaseInstance* inst, ModFolderModel* model, QWidget* parent)
    : ExternalResourcesPage(inst, model, parent), m_model(model)
{
    ui->actionDownloadItem->setText(tr("Descargar Mods"));
    ui->actionDownloadItem->setToolTip(tr("Descarga mods de plataformas online"));
    ui->actionDownloadItem->setEnabled(true);
    ui->actionsToolbar->insertActionBefore(ui->actionAddItem, ui->actionDownloadItem);

    connect(ui->actionDownloadItem, &QAction::triggered, this, &ModFolderPage::downloadMods);

    ui->actionUpdateItem->setToolTip(tr("Verificar o actualizar todos los mods seleccionados (todos si no hay selección)"));
    connect(ui->actionUpdateItem, &QAction::triggered, this, &ModFolderPage::updateMods);
    ui->actionsToolbar->insertActionBefore(ui->actionAddItem, ui->actionUpdateItem);

    auto updateMenu = new QMenu(this);

    auto update = updateMenu->addAction(tr("Buscar actualizaciones"));
    connect(update, &QAction::triggered, this, &ModFolderPage::updateMods);

    updateMenu->addAction(ui->actionVerifyItemDependencies);
    connect(ui->actionVerifyItemDependencies, &QAction::triggered, this, [this] { updateMods(true); });

    auto depsDisabled = APPLICATION->settings()->getSetting("ModDependenciesDisabled");
    ui->actionVerifyItemDependencies->setVisible(!depsDisabled->get().toBool());
    connect(depsDisabled.get(), &Setting::SettingChanged, this,
            [this](const Setting&, const QVariant& value) { ui->actionVerifyItemDependencies->setVisible(!value.toBool()); });

    updateMenu->addAction(ui->actionResetItemMetadata);
    connect(ui->actionResetItemMetadata, &QAction::triggered, this, &ModFolderPage::deleteModMetadata);

    ui->actionUpdateItem->setMenu(updateMenu);

    ui->actionChangeVersion->setToolTip(tr("Cambiar la versión de un mod."));
    connect(ui->actionChangeVersion, &QAction::triggered, this, &ModFolderPage::changeModVersion);
    ui->actionsToolbar->insertActionAfter(ui->actionUpdateItem, ui->actionChangeVersion);

    ui->actionViewHomepage->setToolTip(tr("Ver la página web de los mods seleccionados."));

    ui->actionExportMetadata->setToolTip(tr("Exportar metadatos del mod a texto."));
    connect(ui->actionExportMetadata, &QAction::triggered, this, &ModFolderPage::exportModMetadata);
    ui->actionsToolbar->insertActionAfter(ui->actionViewHomepage, ui->actionExportMetadata);

    ui->actionsToolbar->insertActionAfter(ui->actionViewFolder, ui->actionViewConfigs);
}

bool ModFolderPage::shouldDisplay() const
{
    return true;
}

void ModFolderPage::updateFrame(const QModelIndex& current, [[maybe_unused]] const QModelIndex& previous)
{
    auto sourceCurrent = m_filterModel->mapToSource(current);
    int row = sourceCurrent.row();
    const Mod& mod = m_model->at(row);
    ui->frame->updateWithMod(mod);
}

void ModFolderPage::removeItems(const QItemSelection& selection)
{
    if (m_instance != nullptr && m_instance->isRunning()) {
        auto response = CustomMessageBox::selectable(this, tr("Confirmar eliminación"),
                                                     tr("Si eliminas mods mientras el juego está corriendo puede crashear.\n"
                                                        "¿Seguro que quieres hacer esto?"),
                                                     QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                            ->exec();

        if (response != QMessageBox::Yes)
            return;
    }

    auto indexes = selection.indexes();
    auto affected = m_model->getAffectedMods(indexes, EnableAction::DISABLE);
    if (!affected.isEmpty()) {
        auto response = CustomMessageBox::selectable(this, tr("Confirm Disable"),
                                                     tr("The mods you are trying to delete are required by %1 mods.\n"
                                                        "Do you want to disable them?")
                                                         .arg(affected.length()),
                                                     QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                                     QMessageBox::Cancel)
                            ->exec();

        if (response != QMessageBox::Yes) {
            m_model->setResourceEnabled(affected, EnableAction::DISABLE);
        } else if (response != QMessageBox::Cancel) {
            return;
        }
    }
    m_model->deleteResources(indexes);
}

void ModFolderPage::downloadMods()
{
    if (m_instance->typeName() != "Minecraft")
        return;  // this is a null instance or a legacy instance

    auto profile = static_cast<MinecraftInstance*>(m_instance)->getPackProfile();
    if (!profile->getModLoaders().has_value()) {
        if (handleNoModLoader()) {
            return;
        }
    }

    m_downloadDialog = new ResourceDownload::ModDownloadDialog(this, m_model, m_instance);
    connect(this, &QObject::destroyed, m_downloadDialog, &QDialog::close);
    connect(m_downloadDialog, &QDialog::finished, this, &ModFolderPage::downloadDialogFinished);

    m_downloadDialog->open();
}

void ModFolderPage::downloadDialogFinished(int result)
{
    if (result) {
        auto tasks = new ConcurrentTask(tr("Descargar Mods"), APPLICATION->settings()->get("NumberOfConcurrentDownloads").toInt());
        connect(tasks, &Task::failed, [this, tasks](QString reason) {
            CustomMessageBox::selectable(this, tr("Error"), reason, QMessageBox::Critical)->show();
            tasks->deleteLater();
        });
        connect(tasks, &Task::aborted, [this, tasks]() {
            CustomMessageBox::selectable(this, tr("Cancelado"), tr("Descarga detenida por el usuario."), QMessageBox::Information)->show();
            tasks->deleteLater();
        });
        connect(tasks, &Task::succeeded, [this, tasks]() {
            QStringList warnings = tasks->warnings();
            if (warnings.count())
                CustomMessageBox::selectable(this, tr("Advertencias"), warnings.join('\n'), QMessageBox::Warning)->show();

            tasks->deleteLater();
        });

        if (m_downloadDialog) {
            for (auto& task : m_downloadDialog->getTasks()) {
                tasks->addTask(task);
            }
        } else {
            qWarning() << "ResourceDownloadDialog vanished before we could collect tasks!";
        }

        ProgressDialog loadDialog(this);
        loadDialog.setSkipButton(true, tr("Cancelar"));
        loadDialog.execWithTask(tasks);

        m_model->update();
    }
    if (m_downloadDialog)
        m_downloadDialog->deleteLater();
}

void ModFolderPage::updateMods(bool includeDeps)
{
    if (m_instance->typeName() != "Minecraft")
        return;  // this is a null instance or a legacy instance

    auto profile = static_cast<MinecraftInstance*>(m_instance)->getPackProfile();
    if (!profile->getModLoaders().has_value()) {
        if (handleNoModLoader()) {
            return;
        }
    }
    if (APPLICATION->settings()->get("ModMetadataDisabled").toBool()) {
        QMessageBox::critical(this, tr("Error"), tr("¡Las actualizaciones de mods no están disponibles cuando los metadatos están desactivados!"));
        return;
    }
    if (m_instance != nullptr && m_instance->isRunning()) {
        auto response =
            CustomMessageBox::selectable(this, tr("Confirmar actualización"),
                                         tr("Actualizar mods mientras el juego está corriendo puede causar duplicación de mods y crashes.\n"
                                            "Los archivos viejos pueden no borrarse porque están en uso.\n"
                                            "¿Seguro que quieres hacer esto?"),
                                         QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                ->exec();

        if (response != QMessageBox::Yes)
            return;
    }
    auto selection = m_filterModel->mapSelectionToSource(ui->treeView->selectionModel()->selection()).indexes();

    auto mods_list = m_model->selectedResources(selection);
    bool use_all = mods_list.empty();
    if (use_all)
        mods_list = m_model->allResources();

    ResourceUpdateDialog update_dialog(this, m_instance, m_model, mods_list, includeDeps, profile->getModLoadersList());
    update_dialog.checkCandidates();

    if (update_dialog.aborted()) {
        CustomMessageBox::selectable(this, tr("Cancelado"), tr("¡El actualizador de mods fue cancelado!"), QMessageBox::Warning)->show();
        return;
    }
    if (update_dialog.noUpdates()) {
        QString message{ tr("'%1' está al día. :)").arg(mods_list.front()->name()) };
        if (mods_list.size() > 1) {
            if (use_all) {
                message = tr("¡Todos los mods están al día! :)");
            } else {
                message = tr("¡Todos los mods seleccionados están al día! :)");
            }
        }
        CustomMessageBox::selectable(this, tr("Verificador de actualizaciones"), message)->exec();
        return;
    }

    if (update_dialog.exec()) {
        auto tasks = new ConcurrentTask("Descargar Mods", APPLICATION->settings()->get("NumberOfConcurrentDownloads").toInt());
        connect(tasks, &Task::failed, [this, tasks](QString reason) {
            CustomMessageBox::selectable(this, tr("Error"), reason, QMessageBox::Critical)->show();
            tasks->deleteLater();
        });
        connect(tasks, &Task::aborted, [this, tasks]() {
            CustomMessageBox::selectable(this, tr("Cancelado"), tr("Descarga detenida por el usuario."), QMessageBox::Information)->show();
            tasks->deleteLater();
        });
        connect(tasks, &Task::succeeded, [this, tasks]() {
            QStringList warnings = tasks->warnings();
            if (warnings.count()) {
                CustomMessageBox::selectable(this, tr("Advertencias"), warnings.join('\n'), QMessageBox::Warning)->show();
            }
            tasks->deleteLater();
        });

        for (auto task : update_dialog.getTasks()) {
            tasks->addTask(task);
        }

        ProgressDialog loadDialog(this);
        loadDialog.setSkipButton(true, tr("Cancelar"));
        loadDialog.execWithTask(tasks);

        m_model->update();
    }
}

void ModFolderPage::deleteModMetadata()
{
    auto selection = m_filterModel->mapSelectionToSource(ui->treeView->selectionModel()->selection()).indexes();
    auto selectionCount = m_model->selectedMods(selection).length();
    if (selectionCount == 0)
        return;
    if (selectionCount > 1) {
        auto response = CustomMessageBox::selectable(this, tr("Confirmar eliminación"),
                                                     tr("Vas a eliminar los metadatos de %1 mods.\n"
                                                        "¿Seguro?")
                                                         .arg(selectionCount),
                                                     QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                            ->exec();

        if (response != QMessageBox::Yes)
            return;
    }

    m_model->deleteMetadata(selection);
}

void ModFolderPage::changeModVersion()
{
    if (m_instance->typeName() != "Minecraft")
        return;  // this is a null instance or a legacy instance

    auto profile = static_cast<MinecraftInstance*>(m_instance)->getPackProfile();
    if (!profile->getModLoaders().has_value()) {
        if (handleNoModLoader()) {
            return;
        }
    }
    if (APPLICATION->settings()->get("ModMetadataDisabled").toBool()) {
        QMessageBox::critical(this, tr("Error"), tr("¡Las actualizaciones de mods no están disponibles cuando los metadatos están desactivados!"));
        return;
    }
    auto selection = m_filterModel->mapSelectionToSource(ui->treeView->selectionModel()->selection()).indexes();
    auto mods_list = m_model->selectedMods(selection);
    if (mods_list.length() != 1 || mods_list[0]->metadata() == nullptr)
        return;

    m_downloadDialog = new ResourceDownload::ModDownloadDialog(this, m_model, m_instance);
    connect(this, &QObject::destroyed, m_downloadDialog, &QDialog::close);
    connect(m_downloadDialog, &QDialog::finished, this, &ModFolderPage::downloadDialogFinished);

    m_downloadDialog->setResourceMetadata((*mods_list.begin())->metadata());
    m_downloadDialog->open();
}

void ModFolderPage::exportModMetadata()
{
    auto selection = m_filterModel->mapSelectionToSource(ui->treeView->selectionModel()->selection()).indexes();
    auto selectedMods = m_model->selectedMods(selection);
    if (selectedMods.length() == 0)
        selectedMods = m_model->allMods();

    std::sort(selectedMods.begin(), selectedMods.end(), [](const Mod* a, const Mod* b) { return a->name() < b->name(); });
    ExportToModListDialog dlg(m_instance->name(), selectedMods, this);
    dlg.exec();
}

CoreModFolderPage::CoreModFolderPage(BaseInstance* inst, ModFolderModel* mods, QWidget* parent) : ModFolderPage(inst, mods, parent)
{
    auto mcInst = dynamic_cast<MinecraftInstance*>(m_instance);
    if (mcInst) {
        auto version = mcInst->getPackProfile();
        if (version && version->getComponent("net.minecraftforge") && version->getComponent("net.minecraft")) {
            auto minecraftCmp = version->getComponent("net.minecraft");
            if (!minecraftCmp->m_loaded) {
                version->reload(Net::Mode::Offline);
                auto update = version->getCurrentTask();
                if (update) {
                    connect(update.get(), &Task::finished, this, [this] {
                        if (m_container) {
                            m_container->refreshContainer();
                        }
                    });
                    if (!update->isRunning()) {
                        update->start();
                    }
                }
            }
        }
    }
}

bool CoreModFolderPage::shouldDisplay() const
{
    if (ModFolderPage::shouldDisplay()) {
        auto inst = dynamic_cast<MinecraftInstance*>(m_instance);
        if (!inst)
            return true;

        auto version = inst->getPackProfile();
        if (!version || !version->getComponent("net.minecraftforge") || !version->getComponent("net.minecraft"))
            return false;
        auto minecraftCmp = version->getComponent("net.minecraft");
        return minecraftCmp->m_loaded && minecraftCmp->getReleaseDateTime() < g_VersionFilterData.legacyCutoffDate;
    }
    return false;
}

NilModFolderPage::NilModFolderPage(BaseInstance* inst, ModFolderModel* mods, QWidget* parent) : ModFolderPage(inst, mods, parent) {}

bool NilModFolderPage::shouldDisplay() const
{
    return m_model->dir().exists();
}

// Helper function so this doesn't need to be duplicated 3 times
inline bool ModFolderPage::handleNoModLoader()
{
    int resp =
        QMessageBox::question(this, this->tr("Cargador de mods no instalado"),
                              this->tr("Necesitás instalar un cargador de mods compatible antes de instalar mods. ¿Querés hacerlo ahora?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    switch (resp) {
        case QMessageBox::Yes: {
            // Should be safe
            auto profile = static_cast<MinecraftInstance*>(this->m_instance)->getPackProfile();
            InstallLoaderDialog dialog(profile, QString(), this);
            bool ret = dialog.exec();
            this->m_container->refreshContainer();

            // returning negation of dialog.exec which'll be true if the install loader dialog got canceled/closed
            // and false if the user went through and installed a loader
            return !ret;
        }
        case QMessageBox::No: {
            // Nothing happens the dialog is already closing
            // returning true so the caller doesn't go and continue with opening it's dialog without a mod loader
            return true;
        }
        default: {
            // Unreachable
            // returning true as a safety measure
            return true;
        }
    }
}
