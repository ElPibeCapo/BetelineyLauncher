// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QWidget>

class QLabel;

/**
 * Notificación flotante no bloqueante para logros de marca desbloqueados
 * (Beteliney Fase 3). Se posiciona en la esquina inferior derecha del
 * widget padre y se oculta sola después de unos segundos.
 *
 * No hay antecedente de este patrón "toast" en el codebase (confirmado
 * en sesión 35/36) — es la primera pieza de UI de notificación efímera
 * del proyecto. Se mantiene deliberadamente simple: sin animación de
 * fade, solo mostrar/ocultar y encolar si llegan varios logros seguidos.
 */
class AchievementToast : public QWidget {
  Q_OBJECT
public:
  explicit AchievementToast(QWidget *parent);

  /** Encola un logro. Si ya hay uno visible, este se muestra después. */
  void showAchievement(const QString &title, const QString &description);

private slots:
  void showNext();

private:
  void reposition();

  QLabel *m_titleLabel = nullptr;
  QLabel *m_descriptionLabel = nullptr;
  QList<QPair<QString, QString>> m_queue;
  bool m_visible = false;
};
