// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *
 *  Easter egg: escribe B-E-T-E en la ventana principal para activarlo.
 */
#pragma once

#include <QKeyEvent>

class BetelineyCode : public QObject {
    Q_OBJECT
   public:
    BetelineyCode(QObject* parent = nullptr);
    void input(QEvent* event);

   signals:
    void triggered();

   private:
    int m_progress = 0;
};
