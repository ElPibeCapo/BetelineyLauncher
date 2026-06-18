// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include <QDialog>
#include <QList>

#include "migration/GDLauncherMigrator.h"

class QCheckBox;
class QLabel;
class QListWidget;
class QPushButton;

namespace Beteliney {

/**
 * Diálogo de migración desde GDLauncher Carbon.
 * Se abre desde el menú Archivo → Importar → Desde GDLauncher Carbon.
 */
class GDLauncherMigrateDialog : public QDialog {
    Q_OBJECT
   public:
    explicit GDLauncherMigrateDialog(const QString& destInstancesDir,
                                     QWidget*       parent = nullptr);

   private slots:
    void scanClicked();
    void importClicked();

   private:
    QString              m_destDir;
    QString              m_dataDir;
    QList<GDInstance>    m_instances;

    QLabel*      m_statusLabel;
    QListWidget* m_list;
    QPushButton* m_btnScan;
    QPushButton* m_btnImport;
};

}  // namespace Beteliney
