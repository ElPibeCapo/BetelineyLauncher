// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include <QPixmap>
#include <QWidget>
#include <memory>

#include "modplatform/beteliney/BetelineyPack.h"
#include "modplatform/beteliney/BetelineyPackListModel.h"
#include "ui/pages/BasePage.h"
#include "ui/dialogs/NewInstanceDialog.h"

namespace Ui {
class BetelineyPackPage;
}

namespace Beteliney {

class PackPage : public QWidget, public BasePage {
    Q_OBJECT
public:
    explicit PackPage(NewInstanceDialog* dialog, QWidget* parent = nullptr);
    ~PackPage() override;

    // BasePage
    QString displayName() const override { return tr("Beteliney"); }
    QIcon   icon()        const override { return QIcon::fromTheme("beteliney"); }
    QString id()          const override { return "beteliney"; }
    QString helpPage()    const override { return {}; }

    void openedImpl() override;
    void retranslate() override;

private slots:
    void onPackLoaded(const Beteliney::Pack& pack);
    void onLoadingFinished();
    void onLoadingFailed(const QString& reason);
    void onSelectionChanged();
    void onSearchChanged(const QString& text);
    void suggestCurrent();

private:
    void showDetail(const Pack& pack);
    void clearDetail();
    void loadIcon(const Pack& pack, QListWidgetItem* item);

    Ui::BetelineyPackPage*               m_ui      = nullptr;
    NewInstanceDialog*                   m_dialog  = nullptr;
    std::unique_ptr<PackListModel>       m_model;
    QList<Pack>                          m_visible; // packs filtrados
    int                                  m_selected = -1;
    bool                                 m_loaded   = false;
};

}  // namespace Beteliney
