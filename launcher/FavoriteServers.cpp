// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "FavoriteServers.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "settings/SettingsObject.h"

namespace Beteliney {
namespace FavoriteServers {

namespace {
const char *SETTING_ID = "FavoriteServers";
}

QList<FavoriteServer> load(SettingsObject *settings) {
  QList<FavoriteServer> result;
  if (!settings)
    return result;

  auto raw = settings->get(SETTING_ID).toString();
  auto doc = QJsonDocument::fromJson(raw.toUtf8());
  if (!doc.isArray())
    return result;

  for (const auto &value : doc.array()) {
    if (!value.isObject())
      continue;

    auto obj = value.toObject();
    FavoriteServer server;
    server.address = obj.value("address").toString().trimmed();
    if (server.address.isEmpty())
      continue; // entrada corrupta, la ignoramos en vez de romper toda la lista

    server.name = obj.value("name").toString().trimmed();
    if (server.name.isEmpty())
      server.name = server.address;

    result.append(server);
  }
  return result;
}

void save(SettingsObject *settings, const QList<FavoriteServer> &servers) {
  if (!settings)
    return;

  QJsonArray array;
  for (const auto &server : servers) {
    QJsonObject obj;
    obj["name"] = server.name;
    obj["address"] = server.address;
    array.append(obj);
  }

  auto json = QJsonDocument(array).toJson(QJsonDocument::Compact);
  settings->set(SETTING_ID, QString::fromUtf8(json));
}

} // namespace FavoriteServers
} // namespace Beteliney
