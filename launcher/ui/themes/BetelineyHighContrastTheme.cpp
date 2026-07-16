// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "BetelineyHighContrastTheme.h"
#include <QObject>
#include <utility>
#include <vector>

QString BetelineyHighContrastTheme::id() { return "beteliney-highcontrast"; }
QString BetelineyHighContrastTheme::name() { return QObject::tr("Beteliney Alto Contraste"); }
QString BetelineyHighContrastTheme::tooltip()
{
    return QObject::tr(
        "Variante de accesibilidad del tema Beteliney — negro/blanco planos, sin "
        "degradados sutiles, pensada para baja visión y daltonismo (contraste WCAG AA).");
}

double BetelineyHighContrastTheme::fadeAmount() { return 0.35; }
QColor BetelineyHighContrastTheme::fadeColor() { return QColor(0x00, 0x00, 0x00); }

QPalette BetelineyHighContrastTheme::colorScheme()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(0x00, 0x00, 0x00));
    p.setColor(QPalette::Base, QColor(0x00, 0x00, 0x00));
    p.setColor(QPalette::AlternateBase, QColor(0x00, 0x00, 0x00));
    p.setColor(QPalette::Button, QColor(0x00, 0x00, 0x00));
    p.setColor(QPalette::Mid, QColor(0x8C, 0x8C, 0x8C));
    p.setColor(QPalette::Dark, QColor(0x00, 0x00, 0x00));
    p.setColor(QPalette::Shadow, QColor(0x00, 0x00, 0x00));
    p.setColor(QPalette::WindowText, QColor(0xFF, 0xFF, 0xFF));
    p.setColor(QPalette::Text, QColor(0xFF, 0xFF, 0xFF));
    p.setColor(QPalette::ButtonText, QColor(0xFF, 0xFF, 0xFF));
    p.setColor(QPalette::BrightText, QColor(0xFF, 0x44, 0x44));
    p.setColor(QPalette::PlaceholderText, QColor(0x8C, 0x8C, 0x8C));
    p.setColor(QPalette::Highlight, QColor(0x39, 0xFF, 0x14));
    p.setColor(QPalette::HighlightedText, QColor(0x00, 0x00, 0x00));
    p.setColor(QPalette::Link, QColor(0x39, 0xFF, 0x14));
    p.setColor(QPalette::LinkVisited, QColor(0x00, 0xD4, 0xFF));
    p.setColor(QPalette::ToolTipBase, QColor(0x00, 0x00, 0x00));
    p.setColor(QPalette::ToolTipText, QColor(0xFF, 0xFF, 0xFF));
    return fadeInactive(p, fadeAmount(), fadeColor());
}

namespace {
const std::vector<std::pair<const char*, const char*>> kRecolorTable = {
    { "#060810", "#000000" }, { "#070909", "#000000" }, { "#080912", "#000000" },
    { "#080A12", "#000000" }, { "#080A14", "#000000" }, { "#081408", "#000000" },
    { "#090B13", "#000000" }, { "#090B14", "#000000" }, { "#091508", "#000000" },
    { "#0A1210", "#000000" }, { "#0A1A0C", "#000000" }, { "#0A1B0E", "#000000" },
    { "#0A1D0C", "#000000" }, { "#0A1E0C", "#000000" }, { "#0B0D18", "#000000" },
    { "#0C0E18", "#000000" }, { "#0C0E1A", "#000000" }, { "#0C0E1C", "#000000" },
    { "#0C0F1E", "#000000" }, { "#0C1E10", "#000000" }, { "#0C1F06", "#000000" },
    { "#0D0F1C", "#000000" }, { "#0D1018", "#000000" }, { "#0D101C", "#000000" },
    { "#0D1020", "#000000" }, { "#0D2010", "#000000" }, { "#0E1020", "#000000" },
    { "#0E1A12", "#8C8C8C" }, { "#0E2212", "#000000" }, { "#0E2410", "#000000" },
    { "#0E2412", "#000000" }, { "#0F111C", "#000000" }, { "#0F111E", "#000000" },
    { "#0F1122", "#000000" }, { "#10121C", "#000000" }, { "#10121E", "#000000" },
    { "#10131E", "#000000" }, { "#101320", "#000000" }, { "#101422", "#8C8C8C" },
    { "#101424", "#8C8C8C" }, { "#102608", "#000000" }, { "#102814", "#000000" },
    { "#11131F", "#000000" }, { "#111420", "#8C8C8C" }, { "#111528", "#8C8C8C" },
    { "#12141F", "#8C8C8C" }, { "#121528", "#8C8C8C" }, { "#121628", "#8C8C8C" },
    { "#122816", "#000000" }, { "#131628", "#8C8C8C" }, { "#13162A", "#8C8C8C" },
    { "#141728", "#8C8C8C" }, { "#14172A", "#8C8C8C" }, { "#141828", "#8C8C8C" },
    { "#141830", "#8C8C8C" }, { "#142818", "#000000" }, { "#14320A", "#000000" },
    { "#151828", "#8C8C8C" }, { "#161930", "#8C8C8C" }, { "#161A2E", "#8C8C8C" },
    { "#161C2E", "#8C8C8C" }, { "#163318", "#000000" }, { "#181C2C", "#8C8C8C" },
    { "#181C2E", "#8C8C8C" }, { "#181C30", "#8C8C8C" }, { "#181C34", "#8C8C8C" },
    { "#191D32", "#8C8C8C" }, { "#191E34", "#8C8C8C" }, { "#1A1D2E", "#8C8C8C" },
    { "#1A1D30", "#8C8C8C" }, { "#1A1E34", "#8C8C8C" }, { "#1A3D20", "#000000" },
    { "#1A3E0E", "#000000" }, { "#1C2038", "#8C8C8C" }, { "#1E2235", "#8C8C8C" },
    { "#1E2238", "#8C8C8C" }, { "#1E4226", "#000000" }, { "#1F2440", "#8C8C8C" },
    { "#202438", "#8C8C8C" }, { "#222640", "#8C8C8C" }, { "#232640", "#8C8C8C" },
    { "#252840", "#8C8C8C" }, { "#262A3C", "#8C8C8C" }, { "#272B44", "#8C8C8C" },
    { "#282C3E", "#8C8C8C" }, { "#282C44", "#8C8C8C" }, { "#2A2E45", "#8C8C8C" },
    { "#2A2E48", "#8C8C8C" }, { "#2A2E4A", "#8C8C8C" }, { "#2E3250", "#8C8C8C" },
    { "#2E3258", "#8C8C8C" }, { "#30344E", "#8C8C8C" }, { "#323658", "#8C8C8C" },
    { "#343860", "#8C8C8C" }, { "#363A58", "#8C8C8C" }, { "#3A3E5E", "#8C8C8C" },
    { "#3E4360", "#8C8C8C" }, { "#444860", "#8C8C8C" }, { "#464A62", "#8C8C8C" },
    { "#4A4E78", "#8C8C8C" }, { "#505468", "#8C8C8C" }, { "#7880A0", "#8C8C8C" },
    { "#7A7E96", "#8C8C8C" }, { "#8890A8", "#FFFFFF" }, { "#A8ACBE", "#FFFFFF" },
    { "#A8ACC0", "#FFFFFF" }, { "#B8BCD0", "#FFFFFF" }, { "#C0C4D4", "#FFFFFF" },
    { "#C8CCD8", "#FFFFFF" }, { "#D0D4E0", "#FFFFFF" }, { "#D0D4E8", "#FFFFFF" },
    { "#DCE0F4", "#FFFFFF" }, { "#E0F4FF", "#FFFFFF" }, { "#E8ECF8", "#FFFFFF" },
};
}  // namespace

QString BetelineyHighContrastTheme::appStyleSheet()
{
    QString css = BetelineyTheme::appStyleSheet();
    for (const auto& [from, to] : kRecolorTable)
        css.replace(QLatin1String(from), QLatin1String(to), Qt::CaseInsensitive);

    css += R"(
QPushButton:focus, QComboBox:focus, QLineEdit:focus, QListView:focus,
QTreeView:focus, QTableView:focus, QCheckBox:focus, QRadioButton:focus {
    border: 2px solid #39FF14;
}
)";
    return css;
}
