// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *  Copyright (C) 2024 TheKodeToad <TheKodeToad@proton.me>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "LogPage.h"
#include "ui_LogPage.h"

#include "Application.h"

#include <QDesktopServices>
#include <QIdentityProxyModel>
#include <QScrollBar>
#include <QShortcut>
#include <QUrl>

#include "launch/LaunchTask.h"
#include "minecraft/MinecraftInstance.h"
#include "settings/Setting.h"

#include "ui/GuiUtil.h"
#include "ui/themes/ThemeManager.h"

#include <BuildConfig.h>

QVariant LogFormatProxyModel::data(const QModelIndex& index, int role) const
{
    const LogColors& colors = APPLICATION->themeManager()->getLogColors();

    switch (role) {
        case Qt::FontRole:
            return m_font;
        case Qt::ForegroundRole: {
            MessageLevel level = static_cast<MessageLevel::Enum>(QIdentityProxyModel::data(index, LogModel::LevelRole).toInt());
            QColor result = colors.foreground.value(level);

            if (result.isValid())
                return result;

            break;
        }
        case Qt::BackgroundRole: {
            MessageLevel level = static_cast<MessageLevel::Enum>(QIdentityProxyModel::data(index, LogModel::LevelRole).toInt());
            QColor result = colors.background.value(level);

            if (result.isValid())
                return result;

            break;
        }
    }

    return QIdentityProxyModel::data(index, role);
}

QModelIndex LogFormatProxyModel::find(const QModelIndex& start, const QString& value, bool reverse) const
{
    QModelIndex parentIndex = parent(start);
    auto compare = [this, start, parentIndex, value](int r) -> QModelIndex {
        QModelIndex idx = index(r, start.column(), parentIndex);
        if (!idx.isValid() || idx == start) {
            return QModelIndex();
        }
        QVariant v = data(idx, Qt::DisplayRole);
        QString t = v.toString();
        if (t.contains(value, Qt::CaseInsensitive))
            return idx;
        return QModelIndex();
    };
    if (reverse) {
        int from = start.row();
        int to = 0;

        for (int i = 0; i < 2; ++i) {
            for (int r = from; (r >= to); --r) {
                auto idx = compare(r);
                if (idx.isValid())
                    return idx;
            }
            // prepare for the next iteration
            from = rowCount() - 1;
            to = start.row();
        }
    } else {
        int from = start.row();
        int to = rowCount(parentIndex);

        for (int i = 0; i < 2; ++i) {
            for (int r = from; (r < to); ++r) {
                auto idx = compare(r);
                if (idx.isValid())
                    return idx;
            }
            // prepare for the next iteration
            from = 0;
            to = start.row();
        }
    }
    return QModelIndex();
}

LogPage::LogPage(BaseInstance* instance, QWidget* parent) : QWidget(parent), ui(new Ui::LogPage), m_instance(instance)
{
    ui->setupUi(this);

    m_proxy = new LogFormatProxyModel(this);

    // set up fonts in the log proxy
    {
        QString fontFamily = APPLICATION->settings()->get("ConsoleFont").toString();
        bool conversionOk = false;
        int fontSize = APPLICATION->settings()->get("ConsoleFontSize").toInt(&conversionOk);
        if (!conversionOk) {
            fontSize = 11;
        }
        m_proxy->setFont(QFont(fontFamily, fontSize));
    }

    ui->text->setModel(m_proxy);

    // set up instance and launch process recognition
    {
        auto launchTask = m_instance->getLaunchTask();
        if (launchTask) {
            setInstanceLaunchTaskChanged(launchTask, true);
        }
        connect(m_instance, &BaseInstance::launchTaskChanged, this, &LogPage::onInstanceLaunchTaskChanged);
    }

    auto findShortcut = new QShortcut(QKeySequence(QKeySequence::Find), this);
    connect(findShortcut, &QShortcut::activated, this, &LogPage::findActivated);
    auto findNextShortcut = new QShortcut(QKeySequence(QKeySequence::FindNext), this);
    connect(findNextShortcut, &QShortcut::activated, this, &LogPage::findNextActivated);
    connect(ui->searchBar, &QLineEdit::returnPressed, this, &LogPage::on_findButton_clicked);
    auto findPreviousShortcut = new QShortcut(QKeySequence(QKeySequence::FindPrevious), this);
    connect(findPreviousShortcut, &QShortcut::activated, this, &LogPage::findPreviousActivated);
}

LogPage::~LogPage()
{
    delete ui;
}

void LogPage::modelStateToUI()
{
    if (m_model->wrapLines()) {
        ui->text->setWordWrap(true);
        ui->wrapCheckbox->setCheckState(Qt::Checked);
    } else {
        ui->text->setWordWrap(false);
        ui->wrapCheckbox->setCheckState(Qt::Unchecked);
    }
    if (m_model->colorLines()) {
        ui->text->setColorLines(true);
        ui->colorCheckbox->setCheckState(Qt::Checked);
    } else {
        ui->text->setColorLines(false);
        ui->colorCheckbox->setCheckState(Qt::Unchecked);
    }
    if (m_model->suspended()) {
        ui->trackLogCheckbox->setCheckState(Qt::Unchecked);
    } else {
        ui->trackLogCheckbox->setCheckState(Qt::Checked);
    }
}

void LogPage::UIToModelState()
{
    if (!m_model) {
        return;
    }
    m_model->setLineWrap(ui->wrapCheckbox->checkState() == Qt::Checked);
    m_model->setColorLines(ui->colorCheckbox->checkState() == Qt::Checked);
    m_model->suspend(ui->trackLogCheckbox->checkState() != Qt::Checked);
}

void LogPage::setInstanceLaunchTaskChanged(LaunchTask* proc, bool initial)
{
    m_process = proc;
    // Ocultar diagnóstico anterior cuando empieza una nueva sesión de juego
    hideDiagnosis();
    if (m_process) {
        m_model = proc->getLogModel();
        m_proxy->setSourceModel(m_model.get());
        if (initial) {
            modelStateToUI();
        } else {
            UIToModelState();
        }
        // Conectar señal de fin de proceso para ejecutar el diagnóstico
        connect(m_process, &Task::finished, this, &LogPage::onLaunchTaskFinished);
    } else {
        m_proxy->setSourceModel(nullptr);
        m_model.reset();
    }
}

void LogPage::onInstanceLaunchTaskChanged(LaunchTask* proc)
{
    setInstanceLaunchTaskChanged(proc, false);
}

bool LogPage::apply()
{
    return true;
}

bool LogPage::shouldDisplay() const
{
    return true;
}

void LogPage::on_btnPaste_clicked()
{
    if (!m_model)
        return;

    // FIXME: turn this into a proper task and move the upload logic out of GuiUtil!
    m_model->append(MessageLevel::Launcher,
                    QString("Log upload triggered at: %1").arg(QDateTime::currentDateTime().toString(Qt::RFC2822Date)));
    auto url = GuiUtil::uploadPaste(tr("Minecraft Log"), m_model->toPlainText(), this);
    if (!url.has_value()) {
        m_model->append(MessageLevel::Error, QString("Log upload canceled"));
    } else if (url->isNull()) {
        m_model->append(MessageLevel::Error, QString("Log upload failed!"));
    } else {
        m_model->append(MessageLevel::Launcher, QString("Log uploaded to: %1").arg(url.value()));
    }
}

void LogPage::on_btnCopy_clicked()
{
    if (!m_model)
        return;
    m_model->append(MessageLevel::Launcher, QString("Clipboard copy at: %1").arg(QDateTime::currentDateTime().toString(Qt::RFC2822Date)));
    GuiUtil::setClipboardText(m_model->toPlainText());
}

void LogPage::on_btnClear_clicked()
{
    if (!m_model)
        return;
    m_model->clear();
    m_container->refreshContainer();
}

void LogPage::on_btnBottom_clicked()
{
    ui->text->scrollToBottom();
}

void LogPage::on_trackLogCheckbox_clicked(bool checked)
{
    if (!m_model)
        return;
    m_model->suspend(!checked);
}

void LogPage::on_wrapCheckbox_clicked(bool checked)
{
    ui->text->setWordWrap(checked);
    if (!m_model)
        return;
    m_model->setLineWrap(checked);
}

void LogPage::on_colorCheckbox_clicked(bool checked)
{
    ui->text->setColorLines(checked);
    if (!m_model)
        return;
    m_model->setColorLines(checked);
}

void LogPage::on_findButton_clicked()
{
    auto modifiers = QApplication::keyboardModifiers();
    bool reverse = modifiers & Qt::ShiftModifier;
    ui->text->findNext(ui->searchBar->text(), reverse);
}

void LogPage::findNextActivated()
{
    ui->text->findNext(ui->searchBar->text(), false);
}

void LogPage::findPreviousActivated()
{
    ui->text->findNext(ui->searchBar->text(), true);
}

void LogPage::findActivated()
{
    // focus the search bar if it doesn't have focus
    if (!ui->searchBar->hasFocus()) {
        ui->searchBar->setFocus();
        ui->searchBar->selectAll();
    }
}

void LogPage::retranslate()
{
    ui->retranslateUi(this);
}

// ─── BETELINEY LOG ANALYZER — INTEGRACIÓN ────────────────────────────────────

void LogPage::onLaunchTaskFinished()
{
    if (!m_model || !m_process)
        return;

    // Solo diagnosticar si el juego no cerró limpiamente
    int exitCode = m_process->gameExitCode();
    if (exitCode == 0)
        return;

    QString fullLog = m_model->toPlainText();
    m_diagnoses     = Beteliney::LogAnalyzer::analyze(fullLog, exitCode);

    if (m_diagnoses.isEmpty())
        return;

    m_diagnosisIndex = 0;
    showDiagnosis(0);
}

void LogPage::showDiagnosis(int index)
{
    if (index < 0 || index >= m_diagnoses.size())
        return;

    const Beteliney::Diagnosis& d = m_diagnoses[index];

    // Icono y color según severidad
    QString icon;
    QString panelStyle;
    switch (d.severity) {
        case Beteliney::DiagnosisSeverity::Critical:
            icon       = "🔴";
            panelStyle = "QFrame#diagnosisPanel { border: 2px solid #FF4444; border-radius: 6px; "
                         "background-color: rgba(255,68,68,0.12); }";
            break;
        case Beteliney::DiagnosisSeverity::Error:
            icon       = "🟠";
            panelStyle = "QFrame#diagnosisPanel { border: 2px solid #FF8C00; border-radius: 6px; "
                         "background-color: rgba(255,140,0,0.12); }";
            break;
        case Beteliney::DiagnosisSeverity::Warning:
            icon       = "🟡";
            panelStyle = "QFrame#diagnosisPanel { border: 2px solid #FFD700; border-radius: 6px; "
                         "background-color: rgba(255,215,0,0.10); }";
            break;
        default:
            icon       = "🔵";
            panelStyle = "QFrame#diagnosisPanel { border: 2px solid #00D4FF; border-radius: 6px; "
                         "background-color: rgba(0,212,255,0.10); }";
            break;
    }

    ui->diagnosisPanel->setStyleSheet(panelStyle);
    ui->diagnosisIcon->setText(icon);

    // Título en negrita
    ui->diagnosisTitle->setText(QString("<b>%1</b>").arg(d.title.toHtmlEscaped()));

    // Explicación
    ui->diagnosisExplanation->setText(d.explanation);

    // Solución con prefijo destacado
    ui->diagnosisSolution->setText(
        QString("<span style='color:#39FF14;font-weight:bold;'>→ Solución: </span>%1")
            .arg(d.solution.toHtmlEscaped()));

    // Detalle del log (fragmento relevante)
    if (!d.detail.isEmpty()) {
        ui->diagnosisDetail->setText(
            QString("<code style='font-size:10px;color:#aaa;'>%1</code>")
                .arg(d.detail.toHtmlEscaped()));
        ui->diagnosisDetail->setVisible(true);
    } else {
        ui->diagnosisDetail->setVisible(false);
    }

    // Botón de acción
    if (!d.actionLabel.isEmpty()) {
        ui->diagnosisActionBtn->setText(d.actionLabel);
        ui->diagnosisActionBtn->setVisible(true);
    } else {
        ui->diagnosisActionBtn->setVisible(false);
    }

    // Contador y botón siguiente (si hay más de un diagnóstico)
    if (m_diagnoses.size() > 1) {
        ui->diagnosisCounter->setText(
            QString("%1 / %2").arg(index + 1).arg(m_diagnoses.size()));
        ui->diagnosisCounter->setVisible(true);
        ui->diagnosisNextBtn->setVisible(index < m_diagnoses.size() - 1);
    } else {
        ui->diagnosisCounter->setVisible(false);
        ui->diagnosisNextBtn->setVisible(false);
    }

    ui->diagnosisPanel->setVisible(true);
}

void LogPage::hideDiagnosis()
{
    m_diagnoses.clear();
    m_diagnosisIndex = 0;
    ui->diagnosisPanel->setVisible(false);
}

void LogPage::on_diagnosisActionBtn_clicked()
{
    if (m_diagnosisIndex < 0 || m_diagnosisIndex >= m_diagnoses.size())
        return;

    const QString& target = m_diagnoses[m_diagnosisIndex].actionTarget;

    if (target == "java") {
        APPLICATION->ShowGlobalSettings(this, "java-settings");
    } else if (target == "mods-folder") {
        // Abrir la carpeta mods de la instancia en el explorador de archivos
        auto* mcInstance = dynamic_cast<MinecraftInstance*>(m_instance);
        if (mcInstance)
            QDesktopServices::openUrl(QUrl::fromLocalFile(mcInstance->modsRoot()));
        else
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_instance->gameRoot()));
    } else if (target.startsWith("search-modrinth:")) {
        QString modId = target.mid(QString("search-modrinth:").size());
        QDesktopServices::openUrl(
            QUrl(QString("https://modrinth.com/mods?q=%1").arg(modId)));
    }
}

void LogPage::on_diagnosisNextBtn_clicked()
{
    if (m_diagnosisIndex + 1 < m_diagnoses.size()) {
        m_diagnosisIndex++;
        showDiagnosis(m_diagnosisIndex);
    }
}

void LogPage::on_diagnosisDismissBtn_clicked()
{
    hideDiagnosis();
}
