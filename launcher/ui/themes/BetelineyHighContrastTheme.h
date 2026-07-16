// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 */
#pragma once

#include "BetelineyTheme.h"

/// @brief Variante de alto contraste del tema Beteliney, pensada para baja visión y daltonismo.
///
/// No es un tema nuevo desde cero: reutiliza toda la QSS de BetelineyTheme y la
/// recolorea de forma determinista (tabla fija, no heurística en runtime) aplanando
/// las docenas de tonos oscuros sutiles del tema base a 3 niveles planos:
///   - fondo  -> negro puro (#000000)
///   - borde/estado disabled -> gris medio consistente (#8C8C8C, ~4:1 contra negro)
///   - texto  -> blanco puro (#FFFFFF)
/// Los acentos neón (verde #39FF14 y variantes, cyan #00D4FF) se conservan intactos
/// porque ya cumplen contraste AA/AAA de sobra contra fondo negro.
/// Cualquier cambio futuro a BetelineyTheme::appStyleSheet() se propaga solo con
/// recompilar — este archivo no duplica QSS, solo la recolorea.
class BetelineyHighContrastTheme : public BetelineyTheme {
   public:
    virtual ~BetelineyHighContrastTheme() {}

    QString id() override;
    QString name() override;
    QString tooltip() override;
    QString appStyleSheet() override;
    QPalette colorScheme() override;
    double fadeAmount() override;
    QColor fadeColor() override;
};
