// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "BetelineyPackInstallTask.h"

#include <QCryptographicHash>
#include <QDir>
#include <QEventLoop>
#include <QFile>

#include "Application.h"
#include "FileSystem.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "net/ChecksumValidator.h"
#include "net/Download.h"
#include "net/NetJob.h"
#include "settings/INISettingsObject.h"

namespace Beteliney {

PackInstallTask::PackInstallTask(const Pack& pack,
                                 const QString& stagingPath,
                                 SettingsObject* globalSettings,
                                 QWidget* parent)
    : m_pack(pack), m_parent(parent)
{
    setStagingPath(stagingPath);
    setParentSettings(globalSettings);
    setName(pack.name);
}

bool PackInstallTask::abort()
{
    if (m_dlTask)
        m_dlTask->abort();
    return InstanceCreationTask::abort();
}

std::unique_ptr<MinecraftInstance> PackInstallTask::createInstance()
{
    // 1. Crear instancia
    QString configPath = FS::PathCombine(m_stagingPath, "instance.cfg");
    auto instanceSettings = std::make_unique<INISettingsObject>(configPath);
    auto instance = std::make_unique<MinecraftInstance>(
        m_globalSettings, std::move(instanceSettings), m_stagingPath);

    // 2. Configurar el pack profile (loader + MC)
    auto components = instance->getPackProfile();
    components->buildingFromScratch();
    components->setComponentVersion("net.minecraft", m_pack.minecraft, true);

    if (!m_pack.loader.isEmpty() && m_pack.loader != "vanilla" && !m_pack.loaderVersion.isEmpty()) {
        if (m_pack.loader == "fabric")
            components->setComponentVersion("net.fabricmc.fabric-loader", m_pack.loaderVersion);
        else if (m_pack.loader == "forge")
            components->setComponentVersion("net.minecraftforge", m_pack.loaderVersion);
        else if (m_pack.loader == "neoforge")
            components->setComponentVersion("net.neoforged", m_pack.loaderVersion);
        else if (m_pack.loader == "quilt")
            components->setComponentVersion("org.quiltmc.quilt-loader", m_pack.loaderVersion);
    }

    instance->setManagedPack("beteliney", m_pack.id, m_pack.name, m_pack.id, m_pack.version);
    instance->setName(name());
    instance->saveNow();

    // 3. Si no hay mods, listo
    if (m_pack.mods.isEmpty())
        return instance;

    // 4. Crear carpeta mods
    QString mcPath = FS::PathCombine(m_stagingPath, "minecraft");
    QString modsPath = FS::PathCombine(mcPath, "mods");
    if (!FS::ensureFolderPathExists(modsPath)) {
        setError(tr("No se pudo crear la carpeta mods."));
        return nullptr;
    }

    // 5. Descargar mods sincrónicamente con QEventLoop
    setStatus(tr("Descargando mods (%1 en total)…").arg(m_pack.mods.size()));

    auto job = makeShared<NetJob>(tr("BetelineyPacks: Mods"), APPLICATION->network());
    QList<QPair<QString, QString>> toVerify; // {path, sha512}

    for (const auto& mod : m_pack.mods) {
        // Solo descargamos mods con URL directa (Modrinth CDN o Direct)
        // CurseForge sin URL: los ToS no permiten redistribuir directamente
        if (mod.url.isEmpty())
            continue;

        QString filename = mod.filename;
        if (filename.isEmpty())
            filename = QUrl(mod.url).fileName();
        if (filename.isEmpty())
            filename = QString("%1-%2.jar").arg(mod.projectId, mod.version);

        QString destPath = FS::PathCombine(modsPath, filename);

        auto dl = Net::Download::makeFile(QUrl(mod.url), destPath);
        if (!mod.sha512.isEmpty()) {
            dl->addValidator(new Net::ChecksumValidator(
                QCryptographicHash::Sha512,
                QByteArray::fromHex(mod.sha512.toUtf8())));
        }
        job->addNetAction(dl);

        if (!mod.sha512.isEmpty())
            toVerify << qMakePair(destPath, mod.sha512);
    }

    m_dlTask = job;

    bool jobFailed = false;
    QString failReason;

    QEventLoop loop;
    connect(job.get(), &NetJob::succeeded, &loop, [&] { loop.quit(); });
    connect(job.get(), &NetJob::failed, &loop, [&](const QString& reason) {
        jobFailed = true;
        failReason = reason;
        loop.quit();
    });
    connect(job.get(), &NetJob::progress, this, [this](qint64 cur, qint64 total) {
        setProgress(cur, total);
    });
    connect(job.get(), &NetJob::aborted, &loop, &QEventLoop::quit);

    job->start();
    loop.exec();

    if (m_abort)
        return nullptr;

    if (jobFailed) {
        setError(tr("Error descargando mods: %1").arg(failReason));
        return nullptr;
    }

    // 6. Verificar hashes SHA-512
    setStatus(tr("Verificando integridad de los mods…"));
    for (const auto& [path, expectedHash] : toVerify) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            setError(tr("No se pudo abrir para verificar: %1").arg(QFileInfo(path).fileName()));
            return nullptr;
        }
        QCryptographicHash hasher(QCryptographicHash::Sha512);
        hasher.addData(&file);
        QString actual = hasher.result().toHex();
        if (actual.toLower() != expectedHash.toLower()) {
            setError(tr("Hash inválido para %1\nEsperado: %2\nObtenido:  %3\n\n"
                        "El archivo puede estar corrupto o ser malicioso.")
                         .arg(QFileInfo(path).fileName(), expectedHash, actual));
            return nullptr;
        }
    }

    return instance;
}

}  // namespace Beteliney
