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

#include <QList>
#include <QString>

class SettingsObject;

namespace Beteliney {

/// Un servidor guardado en la lista de favoritos globales (independiente de
/// instancia).
struct FavoriteServer {
  QString name;
  QString
      address; // host[:puerto], mismo formato que MinecraftTarget::parse acepta
};

namespace FavoriteServers {

/// Carga la lista de favoritos desde el setting "FavoriteServers" (JSON). Nunca
/// falla: si el JSON está corrupto o vacío, devuelve una lista vacía.
QList<FavoriteServer> load(SettingsObject *settings);

/// Persiste la lista completa de favoritos, reemplazando lo que hubiera antes.
void save(SettingsObject *settings, const QList<FavoriteServer> &servers);

} // namespace FavoriteServers

} // namespace Beteliney
