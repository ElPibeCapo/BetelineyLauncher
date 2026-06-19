// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "BetelineyPackPage.h"
#include "ui_BetelineyPackPage.h"

#include <QLabel>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>

#include "Application.h"
#include "modplatform/beteliney/BetelineyPackInstallTask.h"
#include "modplatform/beteliney/BetelineyPresets.h"
#include "ui/dialogs/NewInstanceDialog.h"

namespace Beteliney {

PackPage::PackPage(NewInstanceDialog* dialog, QWidget* parent)
    : QWidget(parent), m_ui(new Ui::BetelineyPackPage), m_dialog(dialog)
{
    m_ui->setupUi(this);
    m_model = std::make_unique<PackListModel>(this);

    connect(m_model.get(), &PackListModel::packLoaded,      this, &PackPage::onPackLoaded);
    connect(m_model.get(), &PackListModel::loadingFinished, this, &PackPage::onLoadingFinished);
    connect(m_model.get(), &PackListModel::loadingFailed,   this, &PackPage::onLoadingFailed);

    connect(m_ui->packList,  &QListWidget::currentRowChanged, this, [this](int) { onSelectionChanged(); });
    connect(m_ui->searchEdit, &QLineEdit::textChanged,         this, &PackPage::onSearchChanged);

    clearDetail();
}

PackPage::~PackPage() { delete m_ui; }

void PackPage::openedImpl()
{
    if (!m_loaded) {
        // Cargar presets built-in inmediatamente (sin red)
        for (const auto& preset : builtinPresets())
            onPackLoaded(preset);

        // Luego cargar packs remotos del índice
        m_model->load();
    }
}

void PackPage::retranslate()
{
    m_ui->retranslateUi(this);
}

// ── slots de modelo ─────────────────────────────────────────────────────────

void PackPage::onPackLoaded(const Beteliney::Pack& pack)
{
    // Añadir a lista de todos los packs ya filtrados por búsqueda actual
    QString search = m_ui->searchEdit->text().trimmed().toLower();
    if (!search.isEmpty()) {
        if (!pack.name.toLower().contains(search) &&
            !pack.description.toLower().contains(search) &&
            !pack.tags.join(' ').toLower().contains(search))
            return;
    }

    m_visible << pack;

    auto* item = new QListWidgetItem(pack.name);
    item->setData(Qt::UserRole, m_visible.size() - 1); // índice en m_visible

    // Sub-texto en la lista: MC version + loader
    QString sub = pack.minecraft;
    if (!pack.loader.isEmpty() && pack.loader != "vanilla")
        sub += QString(" • %1 %2").arg(pack.loader, pack.loaderVersion);
    if (pack.featured)
        sub += tr(" ★ Destacado");
    item->setToolTip(pack.description);
    item->setText(pack.name + "\n" + sub);

    // Icono: se descarga async y se asigna cuando llega
    loadIcon(pack, item);

    m_ui->packList->addItem(item);

    // Seleccionar automáticamente el primero que llega
    if (m_ui->packList->count() == 1) {
        m_ui->packList->setCurrentRow(0);
        onSelectionChanged();
    }
}

void PackPage::onLoadingFinished()
{
    m_loaded = true;
    if (m_visible.isEmpty())
        m_ui->statusLabel->setText(tr("No hay modpacks disponibles."));
    else
        m_ui->statusLabel->hide();
}

void PackPage::onLoadingFailed(const QString& reason)
{
    m_ui->statusLabel->setText(tr("Error al cargar packs: %1").arg(reason));
}

// ── selección y búsqueda ────────────────────────────────────────────────────

void PackPage::onSelectionChanged()
{
    int row = m_ui->packList->currentRow();
    if (row < 0 || row >= m_visible.size()) {
        clearDetail();
        return;
    }
    m_selected = row;
    showDetail(m_visible[row]);
    suggestCurrent();
}

void PackPage::onSearchChanged(const QString& text)
{
    // Reconstruir lista filtrada
    m_visible.clear();
    m_ui->packList->clear();
    m_selected = -1;
    clearDetail();

    QString lower = text.trimmed().toLower();
    for (const auto& pack : m_model->packs()) {
        if (!lower.isEmpty() &&
            !pack.name.toLower().contains(lower) &&
            !pack.description.toLower().contains(lower) &&
            !pack.tags.join(' ').toLower().contains(lower))
            continue;
        onPackLoaded(pack); // reutiliza la lógica de añadir item
    }
}

void PackPage::suggestCurrent()
{
    if (m_selected < 0 || m_selected >= m_visible.size())
        return;

    const Pack& pack = m_visible[m_selected];
    // stagingPath y globalSettings los setea NewInstanceDialog después
    // via setStagingPath() / setParentSettings() antes de ejecutar el task
    auto* task = new PackInstallTask(pack,
                                     {},                       // stagingPath: se sobreescribe
                                     APPLICATION->settings(),
                                     this);
    m_dialog->setSuggestedPack(pack.name, pack.version, task);
}

// ── panel de detalle ────────────────────────────────────────────────────────

void PackPage::showDetail(const Pack& pack)
{
    m_ui->nameLabel->setText(pack.name);

    QString meta = QString("Minecraft %1").arg(pack.minecraft);
    if (!pack.loader.isEmpty() && pack.loader != "vanilla")
        meta += QString(" · %1 %2").arg(pack.loader, pack.loaderVersion);
    meta += QString(" · v%1").arg(pack.version);
    m_ui->metaLabel->setText(meta);

    if (!pack.tags.isEmpty())
        m_ui->tagsLabel->setText("# " + pack.tags.join("  # "));
    else
        m_ui->tagsLabel->clear();

    QString desc = pack.longDescription.isEmpty() ? pack.description : pack.longDescription;
    m_ui->descriptionBrowser->setMarkdown(desc);
}

void PackPage::clearDetail()
{
    m_ui->nameLabel->clear();
    m_ui->metaLabel->clear();
    m_ui->tagsLabel->clear();
    m_ui->iconLabel->clear();
    m_ui->descriptionBrowser->clear();
}

// ── descarga de icono ────────────────────────────────────────────────────────

void PackPage::loadIcon(const Pack& pack, QListWidgetItem* item)
{
    if (pack.iconUrl.isEmpty())
        return;

    auto* nam = APPLICATION->network();
    auto* reply = nam->get(QNetworkRequest(QUrl(pack.iconUrl)));

    connect(reply, &QNetworkReply::finished, this, [this, reply, item, packId = pack.id] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError)
            return;

        QPixmap px;
        if (!px.loadFromData(reply->readAll()))
            return;

        QIcon icon(px);
        item->setIcon(icon);

        // Si este pack está seleccionado, actualizar también el icono de detalle
        int row = m_ui->packList->row(item);
        if (row == m_selected)
            m_ui->iconLabel->setPixmap(px.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    });
}

}  // namespace Beteliney
