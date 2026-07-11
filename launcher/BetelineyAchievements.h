// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

class BaseInstance;

namespace Beteliney {

struct Achievement {
  QString id;
  QString name;
  QString description;
  qint64 thresholdSeconds; // umbral de tiempo jugado (por instancia) para
                           // desbloquear
};

/**
 * Sistema de logros de marca (Beteliney Fase 3, sesión 25 → sesión 36).
 *
 * No agrega trackeo nuevo: reusa BaseInstance::totalTimePlayed(), ya
 * persistido por instancia vía SettingsObject (ver BaseInstance.cpp).
 * Esta clase solo agrega la capa de "¿ya cruzó un umbral que no tenía
 * desbloqueado?" más la notificación correspondiente.
 *
 * Persistencia GLOBAL (no por instancia, vía APPLICATION->settings()):
 * un logro de marca es del launcher en su conjunto, no de una instancia
 * puntual — si lo cruzás jugando el modpack A, sigue desbloqueado aunque
 * borres esa instancia.
 */
class Achievements : public QObject {
  Q_OBJECT
public:
  static Achievements &instance();

  /** Lista fija de logros disponibles, ordenada por umbral ascendente. */
  static const QList<Achievement> &definitions();

  /** IDs ya desbloqueados (persistidos globalmente). */
  QStringList unlocked() const;

  /**
   * Revisa el tiempo jugado de esta instancia contra los umbrales y
   * desbloquea + emite achievementUnlocked() por cada logro nuevo cruzado.
   * Llamar cuando una sesión de juego termina (el tiempo ya quedó
   * contabilizado en totalTimePlayed() en ese punto).
   */
  void checkInstance(BaseInstance *instance);

signals:
  void achievementUnlocked(const Achievement &achievement);

private:
  explicit Achievements(QObject *parent = nullptr);
  void unlock(const Achievement &achievement);
};

} // namespace Beteliney
