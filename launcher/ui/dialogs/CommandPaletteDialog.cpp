// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "CommandPaletteDialog.h"

#include <QAction>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QVBoxLayout>

namespace Beteliney {

namespace {
constexpr int kActionRole = Qt::UserRole;
}

CommandPaletteDialog::CommandPaletteDialog(QMenuBar *menuBar,
                                           const QList<QAction *> &extraActions,
                                           QWidget *parent)
    : QDialog(parent) {
  setWindowTitle(tr("Paleta de comandos"));
  setMinimumSize(520, 400);

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(8, 8, 8, 8);
  layout->setSpacing(6);

  m_search = new QLineEdit(this);
  m_search->setPlaceholderText(tr("Escribí para buscar un comando..."));
  m_search->setClearButtonEnabled(true);
  layout->addWidget(m_search);

  m_list = new QListWidget(this);
  m_list->setAlternatingRowColors(true);
  layout->addWidget(m_list);

  // Acciones extra primero (ej: quick-join a favoritos), así quedan arriba en
  // la lista vacía.
  for (auto *action : extraActions) {
    if (!action || !action->isEnabled() || !action->isVisible())
      continue;
    Entry entry;
    entry.action = action;
    entry.label = action->text();
    entry.label.remove('&');
    entry.searchable = entry.label.toLower();
    m_entries.append(entry);
  }

  if (menuBar) {
    for (auto *action : menuBar->actions()) {
      if (auto *menu = action->menu())
        collectActions(menu, QString());
    }
  }

  filterChanged(QString());

  connect(m_search, &QLineEdit::textChanged, this,
          &CommandPaletteDialog::filterChanged);
  connect(m_list, &QListWidget::itemActivated, this,
          &CommandPaletteDialog::itemActivated);

  m_search->installEventFilter(this);
  m_search->setFocus();
}

void CommandPaletteDialog::collectActions(QMenu *menu, const QString &prefix) {
  QString menuLabel = menu->title();
  menuLabel.remove('&');
  QString newPrefix = prefix.isEmpty() ? menuLabel : prefix + " › " + menuLabel;

  for (auto *action : menu->actions()) {
    if (action->isSeparator())
      continue;

    if (auto *submenu = action->menu()) {
      collectActions(submenu, newPrefix);
      continue;
    }

    if (!action->isEnabled() || !action->isVisible())
      continue;
    if (action->text().isEmpty())
      continue;

    QString label = action->text();
    label.remove('&');

    Entry entry;
    entry.action = action;
    entry.label = newPrefix.isEmpty() ? label : newPrefix + " › " + label;
    entry.searchable = entry.label.toLower();
    m_entries.append(entry);
  }
}

void CommandPaletteDialog::filterChanged(const QString &text) {
  m_list->clear();

  QString needle = text.trimmed().toLower();

  for (const auto &entry : m_entries) {
    if (!needle.isEmpty() && !entry.searchable.contains(needle))
      continue;

    auto *item = new QListWidgetItem(entry.label, m_list);
    item->setData(kActionRole,
                  QVariant::fromValue(static_cast<void *>(entry.action)));
  }

  if (m_list->count() > 0)
    m_list->setCurrentRow(0);
}

void CommandPaletteDialog::itemActivated() { confirmSelection(); }

void CommandPaletteDialog::moveSelection(int delta) {
  if (m_list->count() == 0)
    return;

  int row = m_list->currentRow();
  if (row < 0)
    row = 0;

  row += delta;
  row = qBound(0, row, m_list->count() - 1);
  m_list->setCurrentRow(row);
}

void CommandPaletteDialog::confirmSelection() {
  auto *item = m_list->currentItem();
  if (!item) {
    reject();
    return;
  }

  m_selectedAction =
      static_cast<QAction *>(item->data(kActionRole).value<void *>());
  accept();
}

bool CommandPaletteDialog::eventFilter(QObject *obj, QEvent *event) {
  if (obj == m_search && event->type() == QEvent::KeyPress) {
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    switch (keyEvent->key()) {
    case Qt::Key_Down:
      moveSelection(1);
      return true;
    case Qt::Key_Up:
      moveSelection(-1);
      return true;
    case Qt::Key_Return:
    case Qt::Key_Enter:
      confirmSelection();
      return true;
    case Qt::Key_Escape:
      reject();
      return true;
    default:
      break;
    }
  }
  return QDialog::eventFilter(obj, event);
}

} // namespace Beteliney
