// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include <QObject>
#include <QList>
#include <memory>

#include "BetelineyPack.h"
#include "net/NetJob.h"
#include "tasks/Task.h"

namespace Beteliney {

/**
 * Descarga el índice de BetelineyPacks y luego cada pack individual.
 * Emite packLoaded() por cada pack que se carga correctamente.
 * Emite finished() cuando todos los packs están listos.
 */
class PackListModel : public QObject {
    Q_OBJECT
public:
    explicit PackListModel(QObject* parent = nullptr);

    void load();
    void reload();
    const QList<Pack>& packs() const { return m_packs; }
    bool isLoading() const { return m_loading; }

signals:
    void packLoaded(const Beteliney::Pack& pack);
    void loadingFinished();
    void loadingFailed(const QString& reason);

private:
    void fetchIndex();
    void fetchPack(const QString& id);
    void onIndexDownloaded(const QByteArray& data);
    void onPackDownloaded(const QByteArray& data, const QString& id);
    void checkAllLoaded();

    QList<Pack> m_packs;
    QStringList m_pendingIds;
    int m_loadedCount = 0;
    bool m_loading = false;
    NetJob::Ptr m_job;
};

}  // namespace Beteliney
