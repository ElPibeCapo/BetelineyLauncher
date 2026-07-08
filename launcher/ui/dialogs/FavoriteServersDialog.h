// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#pragma once

#include <QDialog>
#include <QList>

#include "FavoriteServers.h"

class QListWidget;
class QPushButton;
class SettingsObject;

namespace Beteliney {

/**
 * Gestión de servidores favoritos (agregar/editar/eliminar).
 *
 * Cada cambio se persiste de inmediato via FavoriteServers::save, así que
 * cerrar el diálogo con la X o con Cerrar da siempre el mismo resultado:
 * no hay un estado "sin guardar" que se pueda perder.
 */
class FavoriteServersDialog : public QDialog {
  Q_OBJECT
public:
  explicit FavoriteServersDialog(SettingsObject *settings,
                                 QWidget *parent = nullptr);

private slots:
  void addClicked();
  void editClicked();
  void removeClicked();
  void selectionChanged();

private:
  void reload();
  void persist();

  SettingsObject *m_settings;
  QList<FavoriteServer> m_servers;

  QListWidget *m_list;
  QPushButton *m_btnEdit;
  QPushButton *m_btnRemove;
};

} // namespace Beteliney
