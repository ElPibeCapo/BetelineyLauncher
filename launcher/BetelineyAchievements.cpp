// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "BetelineyAchievements.h"

#include "Application.h"
#include "BaseInstance.h"
#include "settings/SettingsObject.h"

namespace Beteliney {

namespace {
constexpr const char *kUnlockedSetting = "BrandAchievementsUnlocked";
}

Achievements &Achievements::instance() {
  static Achievements inst;
  return inst;
}

Achievements::Achievements(QObject *parent) : QObject(parent) {
  APPLICATION->settings()->registerSetting(kUnlockedSetting, QStringList());
}

const QList<Achievement> &Achievements::definitions() {
  // Umbrales en segundos. Nombres/descripciones sin acentos en los IDs
  // (persistidos como string plano), traducibles en name/description.
  static const QList<Achievement> defs = {
      {QStringLiteral("playtime_1h"), QObject::tr("Primeros Pasos"),
       QObject::tr("Jugá 1 hora en una misma instancia."), 3600},
      {QStringLiteral("playtime_10h"), QObject::tr("Explorador"),
       QObject::tr("Jugá 10 horas en una misma instancia."), 36000},
      {QStringLiteral("playtime_50h"), QObject::tr("Veterano"),
       QObject::tr("Jugá 50 horas en una misma instancia."), 180000},
      {QStringLiteral("playtime_100h"), QObject::tr("Centenario"),
       QObject::tr("Jugá 100 horas en una misma instancia."), 360000},
      {QStringLiteral("playtime_500h"), QObject::tr("Leyenda de Beteliney"),
       QObject::tr("Jugá 500 horas en una misma instancia."), 1800000},
  };
  return defs;
}

QStringList Achievements::unlocked() const {
  return APPLICATION->settings()->get(kUnlockedSetting).toStringList();
}

void Achievements::checkInstance(BaseInstance *instance) {
  if (!instance)
    return;

  const qint64 played = instance->totalTimePlayed();
  const QStringList already = unlocked();

  for (const auto &achievement : definitions()) {
    if (already.contains(achievement.id))
      continue;
    if (played >= achievement.thresholdSeconds)
      unlock(achievement);
  }
}

void Achievements::unlock(const Achievement &achievement) {
  auto settings = APPLICATION->settings();
  QStringList already = settings->get(kUnlockedSetting).toStringList();
  if (already.contains(achievement.id))
    return; // ya desbloqueado por otra instancia entre el check y acá

  already << achievement.id;
  settings->set(kUnlockedSetting, already);

  emit achievementUnlocked(achievement);
}

} // namespace Beteliney
