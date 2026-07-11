// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "AchievementToast.h"

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

namespace {
constexpr int kDisplayMs = 6000;
constexpr int kMargin = 16;
} // namespace

AchievementToast::AchievementToast(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAttribute(Qt::WA_StyledBackground, true);
  setObjectName(QStringLiteral("AchievementToast"));
  setStyleSheet(
      "#AchievementToast { background-color: rgba(20, 20, 20, 225); "
      "border-radius: 8px; }"
      "#AchievementToast QLabel { color: white; background: transparent; }");

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(14, 10, 14, 10);
  layout->setSpacing(2);

  m_titleLabel = new QLabel(this);
  QFont titleFont = m_titleLabel->font();
  titleFont.setBold(true);
  m_titleLabel->setFont(titleFont);

  m_descriptionLabel = new QLabel(this);
  m_descriptionLabel->setWordWrap(true);
  m_descriptionLabel->setMaximumWidth(280);

  layout->addWidget(m_titleLabel);
  layout->addWidget(m_descriptionLabel);

  hide();
}

void AchievementToast::showAchievement(const QString &title,
                                       const QString &description) {
  m_queue.append({title, description});
  if (!m_visible)
    showNext();
}

void AchievementToast::showNext() {
  if (m_queue.isEmpty()) {
    m_visible = false;
    hide();
    return;
  }

  m_visible = true;
  auto next = m_queue.takeFirst();
  m_titleLabel->setText(tr("🏆 %1").arg(next.first));
  m_descriptionLabel->setText(next.second);

  adjustSize();
  reposition();
  show();
  raise();

  QTimer::singleShot(kDisplayMs, this, &AchievementToast::showNext);
}

void AchievementToast::reposition() {
  if (!parentWidget())
    return;
  const QRect parentRect = parentWidget()->rect();
  move(parentRect.width() - width() - kMargin,
       parentRect.height() - height() - kMargin);
}
