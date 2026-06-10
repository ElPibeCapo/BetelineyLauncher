// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "BetelineyTheme.h"
#include <QObject>

QString BetelineyTheme::id()   { return "beteliney"; }
QString BetelineyTheme::name() { return QObject::tr("Beteliney Gamer"); }
QString BetelineyTheme::tooltip()
{
    return QObject::tr("Tema v5 — deep-space #080912 · neón #39FF14 · cyan #00D4FF · font JetBrains Mono.");
}

QPalette BetelineyTheme::colorScheme()
{
    QPalette p;
    // Fondos — deep space en capas
    p.setColor(QPalette::Window,          QColor(0x08, 0x09, 0x12));
    p.setColor(QPalette::Base,            QColor(0x0C, 0x0E, 0x1A));
    p.setColor(QPalette::AlternateBase,   QColor(0x10, 0x12, 0x1E));
    p.setColor(QPalette::Button,          QColor(0x12, 0x14, 0x22));
    p.setColor(QPalette::Mid,             QColor(0x18, 0x1B, 0x2C));
    p.setColor(QPalette::Dark,            QColor(0x06, 0x07, 0x0E));
    p.setColor(QPalette::Shadow,          QColor(0x02, 0x03, 0x06));
    // Texto — jerarquía clara
    p.setColor(QPalette::WindowText,      QColor(0xCC, 0xD0, 0xE4));
    p.setColor(QPalette::Text,            QColor(0xCC, 0xD0, 0xE4));
    p.setColor(QPalette::ButtonText,      QColor(0xA0, 0xA4, 0xBC));
    p.setColor(QPalette::BrightText,      QColor(0xFF, 0x44, 0x44));
    p.setColor(QPalette::PlaceholderText, QColor(0x30, 0x34, 0x4C));
    // Acento principal — neón verde
    p.setColor(QPalette::Highlight,       QColor(0x39, 0xFF, 0x14));
    p.setColor(QPalette::HighlightedText, QColor(0x04, 0x06, 0x08));
    p.setColor(QPalette::Link,            QColor(0x39, 0xFF, 0x14));
    p.setColor(QPalette::LinkVisited,     QColor(0x00, 0xD4, 0xFF));
    p.setColor(QPalette::ToolTipBase,     QColor(0x10, 0x12, 0x20));
    p.setColor(QPalette::ToolTipText,     QColor(0x39, 0xFF, 0x14));
    return fadeInactive(p, fadeAmount(), fadeColor());
}

double BetelineyTheme::fadeAmount()    { return 0.50; }
QColor BetelineyTheme::fadeColor()     { return QColor(0x08, 0x09, 0x12); }
bool   BetelineyTheme::hasStyleSheet() { return true; }


QString BetelineyTheme::appStyleSheet()
{
    return R"(

/* ================================================================
   BETELINEY GAMER THEME  v4  —  deep-space navy + #39FF14 neon + #00D4FF cyan
   Palette:
     Base    #080912   Surface  #0C0E1A   Raised   #10121E
     Stroke  #1E2235   Dim      #2A2E45   Muted    #505468
     Body    #C0C4D4   Bright   #E8ECF8
     Accent  #39FF14   AccentDim #28CC0F  AccentBg #0A1E0C
   ================================================================ */

/* ── RESET GLOBAL ─────────────────────────────────────────────── */
QWidget {
    background-color: #080912;
    color: #C0C4D4;
    font-family: "JetBrains Mono", "Noto Sans", sans-serif;
    font-size: 10pt;
    selection-background-color: #39FF14;
    selection-color: #070909;
    outline: none;
}
QWidget:disabled { color: #282C3E; }

/* ── MAIN WINDOW ──────────────────────────────────────────────── */
QMainWindow {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #090B13, stop:1 #0D101C);
}
QMainWindow::separator {
    background-color: #1A1D2E;
    width: 1px; height: 1px;
}

/* ── TOOLBARS — base ──────────────────────────────────────────── */
QToolBar {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0E1020, stop:1 #0B0D18);
    border: none;
    border-bottom: 1px solid #1A1D2E;
    padding: 4px 8px;
    spacing: 2px;
}
QToolBar::separator {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 transparent, stop:0.2 #1E2235, stop:0.8 #1E2235, stop:1 transparent);
    width: 1px;
    margin: 4px 4px;
}

/* ── TOOLBARS — por nombre ────────────────────────────────────── */
QToolBar#mainToolBar {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0F1122, stop:1 #0B0D18);
    border-bottom: 1px solid #1E2235;
    padding: 5px 10px;
}
QToolBar#instanceToolBar {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #0D1020, stop:1 #0C0E18);
    border-left: 1px solid #1A1D2E;
    border-bottom: none;
    padding: 6px 4px;
}
QToolBar#newsToolBar {
    background: #080A12;
    border-top: 1px solid #1A1D2E;
    border-bottom: none;
    padding: 2px 8px;
    min-height: 24px;
}
/* ── TOOLBUTTONS ──────────────────────────────────────────────── */
QToolButton {
    background-color: transparent;
    color: #7A7E96;
    border: 1px solid transparent;
    border-radius: 8px;
    padding: 6px 14px;
    font-size: 10pt;
}
QToolButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #1C2038, stop:1 #151828);
    border-color: #2A2E48;
    color: #D0D4E8;
}
QToolButton:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0A1210, stop:1 #0E1A12);
    border-color: #39FF14;
    color: #39FF14;
}
QToolButton:checked {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0E2212, stop:1 #0A1B0E);
    border-color: #39FF14;
    color: #39FF14;
}
QToolButton:checked:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #122816, stop:1 #0D2010);
    border-color: #55FF30;
    color: #55FF30;
}
QToolButton::menu-indicator { image: none; }
QToolButton[popupMode="1"] { padding-right: 16px; }

/* ── BUTTONS ──────────────────────────────────────────────────── */
QPushButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #191D32, stop:1 #13162A);
    color: #A8ACBE;
    border: 1px solid #272B44;
    border-radius: 8px;
    padding: 6px 20px;
    min-height: 26px;
    font-size: 10pt;
}
QPushButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #1F2440, stop:1 #181C34);
    border-color: #3E4360;
    color: #DCE0F4;
}
QPushButton:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0C0F1E, stop:1 #101424);
    border-color: #39FF14;
    color: #39FF14;
}
QPushButton:default {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #102814, stop:1 #0A1D0C);
    border: 1.5px solid #39FF14;
    color: #39FF14;
    font-weight: 700;
}
QPushButton:default:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #163318, stop:1 #0E2410);
    border-color: #5AFF32;
    color: #5AFF32;
}
QPushButton:default:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #081408, stop:1 #0A1A0C);
}
QPushButton:disabled {
    background: #0F111E;
    color: #262A3C;
    border-color: #181C2E;
}
QPushButton:flat { border: none; background: transparent; }
QPushButton:flat:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #181C2E, stop:1 #131628);
    border: 1px solid #282C44;
    border-radius: 6px;
}

/* ── INPUTS ───────────────────────────────────────────────────── */
QLineEdit, QSpinBox, QDoubleSpinBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0F111E, stop:1 #111420);
    color: #C8CCD8;
    border: 1px solid #252840;
    border-radius: 5px;
    padding: 5px 10px;
    selection-background-color: #39FF14;
    selection-color: #070909;
    min-height: 24px;
}
QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
    border-color: #39FF14;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0D1020, stop:1 #101422);
}
QLineEdit:hover, QSpinBox:hover, QDoubleSpinBox:hover { border-color: #323658; }
QLineEdit:disabled, QSpinBox:disabled { color: #282C3E; border-color: #181C2C; }

QTextEdit, QPlainTextEdit {
    background: #0F111C;
    color: #C8CCD8;
    border: 1px solid #252840;
    border-radius: 5px;
    selection-background-color: #39FF14;
    selection-color: #070909;
}
QTextEdit:focus, QPlainTextEdit:focus { border-color: #39FF14; }

QSpinBox::up-button, QSpinBox::down-button,
QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
    background-color: #1A1D2E;
    border: none;
    border-radius: 3px;
    width: 16px;
}
QSpinBox::up-button:hover, QSpinBox::down-button:hover,
QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover {
    background-color: #39FF14;
}

/* ── LISTAS / ÁRBOLES / TABLAS ────────────────────────────────── */
QListView, QTreeView, QTableView, QListWidget, QTreeWidget {
    background-color: #0F111C;
    alternate-background-color: #11131F;
    color: #C0C4D4;
    border: 1px solid #1E2235;
    border-radius: 9px;
    gridline-color: #1A1D2E;
    show-decoration-selected: 1;
}
QListView::item, QTreeView::item, QTableView::item {
    padding: 5px 4px;
    min-height: 22px;
    border: none;
    border-radius: 3px;
}
QListView::item:hover, QTreeView::item:hover, QTableView::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #191E34, stop:1 #141830);
    color: #DCE0F4;
}
/* ── SELECTED ITEM — left accent strip ─────────────────────────── */
QListView::item:selected, QTreeView::item:selected, QTableView::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #1A3D20, stop:0.06 #0E2412, stop:1 #0A1B0E);
    color: #39FF14;
    border-left: 3px solid #39FF14;
    padding-left: 6px;
    border-radius: 0 3px 3px 0;
}
QListView::item:selected:active, QTreeView::item:selected:active {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #1E4226, stop:0.06 #102814, stop:1 #0C1E10);
    color: #4AFF20;
    border-left: 3px solid #4AFF20;
    padding-left: 6px;
}
QTreeView::branch { background-color: #0F111C; }
QTreeView::branch:has-children:closed { color: #39FF14; }
QTreeView::branch:has-children:open   { color: #28CC0F; }

/* ── HEADERS ──────────────────────────────────────────────────── */
QHeaderView { background-color: transparent; border: none; }
QHeaderView::section {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #101320, stop:1 #0D1018);
    color: #444860;
    border: none;
    border-right: 1px solid #1A1D2E;
    border-bottom: 2px solid #1E2235;
    padding: 6px 12px;
    font-size: 8pt;
    font-weight: 600;
    letter-spacing: 1px;
}
QHeaderView::section:first { border-left: none; }
QHeaderView::section:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #161A2E, stop:1 #111528);
    color: #8890A8;
}
QHeaderView::section:checked {
    background: #091508;
    color: #39FF14;
    border-bottom-color: #39FF14;
}

/* ── TABS ─────────────────────────────────────────────────────── */
QTabWidget::pane {
    border: 1px solid #1E2235;
    border-top: none;
    border-radius: 0 0 7px 7px;
    background-color: #10121C;
}
QTabWidget::tab-bar { alignment: left; }
QTabBar {
    background-color: transparent;
    border-bottom: 2px solid #1E2235;
}
QTabBar::tab {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0D0F1C, stop:1 #0B0D18);
    color: #464A62;
    border: 1px solid transparent;
    border-bottom: 2px solid transparent;
    padding: 8px 22px;
    margin-right: 2px;
    border-radius: 7px 7px 0 0;
    font-size: 9pt;
}
QTabBar::tab:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #161C2E, stop:1 #10121C);
    color: #39FF14;
    border-color: #1E2235;
    border-bottom: 2px solid #39FF14;
    font-weight: 600;
}
QTabBar::tab:hover:!selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #131628, stop:1 #0E1020);
    color: #8890A8;
    border-color: #1A1D2E;
}

/* ── SCROLLBARS ───────────────────────────────────────────────── */
QScrollBar { background-color: transparent; border: none; }
QScrollBar:vertical   { width: 8px; margin: 2px 0; }
QScrollBar:horizontal { height: 8px; margin: 0 2px; }
QScrollBar::handle {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #222640, stop:1 #1E2238);
    border-radius: 3px;
    min-height: 30px; min-width: 30px;
}
QScrollBar::handle:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #343860, stop:1 #2E3258);
}
QScrollBar::handle:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #2ECC10, stop:1 #39FF14);
}
QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: none; }

/* ── MENUBAR ──────────────────────────────────────────────────── */
QMenuBar {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0C0E1C, stop:1 #090B14);
    color: #7880A0;
    border-bottom: 1px solid #1A1D2E;
    padding: 2px 6px;
    font-size: 9pt;
}
QMenuBar::item { padding: 4px 12px; border-radius: 5px; }
QMenuBar::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #1A1E34, stop:1 #141828);
    color: #C8CCD8;
}
QMenuBar::item:pressed {
    background: #0D1020;
    color: #39FF14;
}

/* ── MENUS ────────────────────────────────────────────────────── */
QMenu {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #14172A, stop:1 #10131E);
    color: #C0C4D4;
    border: 1px solid #2A2E4A;
    border-top: 1px solid #3A3E5E;
    border-radius: 10px;
    padding: 5px 0;
}
QMenu::item {
    padding: 7px 32px 7px 16px;
    border-radius: 4px;
    margin: 1px 5px;
    font-size: 9pt;
}
QMenu::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #1A3D20, stop:0.15 #0E2412, stop:1 #0A1B0E);
    color: #39FF14;
}
QMenu::separator { height: 1px; background-color: #1A1D2E; margin: 4px 10px; }
QMenu::icon { margin-left: 8px; }

/* ── COMBOBOX ─────────────────────────────────────────────────── */
QComboBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #191D32, stop:1 #13162A);
    color: #A8ACBE;
    border: 1px solid #272B44;
    border-radius: 6px;
    padding: 5px 10px;
    min-height: 26px;
    font-size: 10pt;
}
QComboBox:hover { border-color: #363A58; color: #D0D4E0; }
QComboBox:focus { border-color: #39FF14; }
QComboBox::drop-down {
    border: none;
    width: 22px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #1E2238, stop:1 #181C2E);
    border-radius: 0 6px 6px 0;
}
QComboBox::drop-down:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #272B44, stop:1 #202438);
}
QComboBox QAbstractItemView {
    background: #12141F;
    color: #C0C4D4;
    border: 1px solid #232640;
    border-radius: 6px;
    selection-background-color: #0D2010;
    selection-color: #39FF14;
    padding: 3px 0;
}

/* ── CHECKBOXES & RADIOS ──────────────────────────────────────── */
QCheckBox, QRadioButton {
    color: #B8BCD0;
    spacing: 9px;
    padding: 2px;
    font-size: 9pt;
}
QCheckBox:hover, QRadioButton:hover { color: #DCE0F4; }
QCheckBox::indicator {
    width: 15px; height: 15px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #191D32, stop:1 #13162A);
    border: 1px solid #2E3250;
    border-radius: 4px;
}
QCheckBox::indicator:hover { border-color: #39FF14; }
QCheckBox::indicator:checked {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
        stop:0 #28CC0F, stop:1 #39FF14);
    border-color: #39FF14;
}
QCheckBox::indicator:checked:disabled {
    background: #142818;
    border-color: #142818;
}
QRadioButton::indicator {
    width: 15px; height: 15px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #191D32, stop:1 #13162A);
    border: 1px solid #2E3250;
    border-radius: 8px;
}
QRadioButton::indicator:hover   { border-color: #39FF14; }
QRadioButton::indicator:checked {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
        stop:0 #28CC0F, stop:1 #39FF14);
    border-color: #39FF14;
}

/* ── GROUPBOX ─────────────────────────────────────────────────── */
QGroupBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #10121E, stop:1 #0D1018);
    border: 1px solid #1E2235;
    border-radius: 10px;
    margin-top: 16px;
    padding-top: 14px;
}
QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 14px;
    padding: 0 8px;
    color: #39FF14;
    font-size: 8pt;
    font-weight: 700;
    letter-spacing: 1.2px;
    text-transform: uppercase;
    background-color: transparent;
}

/* ── PROGRESSBAR ──────────────────────────────────────────────── */
QProgressBar {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #121528, stop:1 #0E1020);
    border: 1px solid #1E2235;
    border-radius: 5px;
    color: transparent;
    text-align: center;
    min-height: 6px;
    max-height: 8px;
}
QProgressBar::chunk {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #22BB0C, stop:0.4 #39FF14, stop:1 #50FF28);
    border-radius: 4px;
}

/* ── SPLITTER ─────────────────────────────────────────────────── */
QSplitter::handle {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 transparent, stop:0.3 #1A1D2E, stop:0.7 #1A1D2E, stop:1 transparent);
}
QSplitter::handle:vertical   { height: 2px; }
QSplitter::handle:horizontal { width: 2px; }
QSplitter::handle:hover { background-color: #2A2E48; }

/* ── STATUSBAR ────────────────────────────────────────────────── */
QStatusBar {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #080A14, stop:1 #060810);
    color: #282C44;
    border-top: 1px solid #161930;
    font-size: 8pt;
}
QStatusBar::item { border: none; }
QStatusBar QLabel { color: #30344E; }

/* ── TOOLTIPS ─────────────────────────────────────────────────── */
QToolTip {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #181C30, stop:1 #121628);
    color: #D0D4E8;
    border: 1px solid #2E3250;
    border-top: 1px solid #4A4E78;
    padding: 6px 11px;
    border-radius: 6px;
    font-size: 9pt;
}

/* ── DIALOGS ──────────────────────────────────────────────────── */
QDialog {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #111420, stop:1 #0D101C);
    border: 1px solid #1E2235;
}

/* ── LABELS ───────────────────────────────────────────────────── */
QLabel { background-color: transparent; color: #A8ACC0; }
QLabel:disabled { color: #282C3E; }

/* ── SLIDERS ──────────────────────────────────────────────────── */
QSlider::groove:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #141728, stop:1 #1A1D30);
    height: 4px;
    border-radius: 2px;
}
QSlider::handle:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
        stop:0 #50FF28, stop:1 #28CC0F);
    width: 14px; height: 14px;
    margin: -5px 0;
    border-radius: 7px;
    border: 2px solid #090B14;
}
QSlider::handle:horizontal:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
        stop:0 #62FF38, stop:1 #39FF14);
}
QSlider::sub-page:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #22BB0C, stop:1 #39FF14);
    border-radius: 2px;
}
QSlider::groove:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #141728, stop:1 #1A1D30);
    width: 4px;
    border-radius: 2px;
}
QSlider::handle:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
        stop:0 #50FF28, stop:1 #28CC0F);
    width: 14px; height: 14px;
    margin: 0 -5px;
    border-radius: 7px;
    border: 2px solid #090B14;
}

/* ── FRAMES ───────────────────────────────────────────────────── */
QFrame[frameShape="4"], QFrame[frameShape="5"] { color: #1A1D2E; }

/* ── SCROLLAREA / PANELS ─────────────────────────────────────── */
QScrollArea {
    background-color: #080912;
    border: none;
}
QScrollArea > QWidget > QWidget { background-color: #080912; }

/* ── STACK / MISC ─────────────────────────────────────────────── */
QStackedWidget { background-color: #080912; }

/* ── DOCKWIDGET ───────────────────────────────────────────────── */
QDockWidget {
    background-color: #080912;
    color: #C0C4D4;
    titlebar-close-icon: none;
    titlebar-normal-icon: none;
}
QDockWidget::title {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0F1122, stop:1 #0B0D18);
    color: #39FF14;
    font-size: 8pt;
    font-weight: 700;
    letter-spacing: 1.2px;
    text-transform: uppercase;
    padding: 5px 10px;
    border-bottom: 1px solid #1E2235;
}
QDockWidget::close-button, QDockWidget::float-button {
    background: transparent;
    border: none;
    padding: 2px;
    border-radius: 4px;
}
QDockWidget::close-button:hover, QDockWidget::float-button:hover {
    background-color: #1A1D2E;
}

/* ── MESSAGEBOX ───────────────────────────────────────────────── */
QMessageBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #111420, stop:1 #0D101C);
    color: #C0C4D4;
}
QMessageBox QLabel {
    color: #C0C4D4;
    font-size: 9pt;
    background: transparent;
    padding: 4px 2px;
}
QMessageBox QPushButton {
    min-width: 80px;
}

/* ── CHECKBOX — checkmark (data URIs no soportados en Qt QSS) ── */
QCheckBox::indicator:checked {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
        stop:0 #28CC0F, stop:1 #39FF14);
    border-color: #39FF14;
}

/* ── PROGRESSBAR — porcentaje visible ────────────────────────── */
QProgressBar {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #121528, stop:1 #0E1020);
    border: 1px solid #1E2235;
    border-radius: 5px;
    color: #39FF14;
    font-size: 7pt;
    font-weight: 700;
    text-align: center;
    min-height: 14px;
    max-height: 16px;
}
QProgressBar::chunk {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #22BB0C, stop:0.4 #39FF14, stop:1 #50FF28);
    border-radius: 4px;
}

/* ── WIZARD ───────────────────────────────────────────────────── */
QWizard {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #0E1020, stop:1 #0C0E18);
}
QWizardPage {
    background: #080912;
    color: #C0C4D4;
}

/* ── SIZEGRIP ─────────────────────────────────────────────────── */
QSizeGrip {
    background-color: transparent;
    width: 12px; height: 12px;
}

/* ── INSTANCE VIEW — cards de instancias ─────────────────────── */
InstanceView {
    background-color: #080912;
    border: none;
}

/* ── BOTÓN LAUNCH — acento fuerte ────────────────────────────── */
QPushButton#launchButton, QToolButton#actionLaunchInstance {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #14320A, stop:1 #0C1F06);
    color: #39FF14;
    border: 1.5px solid #39FF14;
    border-radius: 8px;
    font-weight: 700;
    letter-spacing: 0.5px;
}
QPushButton#launchButton:hover, QToolButton#actionLaunchInstance:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #1A3E0E, stop:1 #102608);
    border-color: #55FF30;
    color: #55FF30;
}

/* ── ACENTO CYAN en estados secundarios ──────────────────────── */
QToolButton:hover {
    border-color: #00D4FF;
    color: #00D4FF;
}
QTabBar::tab:hover:!selected {
    color: #00D4FF;
    border-color: #00D4FF;
}
QListView::item:hover, QTreeView::item:hover {
    border-left: 2px solid #00D4FF;
    padding-left: 6px;
    color: #E0F4FF;
}

/* ── LABEL link style ────────────────────────────────────────── */
QLabel[hasLink="true"], QLabel a {
    color: #39FF14;
    text-decoration: underline;
}

/* ── PANEL lateral instancias (instanceToolBar) ──────────────── */
QToolBar#instanceToolBar QToolButton {
    border-radius: 6px;
    margin: 1px 2px;
    padding: 5px 8px;
    min-width: 80px;
    text-align: left;
}
QToolBar#instanceToolBar QToolButton:checked {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #0E2412, stop:1 #091508);
    border-left: 3px solid #39FF14;
    border-radius: 0 6px 6px 0;
    color: #39FF14;
    font-weight: 600;
}

/* ── NEWS TICKER ─────────────────────────────────────────────── */
QToolBar#newsToolBar QLabel {
    color: #505468;
    font-size: 8pt;
    font-style: italic;
}

/* ── ACCENTBAR — borde superior de la ventana ────────────────── */
QMainWindow::separator:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #39FF14, stop:0.5 #00D4FF, stop:1 #39FF14);
    height: 2px;
}

/* ── RUBBER BAND (selección con mouse) ───────────────────────── */
QRubberBand {
    background-color: rgba(57, 255, 20, 0.08);
    border: 1px solid #39FF14;
    border-radius: 2px;
}

/* ── FONT: tamaño consistente ────────────────────────────────── */
QMenuBar, QMenu, QToolButton, QPushButton,
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox,
QCheckBox, QRadioButton, QTabBar::tab,
QListView, QTreeView, QTableView {
    font-family: "JetBrains Mono", "Noto Sans", sans-serif;
    font-size: 10pt;
}
QHeaderView::section, QStatusBar, QGroupBox::title {
    font-family: "JetBrains Mono", "Noto Sans", sans-serif;
    font-size: 8pt;
}

    )";
}
