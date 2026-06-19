// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "GDLauncherMigrator.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTextStream>
#include <QUuid>

namespace Beteliney {

// ── Detección del directorio GDLauncher ───────────────────────────────────

QString gdlauncherDataDir()
{
    // Candidatos ordenados por preferencia
    QStringList candidates;

#if defined(Q_OS_WIN32)
    QString appData  = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString localApp = QDir::fromNativeSeparators(qEnvironmentVariable("LOCALAPPDATA"));
    // AppDataLocation en Windows apunta a %APPDATA%\<AppName> — necesitamos el padre
    appData = QFileInfo(appData).absolutePath();

    candidates << appData  + "/gdlauncher_next"
               << localApp + "/gdlauncher_next"
               << appData  + "/gdlauncher"
               << localApp + "/gdlauncher";
#else
    QString home = QDir::homePath();
    candidates << home + "/.local/share/gdlauncher_next"
               << home + "/.local/share/gdlauncher"
               << home + "/.config/gdlauncher_next"
               << home + "/.config/gdlauncher";
#endif

    for (const QString& path : candidates) {
        if (QDir(path).exists() && QFile::exists(path + "/data.sqlite"))
            return path;
    }
    return {};
}

// ── Lectura de instancias desde SQLite ────────────────────────────────────

QList<GDInstance> readGDInstances(const QString& dataDir, QString* errorOut)
{
    QList<GDInstance> result;

    // Conexión única por llamada con nombre único para evitar conflictos
    const QString connName = QStringLiteral("gdl_migrator_") +
                             QUuid::createUuid().toString(QUuid::WithoutBraces);

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
        db.setDatabaseName(dataDir + "/data.sqlite");
        db.setConnectOptions("QSQLITE_OPEN_READONLY");

        if (!db.open()) {
            if (errorOut)
                *errorOut = db.lastError().text();
            QSqlDatabase::removeDatabase(connName);
            return result;
        }

        // GDLauncher Carbon usa tabla "instance" (singular)
        // Fallback a "instances" por compatibilidad con versiones antiguas
        QStringList tableNames = db.tables();
        QString tableName;
        if (tableNames.contains("instance"))
            tableName = "instance";
        else if (tableNames.contains("instances"))
            tableName = "instances";
        else {
            if (errorOut)
                *errorOut = QObject::tr("No se encontró la tabla de instancias en data.sqlite. "
                                        "Tablas disponibles: %1").arg(tableNames.join(", "));
            db.close();
            QSqlDatabase::removeDatabase(connName);
            return result;
        }

        // Obtener columnas disponibles para adaptar la query
        QSqlQuery colQuery(db);
        colQuery.exec(QString("PRAGMA table_info(%1)").arg(tableName));
        QStringList cols;
        while (colQuery.next())
            cols << colQuery.value(1).toString();  // columna "name" en PRAGMA

        // Construir SELECT adaptativo
        auto bestCol = [&](std::initializer_list<const char*> names) -> QString {
            for (const char* n : names)
                if (cols.contains(QLatin1String(n))) return QLatin1String(n);
            return "NULL";
        };

        QString selectSql = QString("SELECT %1 AS id, %2 AS name, %3 AS mc_version, "
                            "       %4 AS loader, %5 AS loader_version, %6 AS shortpath "
                            "FROM %7")
            .arg(bestCol({"id"}))
            .arg(bestCol({"name"}))
            .arg(bestCol({"mc_version", "mcVersion", "minecraft_version"}))
            .arg(bestCol({"modloader", "loader", "mod_loader", "modLoader"}))
            .arg(bestCol({"modloader_version", "loaderVersion", "loader_version", "modloaderVersion"}))
            .arg(bestCol({"shortpath", "short_path", "path", "instance_path"}))
            .arg(tableName);

        QSqlQuery q(db);
        if (!q.exec(selectSql)) {
            if (errorOut)
                *errorOut = q.lastError().text();
            db.close();
            QSqlDatabase::removeDatabase(connName);
            return result;
        }

        while (q.next()) {
            GDInstance inst;
            inst.id            = q.value("id").toString();
            inst.name          = q.value("name").toString();
            inst.mcVersion     = q.value("mc_version").toString();
            inst.loader        = q.value("loader").toString().toLower();
            inst.loaderVersion = q.value("loader_version").toString();
            inst.shortpath     = q.value("shortpath").toString();

            if (inst.name.isEmpty())
                continue;

            // Construir ruta de origen
            if (!inst.shortpath.isEmpty())
                inst.sourcePath = dataDir + "/instances/" + inst.shortpath;
            else if (!inst.id.isEmpty())
                inst.sourcePath = dataDir + "/instances/" + inst.id;

            result << inst;
        }

        db.close();
    }
    QSqlDatabase::removeDatabase(connName);
    return result;
}

// ── Importar una instancia al formato Prism ───────────────────────────────

static bool copyDirRecursive(const QString& src, const QString& dst)
{
    QDir srcDir(src);
    if (!srcDir.exists())
        return false;

    QDir dstDir(dst);
    if (!dstDir.exists() && !QDir().mkpath(dst))
        return false;

    for (const QFileInfo& fi : srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        QString srcPath = fi.absoluteFilePath();
        QString dstPath = dst + "/" + fi.fileName();

        if (fi.isDir()) {
            if (!copyDirRecursive(srcPath, dstPath))
                return false;
        } else {
            QFile::remove(dstPath);  // sobreescribir si existe
            if (!QFile::copy(srcPath, dstPath))
                return false;
        }
    }
    return true;
}

QString importGDInstance(const GDInstance& inst, const QString& destInstancesDir)
{
    // Crear directorio destino con nombre sanitizado
    QString safeName = inst.name;
    safeName.replace(QRegularExpression(R"([\\/:*?"<>|])"), "_");
    if (safeName.isEmpty())
        safeName = "GDL_" + inst.id;

    QString destPath = destInstancesDir + "/" + safeName;

    // Si ya existe, añadir sufijo
    if (QDir(destPath).exists()) {
        int idx = 2;
        while (QDir(destPath + "_" + QString::number(idx)).exists())
            ++idx;
        destPath += "_" + QString::number(idx);
        safeName += "_" + QString::number(idx);
    }

    if (!QDir().mkpath(destPath))
        return QObject::tr("No se pudo crear el directorio: %1").arg(destPath);

    // 1. Escribir instance.cfg (formato Prism)
    {
        QFile cfg(destPath + "/instance.cfg");
        if (!cfg.open(QIODevice::WriteOnly | QIODevice::Text))
            return QObject::tr("No se pudo crear instance.cfg");

        QTextStream ts(&cfg);
        ts << "[General]\n";
        ts << "ConfigVersion=1.2\n";
        ts << "InstanceType=OneSix\n";
        ts << "name=" << inst.name << "\n";
        ts << "iconKey=grass\n";
        ts << "JoinServerOnLaunch=false\n";
        ts << "OverrideCommands=false\n";
        ts << "OverrideConsole=false\n";
        ts << "OverrideJavaArgs=false\n";
        ts << "OverrideJavaLocation=false\n";
        ts << "OverrideMemory=false\n";
        ts << "OverrideNativeWorkarounds=false\n";
        ts << "notes=Importada desde GDLauncher Carbon\n";
    }

    // 2. Escribir mmc-pack.json (componentes: MC version + loader)
    {
        QJsonObject pack;
        pack["formatVersion"] = 1;
        pack["name"]          = inst.name;

        QJsonArray components;

        // Componente Minecraft
        if (!inst.mcVersion.isEmpty()) {
            QJsonObject mc;
            mc["uid"]     = "net.minecraft";
            mc["version"] = inst.mcVersion;
            components.append(mc);
        }

        // Componente loader
        if (inst.loader == "fabric") {
            QJsonObject fab;
            fab["uid"]     = "net.fabricmc.fabric-loader";
            fab["version"] = inst.loaderVersion;
            components.append(fab);
        } else if (inst.loader == "quilt") {
            QJsonObject q;
            q["uid"]     = "org.quiltmc.quilt-loader";
            q["version"] = inst.loaderVersion;
            components.append(q);
        } else if (inst.loader == "forge") {
            QJsonObject f;
            f["uid"]     = "net.minecraftforge";
            f["version"] = inst.loaderVersion;
            components.append(f);
        } else if (inst.loader == "neoforge") {
            QJsonObject n;
            n["uid"]     = "net.neoforged.neoforge";
            n["version"] = inst.loaderVersion;
            components.append(n);
        }

        pack["components"] = components;

        QFile packFile(destPath + "/mmc-pack.json");
        if (!packFile.open(QIODevice::WriteOnly))
            return QObject::tr("No se pudo crear mmc-pack.json");

        packFile.write(QJsonDocument(pack).toJson(QJsonDocument::Indented));
    }

    // 3. Copiar archivos del juego (.minecraft/)
    if (!inst.sourcePath.isEmpty() && QDir(inst.sourcePath).exists()) {
        // GDLauncher puede guardar el .minecraft directamente o en un subdirectorio
        QStringList possibleMc = {
            inst.sourcePath + "/.minecraft",
            inst.sourcePath + "/minecraft",
            inst.sourcePath,  // último recurso: copiar todo
        };

        bool copied = false;
        for (const QString& mcSrc : possibleMc) {
            if (QDir(mcSrc).exists()) {
                if (!copyDirRecursive(mcSrc, destPath + "/.minecraft"))
                    return QObject::tr("Error copiando archivos del juego desde: %1").arg(mcSrc);
                copied = true;
                break;
            }
        }

        if (!copied) {
            // No es un error fatal — la instancia se puede completar descargando desde MC
            QFile warn(destPath + "/AVISO_MIGRACIÓN.txt");
            if (warn.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream ts(&warn);
                ts << "La carpeta de archivos de GDLauncher no fue encontrada:\n";
                ts << inst.sourcePath << "\n\n";
                ts << "La instancia se importó con configuración básica.\n";
                ts << "Lanza la instancia para descargar los archivos del juego.\n";
            }
        }
    }

    return {};  // éxito
}

}  // namespace Beteliney
