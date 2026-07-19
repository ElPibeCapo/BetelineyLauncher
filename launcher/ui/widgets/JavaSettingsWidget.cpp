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

#include "JavaSettingsWidget.h"

#include <QFileDialog>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>
#include "Application.h"
#include "BuildConfig.h"
#include "FileSystem.h"
#include "JavaCommon.h"
#include "java/JavaInstallList.h"
#include "java/JavaUtils.h"
#include "settings/Setting.h"
#include "HardwareInfo.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/VersionSelectDialog.h"
#include "ui/java/InstallJavaDialog.h"

#include "ui_JavaSettingsWidget.h"

JavaSettingsWidget::JavaSettingsWidget(BaseInstance* instance, QWidget* parent)
    : QWidget(parent), m_instance(instance), m_ui(new Ui::JavaSettingsWidget)
{
    m_ui->setupUi(this);

    // Beteliney: ocultar PermGen (deprecado desde Java 8)
    m_ui->permGenSpinBox->setVisible(false);
    m_ui->label_11->setVisible(false);
    m_ui->label_3->setVisible(false);

    if (m_instance == nullptr) {
        m_ui->javaDownloadBtn->hide();
        if (BuildConfig.JAVA_DOWNLOADER_ENABLED) {
            connect(m_ui->autodetectJavaCheckBox, &QCheckBox::stateChanged, this, [this](bool state) {
                m_ui->autodownloadJavaCheckBox->setEnabled(state);
                if (!state)
                    m_ui->autodownloadJavaCheckBox->setChecked(false);
            });
        } else {
            m_ui->autodownloadJavaCheckBox->hide();
        }
    } else {
        m_ui->javaDownloadBtn->setVisible(BuildConfig.JAVA_DOWNLOADER_ENABLED);
        m_ui->skipWizardCheckBox->hide();
        m_ui->autodetectJavaCheckBox->hide();
        m_ui->autodownloadJavaCheckBox->hide();

        m_ui->javaInstallationGroupBox->setCheckable(true);
        m_ui->memoryGroupBox->setCheckable(true);
        m_ui->javaArgumentsGroupBox->setCheckable(true);

        SettingsObject* settings = m_instance->settings();

        connect(settings->getSetting("OverrideJavaLocation").get(), &Setting::SettingChanged, m_ui->javaInstallationGroupBox,
                [this, settings] { m_ui->javaInstallationGroupBox->setChecked(settings->get("OverrideJavaLocation").toBool()); });
        connect(settings->getSetting("JavaPath").get(), &Setting::SettingChanged, m_ui->javaInstallationGroupBox,
                [this, settings] { m_ui->javaPathTextBox->setText(settings->get("JavaPath").toString()); });

        connect(m_ui->javaDownloadBtn, &QPushButton::clicked, this, [this] {
            auto javaDialog = new Java::InstallDialog({}, m_instance, this);
            javaDialog->exec();
        });
        connect(m_ui->javaPathTextBox, &QLineEdit::textChanged, [this](QString newValue) {
            if (m_instance->settings()->get("JavaPath").toString() != newValue) {
                m_instance->settings()->set("AutomaticJava", false);
            }
        });
    }

    connect(m_ui->javaTestBtn, &QPushButton::clicked, this, &JavaSettingsWidget::onJavaTest);
    connect(m_ui->javaDetectBtn, &QPushButton::clicked, this, &JavaSettingsWidget::onJavaAutodetect);
    connect(m_ui->javaBrowseBtn, &QPushButton::clicked, this, &JavaSettingsWidget::onJavaBrowse);

    // MEJ-5: Detectar GraalVM al cambiar la ruta Java — actualizar tooltip con badge de rendimiento
    connect(m_ui->javaPathTextBox, &QLineEdit::textChanged, this, [this](const QString& path) {
        if (path.isEmpty()) return;
        QFileInfo fi(path);
        if (!fi.exists() || !fi.isExecutable()) return;
        // Lanza java -version en background; stderr contiene la versión
        QProcess* proc = new QProcess(this);
        proc->setProgram(path);
        proc->setArguments(QStringList() << "-version");
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, proc](int, QProcess::ExitStatus) {
                QString out = QString::fromLocal8Bit(proc->readAllStandardError())
                            + QString::fromLocal8Bit(proc->readAllStandardOutput());
                proc->deleteLater();
                if (out.contains("GraalVM", Qt::CaseInsensitive)) {
                    // Extraer versión GraalVM del output
                    // Sesión 45: la cifra "+10-20% FPS" que estaba acá venía de una fuente
                    // ya marcada como no verificada/cita mal puesta en ESTADO.md (sesión 42,
                    // "Hoja de Ruta de Excelencia Técnica 2026" — su referencia sobre GraalVM
                    // apuntaba a un artículo sin relación con JVM). Se corrigió la documentación
                    // pero este texto de UI, que el usuario ve de verdad, había quedado sin
                    // tocar mostrando la misma cifra sin respaldo. Se saca el número; queda
                    // solo la afirmación técnica real (JIT más agresivo, documentado por
                    // GraalVM/Oracle) sin comprometerse con ningún porcentaje sin medir.
                    QString badge = tr("⬡ GraalVM CE detectado — compilador JIT más agresivo que OpenJDK");
                    QString vendor = "GraalVM";
                    // Buscar "GraalVM CE X.Y.Z" en output
                    QRegularExpression re(R"(GraalVM CE [\d.]+)");
                    auto match = re.match(out);
                    if (match.hasMatch()) vendor = match.captured(0);
                    m_ui->javaPathTextBox->setToolTip(
                        tr("%1\n\nGraalVM tiene un compilador JIT más agresivo que OpenJDK.\n"
                           "Los perfiles JVM de Beteliney son compatibles con GraalVM.").arg(badge));
                    m_ui->javaPathTextBox->setStyleSheet(
                        "QLineEdit { border: 1px solid #39FF14; border-radius: 3px; }");
                } else {
                    m_ui->javaPathTextBox->setToolTip("");
                    m_ui->javaPathTextBox->setStyleSheet("");
                }
            });
        proc->start();
    });

    // Beteliney: selector de perfiles JVM
    // El primer perfil (índice 0) es "Personalizado (sin flags)" — no muestra rango de memoria
    {
        const auto& clearProfile = BETELINEY_PROFILES[0];
        m_ui->betelineyProfileComboBox->addItem(clearProfile.name);
        m_ui->betelineyProfileComboBox->setItemData(0, clearProfile.description, Qt::ToolTipRole);
    }
    for (int i = 1; i < BETELINEY_PROFILES.size(); ++i) {
        const auto& profile = BETELINEY_PROFILES[i];
        QString label = QString("%1  (%2\u2013%3 MB)").arg(profile.name).arg(profile.minMemMB).arg(profile.maxMemMB);
        // MEJ-6: marcar perfiles ZGC con indicador visual
        if (profile.jvmArgs.contains("-XX:+UseZGC"))
            label = QString("\u26a1 %1").arg(label); // ⚡ ZGC
        m_ui->betelineyProfileComboBox->addItem(label);
        QString tooltip = profile.description;
        if (profile.jvmArgs.contains("-XX:+UseZGC"))
            tooltip += tr("\n\n⚠ Solo Java 21+. Verificar compatibilidad con mods antes de usar.");
        m_ui->betelineyProfileComboBox->setItemData(i, tooltip, Qt::ToolTipRole);
    }

    // Detección automática de GPU integrada y RAM para preseleccionar perfil JVM.
    // En Linux: usa lspci para detectar iGPU (Vega, Intel UHD, etc.)
    // En Windows: fallback a detección por RAM solamente.
    {
        auto sysMiB = static_cast<int>(HardwareInfo::totalRamMiB());
        int suggested = 1; // Ligero por defecto

        // ── Detectar iGPU (Linux) ────────────────────────────────────────
        bool hasIGPU = false;
#if defined(Q_OS_LINUX)
        {
            QProcess lspci;
            lspci.start("lspci", QStringList() << "-mm");
            if (lspci.waitForFinished(2000)) {
                QString output = QString::fromLocal8Bit(lspci.readAllStandardOutput());
                // Buscar VGA/3D controller que sea AMD APU, Intel UHD/HD o similar integrado
                static const QStringList iGPUKeywords = {
                    "Vega", "Radeon RX Vega", "Picasso", "Renoir", "Cezanne",
                    "UHD Graphics", "HD Graphics", "Iris", "Xe Graphics",
                    "Kaveri", "Carrizo", "Bristol Ridge", "Raven"
                };
                for (const QString& kw : iGPUKeywords) {
                    if (output.contains(kw, Qt::CaseInsensitive)) {
                        hasIGPU = true;
                        break;
                    }
                }
                // Si hay VGA pero no GPU dedicada (sin NVIDIA/RX en líneas de 3D/VGA separadas)
                if (!hasIGPU) {
                    bool hasDiscreteGPU = output.contains("NVIDIA", Qt::CaseInsensitive) ||
                                         output.contains("GeForce", Qt::CaseInsensitive) ||
                                         output.contains("Radeon RX", Qt::CaseInsensitive);
                    bool hasAnyGPU = output.contains("VGA compatible controller", Qt::CaseInsensitive) ||
                                     output.contains("3D controller", Qt::CaseInsensitive);
                    if (hasAnyGPU && !hasDiscreteGPU)
                        hasIGPU = true;
                }
            }
        }
#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN)
        // Windows: detectar iGPU vía WMIC (sin dependencias extra)
        if (!hasIGPU) {
            QProcess wmic;
            wmic.start("wmic", QStringList() << "path" << "Win32_VideoController" << "get" << "Name" << "/format:list");
            if (wmic.waitForFinished(3000)) {
                QString output = QString::fromLocal8Bit(wmic.readAllStandardOutput());
                static const QStringList iGPUKeywords = {
                    "Vega", "Radeon Vega", "Picasso", "Renoir", "Cezanne",
                    "UHD Graphics", "HD Graphics", "Iris", "Xe Graphics",
                    "Intel(R) UHD", "Intel(R) HD", "Intel(R) Iris"
                };
                // Buscar en líneas "Name=..." si hay keywords de iGPU
                for (const QString& line : output.split('\n')) {
                    if (!line.trimmed().startsWith("Name="))
                        continue;
                    for (const QString& kw : iGPUKeywords) {
                        if (line.contains(kw, Qt::CaseInsensitive)) {
                            hasIGPU = true;
                            break;
                        }
                    }
                    if (hasIGPU) break;
                }
            }
        }
#endif

        // ── Seleccionar perfil ───────────────────────────────────────────
        if (hasIGPU && sysMiB < 12288) {
            // iGPU con menos de 12 GB: la VRAM comparte RAM, restringir a 1.5 GB
            suggested = 0; // índice 0 = iGPU / RAM compartida
        } else if (sysMiB >= 32768) {
            suggested = 4; // Extremo (≥32 GB)
        } else if (sysMiB >= 16384) {
            suggested = 3; // Pesado (≥16 GB)
        } else if (sysMiB >= 8192) {
            suggested = 2; // Balanceado (≥8 GB)
        } else {
            suggested = 1; // Ligero (<8 GB sin iGPU detectada)
        }

        m_ui->betelineyProfileComboBox->setCurrentIndex(suggested);

        // Mostrar tooltip indicando si se detectó iGPU
        if (hasIGPU) {
            m_ui->betelineyProfileComboBox->setToolTip(
                tr("GPU integrada detectada — se sugiere perfil iGPU para no privar de RAM a la VRAM compartida."));
        }
    }

    connect(m_ui->applyBetelineyProfileBtn, &QPushButton::clicked, this, &JavaSettingsWidget::onApplyBetelineyProfile);

    connect(m_ui->maxMemSpinBox, &QSpinBox::valueChanged, this, &JavaSettingsWidget::updateThresholds);
    connect(m_ui->minMemSpinBox, &QSpinBox::valueChanged, this, &JavaSettingsWidget::updateThresholds);

    loadSettings();
    updateThresholds();
}

JavaSettingsWidget::~JavaSettingsWidget()
{
    delete m_ui;
}

void JavaSettingsWidget::loadSettings()
{
    SettingsObject* settings;

    if (m_instance != nullptr)
        settings = m_instance->settings();
    else
        settings = APPLICATION->settings();

    // Java Settings
    m_ui->javaInstallationGroupBox->setChecked(settings->get("OverrideJavaLocation").toBool());
    m_ui->javaPathTextBox->setText(settings->get("JavaPath").toString());

    m_ui->skipCompatibilityCheckBox->setChecked(settings->get("IgnoreJavaCompatibility").toBool());

    m_ui->javaArgumentsGroupBox->setChecked(m_instance == nullptr || settings->get("OverrideJavaArgs").toBool());
    m_ui->jvmArgsTextBox->setPlainText(settings->get("JvmArgs").toString());

    if (m_instance == nullptr) {
        m_ui->skipWizardCheckBox->setChecked(settings->get("IgnoreJavaWizard").toBool());
        m_ui->autodetectJavaCheckBox->setChecked(settings->get("AutomaticJavaSwitch").toBool());
        m_ui->autodetectJavaCheckBox->stateChanged(m_ui->autodetectJavaCheckBox->isChecked());
        m_ui->autodownloadJavaCheckBox->setChecked(settings->get("AutomaticJavaDownload").toBool());
    }

    // Memory
    m_ui->memoryGroupBox->setChecked(m_instance == nullptr || settings->get("OverrideMemory").toBool());
    int min = settings->get("MinMemAlloc").toInt();
    int max = settings->get("MaxMemAlloc").toInt();
    if (min < max) {
        m_ui->minMemSpinBox->setValue(min);
        m_ui->maxMemSpinBox->setValue(max);
    } else {
        m_ui->minMemSpinBox->setValue(max);
        m_ui->maxMemSpinBox->setValue(min);
    }
    m_ui->permGenSpinBox->setValue(settings->get("PermGen").toInt());

    // Java arguments
    m_ui->javaArgumentsGroupBox->setChecked(m_instance == nullptr || settings->get("OverrideJavaArgs").toBool());
    m_ui->jvmArgsTextBox->setPlainText(settings->get("JvmArgs").toString());
}

void JavaSettingsWidget::saveSettings()
{
    SettingsObject* settings;

    if (m_instance != nullptr)
        settings = m_instance->settings();
    else
        settings = APPLICATION->settings();

    SettingsObject::Lock lock(settings);

    // Java Install Settings
    bool javaInstall = m_instance == nullptr || m_ui->javaInstallationGroupBox->isChecked();

    if (m_instance != nullptr)
        settings->set("OverrideJavaLocation", javaInstall);

    if (javaInstall) {
        settings->set("JavaPath", m_ui->javaPathTextBox->text());
        settings->set("IgnoreJavaCompatibility", m_ui->skipCompatibilityCheckBox->isChecked());
    } else {
        settings->reset("JavaPath");
        settings->reset("IgnoreJavaCompatibility");
    }

    if (m_instance == nullptr) {
        settings->set("IgnoreJavaWizard", m_ui->skipWizardCheckBox->isChecked());
        settings->set("AutomaticJavaSwitch", m_ui->autodetectJavaCheckBox->isChecked());
        settings->set("AutomaticJavaDownload", m_ui->autodownloadJavaCheckBox->isChecked());
    }

    // Memory
    bool memory = m_instance == nullptr || m_ui->memoryGroupBox->isChecked();

    if (m_instance != nullptr)
        settings->set("OverrideMemory", memory);

    if (memory) {
        int min = m_ui->minMemSpinBox->value();
        int max = m_ui->maxMemSpinBox->value();
        if (min < max) {
            settings->set("MinMemAlloc", min);
            settings->set("MaxMemAlloc", max);
        } else {
            settings->set("MinMemAlloc", max);
            settings->set("MaxMemAlloc", min);
        }
        settings->set("PermGen", m_ui->permGenSpinBox->value());
    } else {
        settings->reset("MinMemAlloc");
        settings->reset("MaxMemAlloc");
        settings->reset("PermGen");
    }

    // Java arguments
    bool javaArgs = m_instance == nullptr || m_ui->javaArgumentsGroupBox->isChecked();

    if (m_instance != nullptr)
        settings->set("OverrideJavaArgs", javaArgs);

    if (javaArgs) {
        settings->set("JvmArgs", m_ui->jvmArgsTextBox->toPlainText().replace("\n", " "));
    } else {
        settings->reset("JvmArgs");
    }
}

void JavaSettingsWidget::onJavaBrowse()
{
    QString rawPath = QFileDialog::getOpenFileName(this, tr("Find Java executable"));

    // do not allow current dir - it's dirty. Do not allow dirs that don't exist
    if (rawPath.isEmpty()) {
        return;
    }

    QString cookedPath = FS::NormalizePath(rawPath);
    QFileInfo javaInfo(cookedPath);
    if (!javaInfo.exists() || !javaInfo.isExecutable()) {
        return;
    }
    m_ui->javaPathTextBox->setText(cookedPath);
}

void JavaSettingsWidget::onJavaTest()
{
    if (m_checker != nullptr)
        return;

    QString jvmArgs;

    if (m_instance == nullptr || m_ui->javaArgumentsGroupBox->isChecked())
        jvmArgs = m_ui->jvmArgsTextBox->toPlainText().replace("\n", " ");
    else
        jvmArgs = APPLICATION->settings()->get("JvmArgs").toString();

    m_checker.reset(new JavaCommon::TestCheck(this, m_ui->javaPathTextBox->text(), jvmArgs, m_ui->minMemSpinBox->value(),
                                              m_ui->maxMemSpinBox->value(), m_ui->permGenSpinBox->value()));
    connect(m_checker.get(), &JavaCommon::TestCheck::finished, this, [this] { m_checker.reset(); });
    m_checker->run();
}

void JavaSettingsWidget::onJavaAutodetect()
{
    if (JavaUtils::getJavaCheckPath().isEmpty()) {
        JavaCommon::javaCheckNotFound(this);
        return;
    }

    VersionSelectDialog versionDialog(APPLICATION->javalist(), tr("Select a Java version"), this, true);
    versionDialog.setResizeOn(2);
    versionDialog.exec();

    if (versionDialog.result() == QDialog::Accepted && versionDialog.selectedVersion()) {
        JavaInstallPtr java = std::dynamic_pointer_cast<JavaInstall>(versionDialog.selectedVersion());
        m_ui->javaPathTextBox->setText(java->path);

        if (!java->is_64bit && m_ui->maxMemSpinBox->value() > 2048) {
            CustomMessageBox::selectable(this, tr("Confirm Selection"),
                                         tr("You selected a 32-bit version of Java.\n"
                                            "This installation does not support more than 2048MiB of RAM.\n"
                                            "Please make sure that the maximum memory value is lower."),
                                         QMessageBox::Warning, QMessageBox::Ok, QMessageBox::Ok)
                ->exec();
        }
    }
}
void JavaSettingsWidget::updateThresholds()
{
    auto sysMiB = HardwareInfo::totalRamMiB();
    unsigned int maxMem = m_ui->maxMemSpinBox->value();
    unsigned int minMem = m_ui->minMemSpinBox->value();

    const QString warningColour(QStringLiteral("<span style='color:#f5c211'>%1</span>"));

    // BUG 4 fix: si no se pudo detectar la RAM, no mostrar warnings de capacidad
    if (sysMiB == 0) {
        if (maxMem < minMem) {
            m_ui->labelMaxMemNotice->setText(warningColour.arg(tr("Your maximum memory allocation is below the minimum memory allocation.")));
            m_ui->labelMaxMemNotice->show();
        } else {
            m_ui->labelMaxMemNotice->hide();
        }
        return;
    }

    if (maxMem >= sysMiB) {
        m_ui->labelMaxMemNotice->setText(
            QString("<span style='color:red'>%1</span>").arg(tr("Your maximum memory allocation exceeds your system memory capacity.")));
        m_ui->labelMaxMemNotice->show();
    } else if (maxMem > (sysMiB * 0.9)) {
        m_ui->labelMaxMemNotice->setText(warningColour.arg(tr("Your maximum memory allocation is close to your system memory capacity.")));
        m_ui->labelMaxMemNotice->show();
    } else if (maxMem > (sysMiB / 2) && sysMiB <= 16384) {
        // Con GPU integrada que comparte RAM, asignar >50% puede causar lag de VRAM
        int pct = static_cast<int>(maxMem * 100.0 / sysMiB);
        m_ui->labelMaxMemNotice->setText(warningColour.arg(
            tr("Estás asignando el %1% de la RAM del sistema. Con GPU integrada (VRAM compartida) "
               "esto puede causar lag. Se recomienda no superar el 50%.").arg(pct)));
        m_ui->labelMaxMemNotice->show();
    } else if (maxMem < minMem) {
        m_ui->labelMaxMemNotice->setText(warningColour.arg(tr("Your maximum memory allocation is below the minimum memory allocation.")));
        m_ui->labelMaxMemNotice->show();
    } else {
        m_ui->labelMaxMemNotice->hide();
    }
}

void JavaSettingsWidget::onApplyBetelineyProfile()
{
    int idx = m_ui->betelineyProfileComboBox->currentIndex();
    if (idx < 0 || idx >= BETELINEY_PROFILES.size())
        return;

    const BetelineyJVMProfile& profile = BETELINEY_PROFILES[idx];

    // Pedir confirmación si hay contenido previo que se va a sobreescribir
    bool hasExistingArgs = !m_ui->jvmArgsTextBox->toPlainText().trimmed().isEmpty();
    bool hasCustomMem = (m_ui->minMemSpinBox->value() != 0 || m_ui->maxMemSpinBox->value() != 0);
    bool isClearProfile = profile.jvmArgs.isEmpty();

    if (hasExistingArgs || (isClearProfile && hasCustomMem)) {
        QString msg = isClearProfile
            ? tr("Esto borrará todos los argumentos JVM y restablecerá la memoria a 0.\n¿Continuar?")
            : tr("Esto reemplazará los argumentos JVM actuales con el perfil \"%1\".\n¿Continuar?").arg(profile.name);

        auto answer = QMessageBox::question(
            this,
            tr("Aplicar perfil Beteliney"),
            msg,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if (answer != QMessageBox::Yes)
            return;
    }

    if (profile.minMemMB > 0)
        m_ui->minMemSpinBox->setValue(profile.minMemMB);
    else
        m_ui->minMemSpinBox->setValue(0);

    if (profile.maxMemMB > 0)
        m_ui->maxMemSpinBox->setValue(profile.maxMemMB);
    else
        m_ui->maxMemSpinBox->setValue(0);

    // Advertir si el usuario tiene -Xmx o -Xms en el text box (el perfil usa los spinboxes,
    // pero flags de memoria en los args de texto los sobrescriben en runtime)
    QString currentArgs = m_ui->jvmArgsTextBox->toPlainText();
    bool hasXmxInArgs = currentArgs.contains(QRegularExpression("-Xmx[0-9]+[kmgKMG]?"));
    bool hasXmsInArgs = currentArgs.contains(QRegularExpression("-Xms[0-9]+[kmgKMG]?"));
    if ((hasXmxInArgs || hasXmsInArgs) && !profile.jvmArgs.isEmpty()) {
        QStringList conflictFlags;
        if (hasXmxInArgs) conflictFlags << "-Xmx";
        if (hasXmsInArgs) conflictFlags << "-Xms";
        QMessageBox::warning(this,
            tr("Conflicto de memoria"),
            tr("Tus argumentos JVM contienen %1 que anulará los valores de memoria del perfil en runtime.\n\n"
               "Elimínalos del campo de argumentos si quieres que el perfil controle la memoria.")
                .arg(conflictFlags.join(tr(" y "))));
    }

    m_ui->jvmArgsTextBox->setPlainText(profile.jvmArgs.join(" "));
    updateThresholds();
}
