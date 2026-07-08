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
#include <QString>

class QAction;
class QLineEdit;
class QListWidget;
class QMenu;
class QMenuBar;

namespace Beteliney {

/**
 * Paleta de comandos estilo VSCode/Sublime (Ctrl+K).
 *
 * Recorre recursivamente todas las QAction habilitadas y visibles del menú
 * principal (incluyendo submenús), y las junta con cualquier acción extra que
 * el caller quiera ofrecer (por ejemplo, quick-join a servidores favoritos).
 * Escribir filtra la lista; Enter o doble click confirma.
 *
 * Importante: este diálogo NUNCA dispara la acción elegida por sí mismo. Sólo
 * guarda cuál se eligió (selectedAction()) para que el caller la dispare
 * *después* de que exec() retorne y el diálogo ya se haya cerrado. Así se evita
 * cualquier ventana de reentrancia/use-after-free si la acción termina abriendo
 * otro diálogo o destruyendo objetos que el caller le pasó por punteros crudos.
 */
class CommandPaletteDialog : public QDialog {
  Q_OBJECT
public:
  /// extraActions: acciones adicionales (dueño es el caller) a ofrecer junto
  /// con las del menú.
  explicit CommandPaletteDialog(QMenuBar *menuBar,
                                const QList<QAction *> &extraActions,
                                QWidget *parent = nullptr);

  /// Acción elegida por el usuario, o nullptr si canceló (Escape / cerrar la
  /// ventana).
  QAction *selectedAction() const { return m_selectedAction; }

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void filterChanged(const QString &text);
  void itemActivated();

private:
  struct Entry {
    QAction *action;
    QString label;      // texto a mostrar, sin mnemonics ("&")
    QString searchable; // label en minúscula, para filtrar
  };

  void collectActions(QMenu *menu, const QString &prefix);
  void moveSelection(int delta);
  void confirmSelection();

  QList<Entry> m_entries;
  QAction *m_selectedAction = nullptr;

  QLineEdit *m_search;
  QListWidget *m_list;
};

} // namespace Beteliney
