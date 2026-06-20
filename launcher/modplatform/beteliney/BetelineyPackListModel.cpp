// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "BetelineyPackListModel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "Application.h"
#include "BuildConfig.h"
#include "net/Download.h"
#include "net/NetJob.h"

namespace Beteliney {

PackListModel::PackListModel(QObject* parent) : QObject(parent) {}

void PackListModel::load()
{
    if (m_loading)
        return;
    m_loading = true;
    m_packs.clear();
    m_pendingIds.clear();
    m_loadedCount = 0;
    fetchIndex();
}

void PackListModel::reload()
{
    m_loading = false;
    load();
}

void PackListModel::fetchIndex()
{
    QString indexUrl = BuildConfig.BETELINEY_PACKS_URL + "index.json";

    auto job = makeShared<NetJob>("BetelineyPacks::Index", APPLICATION->network());
    auto [dl, response] = Net::Download::makeByteArray(QUrl(indexUrl));
    job->addNetAction(dl);

    connect(job.get(), &NetJob::succeeded, this, [this, response] { onIndexDownloaded(*response); });
    connect(job.get(), &NetJob::failed, this, [this](const QString& reason) {
        m_loading = false;
        emit loadingFailed(tr("No se pudo descargar el índice de BetelineyPacks: %1").arg(reason));
    });

    m_job = job;
    job->start();
}

void PackListModel::onIndexDownloaded(const QByteArray& data)
{
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        m_loading = false;
        emit loadingFailed(tr("El índice de BetelineyPacks tiene JSON inválido: %1").arg(err.errorString()));
        return;
    }

    auto root = doc.object();
    auto idsArray = root.value("ids").toArray();

    if (idsArray.isEmpty()) {
        m_loading = false;
        emit loadingFailed(tr("El índice de BetelineyPacks está vacío."));
        return;
    }

    for (const auto& v : idsArray)
        m_pendingIds << v.toString();

    for (const auto& id : m_pendingIds)
        fetchPack(id);
}

void PackListModel::fetchPack(const QString& id)
{
    QString url = BuildConfig.BETELINEY_PACKS_URL + id + ".json";

    auto job = makeShared<NetJob>(QString("BetelineyPacks::Pack::%1").arg(id), APPLICATION->network());
    auto [dl, response] = Net::Download::makeByteArray(QUrl(url));
    job->addNetAction(dl);

    connect(job.get(), &NetJob::succeeded, this, [this, response, id] { onPackDownloaded(*response, id); });
    connect(job.get(), &NetJob::failed, this, [this, id](const QString&) {
        // No falla todo — simplemente se salta el pack con error
        m_loadedCount++;
        checkAllLoaded();
    });

    job->start();
}

void PackListModel::onPackDownloaded(const QByteArray& data, const QString& /*id*/)
{
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        m_loadedCount++;
        checkAllLoaded();
        return;
    }

    auto root = doc.object();
    Pack pack;
    pack.formatVersion   = root.value("formatVersion").toInt(1);
    pack.id              = root.value("id").toString();
    pack.name            = root.value("name").toString();
    pack.description     = root.value("description").toString();
    pack.longDescription = root.value("longDescription").toString();
    pack.version         = root.value("version").toString();
    pack.minecraft       = root.value("minecraft").toString();
    pack.loader          = root.value("loader").toString();
    pack.loaderVersion   = root.value("loaderVersion").toString();
    pack.iconUrl         = root.value("icon").toString();
    pack.featured        = root.value("featured").toBool(false);

    for (const auto& s : root.value("screenshots").toArray())
        pack.screenshots << s.toString();

    for (const auto& t : root.value("tags").toArray())
        pack.tags << t.toString();

    for (const auto& m : root.value("mods").toArray()) {
        auto obj = m.toObject();
        PackMod mod;
        QString provider = obj.value("provider").toString("modrinth").toLower();
        if (provider == "modrinth")       mod.provider = PackProvider::Modrinth;
        else if (provider == "curseforge") mod.provider = PackProvider::CurseForge;
        else                               mod.provider = PackProvider::Direct;

        mod.projectId = obj.value("projectId").toString();
        mod.version   = obj.value("version").toString();
        mod.url       = obj.value("url").toString();
        mod.sha512    = obj.value("sha512").toString();
        mod.filename  = obj.value("filename").toString();
        pack.mods << mod;
    }

    if (!pack.id.isEmpty() && !pack.name.isEmpty()) {
        m_packs << pack;
        emit packLoaded(pack);
    }

    m_loadedCount++;
    checkAllLoaded();
}

void PackListModel::checkAllLoaded()
{
    if (m_loadedCount >= m_pendingIds.size()) {
        m_loading = false;
        // Ordenar: primero los destacados, luego por nombre
        std::stable_sort(m_packs.begin(), m_packs.end(), [](const Pack& a, const Pack& b) {
            if (a.featured != b.featured)
                return a.featured > b.featured;
            return a.name < b.name;
        });
        emit loadingFinished();
    }
}

}  // namespace Beteliney
