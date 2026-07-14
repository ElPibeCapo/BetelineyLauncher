/* Copyright 2015-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "JsonFormat.h"

// FIXME: remove this from here... somehow
#include "Json.h"
#include "minecraft/OneSixVersionFormat.h"

#include "Index.h"
#include "Version.h"
#include "VersionList.h"

using namespace Json;

namespace Meta {

MetadataVersion currentFormatVersion()
{
    return MetadataVersion::InitialRelease;
}

// Auditoría de seguridad (sesión 38): 'uid' y 'version' vienen del feed de meta
// remoto y se usan sin más para construir rutas de archivo (ver
// VersionList::localFilename / Version::localFilename, y su consumo en
// BaseEntityLoadTask y HttpMetaCache::resolveEntry). FS::RemoveInvalidPathChars
// solo filtra caracteres inválidos de NTFS/FAT en Windows y NO bloquea '/' ni
// '..' en Linux, así que sin esta validación un feed comprometido (o un
// MetaURLOverride apuntando a un host hostil) podría inyectar un uid/version
// tipo "../../../.ssh" y leer, escribir o borrar fuera del directorio de cache
// esperado. Se rechaza cualquier valor que no sea un identificador seguro.
bool isSafePathComponent(const QString& value)
{
    if (value.isEmpty() || value.size() > 256)
        return false;
    if (value == "." || value == "..")
        return false;
    if (value.contains(QLatin1String("..")) || value.contains('/') || value.contains('\\') || value.contains(QChar(0)))
        return false;
    return true;
}

static QString requireSafePathComponent(const QJsonObject& obj, const QString& key)
{
    auto value = requireString(obj, key);
    if (!isSafePathComponent(value)) {
        throw ParseException(QObject::tr("Invalid '%1' in meta entry: contains illegal path characters").arg(key));
    }
    return value;
}

// Index
static std::shared_ptr<Index> parseIndexInternal(const QJsonObject& obj)
{
    const QList<QJsonObject> objects = requireIsArrayOf<QJsonObject>(obj, "packages");
    QList<VersionList::Ptr> lists;
    lists.reserve(objects.size());
    std::transform(objects.begin(), objects.end(), std::back_inserter(lists), [](const QJsonObject& obj) {
        VersionList::Ptr list = std::make_shared<VersionList>(requireSafePathComponent(obj, "uid"));
        list->setName(obj["name"].toString());
        list->setSha256(obj["sha256"].toString());
        return list;
    });
    return std::make_shared<Index>(lists);
}

// Version
static Version::Ptr parseCommonVersion(const QString& uid, const QJsonObject& obj)
{
    Version::Ptr version = std::make_shared<Version>(uid, requireSafePathComponent(obj, "version"));
    version->setTime(QDateTime::fromString(requireString(obj, "releaseTime"), Qt::ISODate).toMSecsSinceEpoch() / 1000);
    version->setType(obj["type"].toString());
    version->setRecommended(obj["recommended"].toBool());
    version->setVolatile(obj["volatile"].toBool());
    RequireSet reqs, conflicts;
    parseRequires(obj, &reqs, "requires");
    parseRequires(obj, &conflicts, "conflicts");
    version->setRequires(reqs, conflicts);
    if (auto sha256 = obj["sha256"].toString(); !sha256.isEmpty()) {
        version->setSha256(sha256);
    }
    return version;
}

static Version::Ptr parseVersionInternal(const QJsonObject& obj)
{
    Version::Ptr version = parseCommonVersion(requireSafePathComponent(obj, "uid"), obj);

    version->setData(OneSixVersionFormat::versionFileFromJson(
        QJsonDocument(obj), QString("%1/%2.json").arg(version->uid(), version->version()), obj.contains("order")));
    return version;
}

// Version list / package
static VersionList::Ptr parseVersionListInternal(const QJsonObject& obj)
{
    const QString uid = requireSafePathComponent(obj, "uid");

    const QList<QJsonObject> versionsRaw = requireIsArrayOf<QJsonObject>(obj, "versions");
    QList<Version::Ptr> versions;
    versions.reserve(versionsRaw.size());
    std::transform(versionsRaw.begin(), versionsRaw.end(), std::back_inserter(versions), [uid](const QJsonObject& vObj) {
        auto version = parseCommonVersion(uid, vObj);
        version->setProvidesRecommendations();
        return version;
    });

    VersionList::Ptr list = std::make_shared<VersionList>(uid);
    list->setName(obj["name"].toString());
    list->setVersions(versions);
    return list;
}

MetadataVersion parseFormatVersion(const QJsonObject& obj, bool required)
{
    if (!obj.contains("formatVersion")) {
        if (required) {
            return MetadataVersion::Invalid;
        }
        return MetadataVersion::InitialRelease;
    }
    if (!obj.value("formatVersion").isDouble()) {
        return MetadataVersion::Invalid;
    }
    switch (obj.value("formatVersion").toInt()) {
        case 0:
        case 1:
            return MetadataVersion::InitialRelease;
        default:
            return MetadataVersion::Invalid;
    }
}

void serializeFormatVersion(QJsonObject& obj, Meta::MetadataVersion version)
{
    if (version == MetadataVersion::Invalid) {
        return;
    }
    obj.insert("formatVersion", int(version));
}

void parseIndex(const QJsonObject& obj, Index* ptr)
{
    const MetadataVersion version = parseFormatVersion(obj);
    switch (version) {
        case MetadataVersion::InitialRelease:
            ptr->merge(parseIndexInternal(obj));
            break;
        case MetadataVersion::Invalid:
            throw ParseException(QObject::tr("Unknown format version!"));
    }
}

void parseVersionList(const QJsonObject& obj, VersionList* ptr)
{
    const MetadataVersion version = parseFormatVersion(obj);
    switch (version) {
        case MetadataVersion::InitialRelease:
            ptr->merge(parseVersionListInternal(obj));
            break;
        case MetadataVersion::Invalid:
            throw ParseException(QObject::tr("Unknown format version!"));
    }
}

void parseVersion(const QJsonObject& obj, Version* ptr)
{
    const MetadataVersion version = parseFormatVersion(obj);
    switch (version) {
        case MetadataVersion::InitialRelease:
            ptr->merge(parseVersionInternal(obj));
            break;
        case MetadataVersion::Invalid:
            throw ParseException(QObject::tr("Unknown format version!"));
    }
}

/*
[
{"uid":"foo", "equals":"version"}
]
*/
void parseRequires(const QJsonObject& obj, RequireSet* ptr, const char* keyName)
{
    if (obj.contains(keyName)) {
        auto reqArray = requireArray(obj, keyName);
        auto iter = reqArray.begin();
        while (iter != reqArray.end()) {
            auto reqObject = requireObject(*iter);
            // Sesión 39: 'uid' aquí alimenta Index::get()/ComponentUpdateTask
            // (inyección automática de dependencias) y esta misma función
            // parsea tanto el feed remoto como cachedRequires/cachedConflicts
            // de mmc-pack.json local — sin esta validación, un modpack
            // malicioso con un uid tipo "../../../.ssh" en sus "requires" se
            // inyecta como Component automáticamente en resolveDependencies(),
            // sin que el usuario interactúe, y termina en un path traversal.
            auto uid = requireSafePathComponent(reqObject, "uid");
            auto equals = reqObject["equals"].toString();
            auto suggests = reqObject["suggests"].toString();
            ptr->insert({ uid, equals, suggests });
            iter++;
        }
    }
}
void serializeRequires(QJsonObject& obj, RequireSet* ptr, const char* keyName)
{
    if (!ptr || ptr->empty()) {
        return;
    }
    QJsonArray arrOut;
    for (auto& iter : *ptr) {
        QJsonObject reqOut;
        reqOut.insert("uid", iter.uid);
        if (!iter.equalsVersion.isEmpty()) {
            reqOut.insert("equals", iter.equalsVersion);
        }
        if (!iter.suggests.isEmpty()) {
            reqOut.insert("suggests", iter.suggests);
        }
        arrOut.append(reqOut);
    }
    obj.insert(keyName, arrOut);
}

}  // namespace Meta
