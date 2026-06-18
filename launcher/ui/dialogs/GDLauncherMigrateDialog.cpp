// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "GDLauncherMigrateDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace Beteliney {

GDLauncherMigrateDialog::GDLauncherMigrateDialog(const QString& destInstancesDir,
                                                  QWidget*       parent)
    : QDialog(parent), m_destDir(destInstancesDir)
{
    setWindowTitle(tr("Importar desde GDLauncher Carbon"));
    setMinimumSize(560, 420);

    auto* layout = new QVBoxLayout(this);

    m_statusLabel = new QLabel(
        tr("<b>Importar instancias desde GDLauncher Carbon</b><br>"
           "Detecta automáticamente la instalación de GDLauncher Carbon "
           "y convierte tus instancias al formato de BetelineyLauncher.<br>"
           "Los mods, mundos y configuraciones se copian sin modificar."));
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    m_btnScan = new QPushButton(tr("Detectar GDLauncher Carbon"), this);
    layout->addWidget(m_btnScan);

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::MultiSelection);
    m_list->setVisible(false);
    layout->addWidget(m_list);

    auto* buttons = new QDialogButtonBox(this);
    m_btnImport = buttons->addButton(tr("Importar seleccionadas"), QDialogButtonBox::AcceptRole);
    auto* btnClose = buttons->addButton(QDialogButtonBox::Close);
    m_btnImport->setEnabled(false);
    layout->addWidget(buttons);

    connect(m_btnScan,   &QPushButton::clicked,     this, &GDLauncherMigrateDialog::scanClicked);
    connect(m_btnImport, &QPushButton::clicked,     this, &GDLauncherMigrateDialog::importClicked);
    connect(btnClose,    &QPushButton::clicked,     this, &QDialog::reject);
    connect(m_list,      &QListWidget::itemSelectionChanged, this, [this] {
        m_btnImport->setEnabled(!m_list->selectedItems().isEmpty());
    });
}

void GDLauncherMigrateDialog::scanClicked()
{
    m_statusLabel->setText(tr("Buscando GDLauncher Carbon..."));
    m_list->clear();
    m_list->setVisible(false);
    m_btnImport->setEnabled(false);
    QApplication::processEvents();

    m_dataDir = gdlauncherDataDir();
    if (m_dataDir.isEmpty()) {
        m_statusLabel->setText(
            tr("<b>GDLauncher Carbon no encontrado.</b><br>"
               "Rutas buscadas:<br>"
               "• Linux: ~/.local/share/gdlauncher_next/<br>"
               "• Windows: %%APPDATA%%/gdlauncher_next/<br><br>"
               "Asegurate de que GDLauncher Carbon esté instalado y hayas iniciado "
               "sesión al menos una vez."));
        return;
    }

    QString error;
    m_instances = readGDInstances(m_dataDir, &error);

    if (m_instances.isEmpty()) {
        QString msg = tr("<b>No se encontraron instancias.</b>");
        if (!error.isEmpty())
            msg += tr("<br>Error: ") + error;
        else
            msg += tr("<br>Ruta de datos: ") + m_dataDir;
        m_statusLabel->setText(msg);
        return;
    }

    m_statusLabel->setText(
        tr("✔ Encontradas <b>%1 instancias</b> en <code>%2</code><br>"
           "Seleccioná las que querés importar (Ctrl+clic para múltiple):")
        .arg(m_instances.size())
        .arg(m_dataDir));

    for (const auto& inst : m_instances) {
        QString label = inst.name;
        if (!inst.mcVersion.isEmpty())
            label += QString(" — MC %1").arg(inst.mcVersion);
        if (!inst.loader.isEmpty() && inst.loader != "vanilla")
            label += QString(" [%1 %2]").arg(inst.loader, inst.loaderVersion);

        auto* item = new QListWidgetItem(label, m_list);
        item->setCheckState(Qt::Unchecked);
    }

    m_list->setVisible(true);
    // Seleccionar todo por defecto
    m_list->selectAll();
    m_btnImport->setEnabled(true);
}

void GDLauncherMigrateDialog::importClicked()
{
    // Recoger índices seleccionados
    QList<int> selected;
    for (int i = 0; i < m_list->count(); ++i) {
        if (m_list->item(i)->isSelected())
            selected << i;
    }

    if (selected.isEmpty())
        return;

    auto* progress = new QProgressDialog(
        tr("Importando instancias..."), tr("Cancelar"), 0, selected.size(), this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    progress->setValue(0);

    QStringList errors;
    int ok = 0;

    for (int i = 0; i < selected.size(); ++i) {
        if (progress->wasCanceled())
            break;

        const GDInstance& inst = m_instances[selected[i]];
        progress->setLabelText(tr("Importando: %1").arg(inst.name));
        progress->setValue(i);
        QApplication::processEvents();

        QString err = importGDInstance(inst, m_destDir);
        if (err.isEmpty())
            ++ok;
        else
            errors << QString("%1: %2").arg(inst.name, err);
    }

    progress->setValue(selected.size());

    QString msg;
    if (ok > 0)
        msg += tr("✔ %1 instancia(s) importadas correctamente.\n").arg(ok);
    if (!errors.isEmpty())
        msg += tr("⚠ Errores:\n") + errors.join("\n");

    if (ok > 0) {
        QMessageBox::information(this, tr("Importación completada"), msg +
            tr("\nReiniciá BetelineyLauncher para ver las instancias importadas."));
        accept();
    } else {
        QMessageBox::warning(this, tr("Error en importación"), msg);
    }
}

}  // namespace Beteliney
