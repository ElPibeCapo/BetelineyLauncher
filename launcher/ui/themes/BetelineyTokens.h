// SPDX-FileCopyrightText: 2026 El_PibeCapo
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <QString>

/// Tokens de diseño de Beteliney. Fuente de verdad única para el QSS
/// (BetelineyTheme) y la QPalette. No añadir colores fuera de esta tabla.
///
/// Motivo (AUDITORIA_UI.md §C2): había 124 colores distintos escritos a mano
/// solo en BetelineyTheme.cpp, entre ellos siete verdes de acento y cinco
/// bordes casi idénticos. Nadie decidió tenerlos: aparecieron uno por uno al
/// ajustar gradientes. Con los tokens, cambiar el acento es cambiar una línea.
namespace BetelineyTokens {

// Superficies
constexpr auto kBg = "#080912";       // fondo base
constexpr auto kSurface = "#0C0E1A";  // paneles
constexpr auto kRaised = "#10121E";   // elementos elevados
constexpr auto kSunken = "#06070E";

// Bordes
constexpr auto kStroke = "#1E2235";
constexpr auto kStrokeDim = "#1A1D2E";
constexpr auto kStrokeHi = "#363A58";

// Texto — el comentario es la ratio de contraste medida contra kBg
constexpr auto kBright = "#E8ECF8";     // 16.81:1
constexpr auto kBody = "#C0C4D4";       // 11.42:1
constexpr auto kSecondary = "#A8ACBE";  //  7.99:1
constexpr auto kMuted = "#8890A8";      //  5.62:1 — mínimo permitido para texto
constexpr auto kDisabled = "#282C3E";   //  exento de WCAG (control inactivo)

// Acentos
constexpr auto kAccent = "#39FF14";  // 14.64:1
constexpr auto kAccentHi = "#55FF30";
constexpr auto kAccentDim = "#28CC0F";  //  9.20:1
constexpr auto kAccentBg = "#0A1E0C";
constexpr auto kCyan = "#00D4FF";  // 11.21:1
constexpr auto kDanger = "#FF4444";
constexpr auto kWarning = "#F5C211";

/// Sustituye los marcadores @token del QSS por su valor.
QString expand(QString css);

}  // namespace BetelineyTokens
