// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "FavoriteServersDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace Beteliney {

namespace {

/// Mini-diálogo modal con dos campos (nombre + dirección). Devuelve true si el
/// usuario aceptó, en cuyo caso `server` queda con los valores ingresados.
bool promptServer(QWidget *parent, const QString &title,
                  FavoriteServer &server) {
  QDialog dlg(parent);
  dlg.setWindowTitle(title);
  dlg.setMinimumWidth(360);

  auto *layout = new QVBoxLayout(&dlg);
  auto *form = new QFormLayout();

  auto *nameEdit = new QLineEdit(server.name, &dlg);
  auto *addressEdit = new QLineEdit(server.address, &dlg);
  addressEdit->setPlaceholderText(QObject::tr("host o host:puerto"));

  form->addRow(QObject::tr("Nombre:"), nameEdit);
  form->addRow(QObject::tr("Dirección:"), addressEdit);
  layout->addLayout(form);

  auto *buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
  layout->addWidget(buttons);

  QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg,
                   &QDialog::accept);
  QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg,
                   &QDialog::reject);

  if (dlg.exec() != QDialog::Accepted)
    return false;

  QString address = addressEdit->text().trimmed();
  if (address.isEmpty()) {
    QMessageBox::warning(
        parent, QObject::tr("Falta la dirección"),
        QObject::tr("La dirección del servidor no puede estar vacía."));
    return false;
  }

  QString name = nameEdit->text().trimmed();
  server.name = name.isEmpty() ? address : name;
  server.address = address;
  return true;
}

} // namespace

FavoriteServersDialog::FavoriteServersDialog(SettingsObject *settings,
                                             QWidget *parent)
    : QDialog(parent), m_settings(settings) {
  setWindowTitle(tr("Servidores favoritos"));
  setMinimumSize(420, 360);

  auto *layout = new QVBoxLayout(this);

  m_list = new QListWidget(this);
  layout->addWidget(m_list);

  auto *buttonsRow = new QHBoxLayout();
  auto *btnAdd = new QPushButton(tr("Agregar..."), this);
  m_btnEdit = new QPushButton(tr("Editar..."), this);
  m_btnRemove = new QPushButton(tr("Eliminar"), this);
  m_btnEdit->setEnabled(false);
  m_btnRemove->setEnabled(false);

  buttonsRow->addWidget(btnAdd);
  buttonsRow->addWidget(m_btnEdit);
  buttonsRow->addWidget(m_btnRemove);
  buttonsRow->addStretch();
  layout->addLayout(buttonsRow);

  auto *closeButtons = new QDialogButtonBox(QDialogButtonBox::Close, this);
  layout->addWidget(closeButtons);

  connect(btnAdd, &QPushButton::clicked, this,
          &FavoriteServersDialog::addClicked);
  connect(m_btnEdit, &QPushButton::clicked, this,
          &FavoriteServersDialog::editClicked);
  connect(m_btnRemove, &QPushButton::clicked, this,
          &FavoriteServersDialog::removeClicked);
  connect(m_list, &QListWidget::itemSelectionChanged, this,
          &FavoriteServersDialog::selectionChanged);
  connect(m_list, &QListWidget::itemDoubleClicked, this,
          &FavoriteServersDialog::editClicked);
  connect(closeButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  reload();
}

void FavoriteServersDialog::reload() {
  m_servers = FavoriteServers::load(m_settings);

  m_list->clear();
  for (const auto &server : m_servers)
    new QListWidgetItem(QString("%1  (%2)").arg(server.name, server.address),
                        m_list);

  selectionChanged();
}

void FavoriteServersDialog::persist() {
  FavoriteServers::save(m_settings, m_servers);
}

void FavoriteServersDialog::selectionChanged() {
  bool hasSelection = m_list->currentRow() >= 0;
  m_btnEdit->setEnabled(hasSelection);
  m_btnRemove->setEnabled(hasSelection);
}

void FavoriteServersDialog::addClicked() {
  FavoriteServer server;
  if (!promptServer(this, tr("Agregar servidor favorito"), server))
    return;

  m_servers.append(server);
  persist();
  reload();
  m_list->setCurrentRow(m_list->count() - 1);
}

void FavoriteServersDialog::editClicked() {
  int row = m_list->currentRow();
  if (row < 0 || row >= m_servers.size())
    return;

  FavoriteServer server = m_servers[row];
  if (!promptServer(this, tr("Editar servidor favorito"), server))
    return;

  m_servers[row] = server;
  persist();
  reload();
  m_list->setCurrentRow(row);
}

void FavoriteServersDialog::removeClicked() {
  int row = m_list->currentRow();
  if (row < 0 || row >= m_servers.size())
    return;

  auto reply = QMessageBox::question(
      this, tr("Eliminar favorito"),
      tr("¿Eliminar \"%1\" de favoritos?").arg(m_servers[row].name));
  if (reply != QMessageBox::Yes)
    return;

  m_servers.removeAt(row);
  persist();
  reload();
}

} // namespace Beteliney
