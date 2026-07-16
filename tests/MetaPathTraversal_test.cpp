// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
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
 */

// Regresión de seguridad (sesión 38): 'uid' y 'version' en el feed de meta remoto
// se usan para construir rutas de archivo (VersionList::localFilename,
// Version::localFilename). Sin validación, un feed comprometido podía inyectar
// '../' y escapar del directorio de cache (path traversal). Este test fija que
// el parser rechaza esos valores en vez de aceptarlos silenciosamente.
//
// Addendum sesión 39: el mismo patrón existía en 'Require::uid' (campo "uid"
// dentro de arrays "requires"/"conflicts"), parseado por Meta::parseRequires().
// Esa función es compartida por el feed remoto Y por mmc-pack.json de la
// instancia local (PackProfile.cpp lee cachedRequires/cachedConflicts, y
// OneSixVersionFormat.cpp lee patches/*.json) — un Require::uid inseguro ahí
// se inyecta automáticamente como Component nuevo en
// ComponentUpdateTask::resolveDependencies(), sin interacción del usuario,
// terminando en el mismo path traversal. Ver también: PackProfile.cpp
// componentFromJsonV1 ahora valida el 'uid' de nivel superior del componente
// con Meta::isSafePathComponent (expuesta públicamente en JsonFormat.h para
// ese fin) — esa función es estática/interna a PackProfile.cpp, así que se
// prueba de punta a punta (construyendo una MinecraftInstance real sobre
// disco) en tests/PackProfileLoadPathTraversal_test.cpp en vez de aquí.

#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include <meta/Index.h>
#include <meta/JsonFormat.h>
#include <meta/Version.h>
#include <meta/VersionList.h>

class MetaPathTraversalTest : public QObject {
    Q_OBJECT

   private slots:
    void test_indexRejectsTraversalUid_data()
    {
        QTest::addColumn<QString>("uid");
        QTest::newRow("dotdot slash") << "../../../etc/cron.d/evil";
        QTest::newRow("absolute-ish") << "..";
        QTest::newRow("embedded dotdot") << "net.minecraft/../../evil";
        QTest::newRow("backslash") << "net.minecraft\\..\\evil";
        QTest::newRow("empty") << "";
    }

    void test_indexRejectsTraversalUid()
    {
        QFETCH(QString, uid);

        QJsonObject pkg;
        pkg.insert("uid", uid);
        pkg.insert("name", "Evil");
        QJsonArray packages;
        packages.append(pkg);
        QJsonObject root;
        root.insert("formatVersion", 1);
        root.insert("packages", packages);

        Meta::Index index;
        bool threw = false;
        try {
            Meta::parseIndex(root, &index);
        } catch (const Meta::ParseException&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void test_versionListRejectsTraversalUid()
    {
        QJsonObject root;
        root.insert("formatVersion", 1);
        root.insert("uid", "../../../home/user/.ssh");
        root.insert("name", "Evil");
        root.insert("versions", QJsonArray());

        Meta::VersionList list("placeholder");
        bool threw = false;
        try {
            Meta::parseVersionList(root, &list);
        } catch (const Meta::ParseException&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void test_versionRejectsTraversalVersion()
    {
        QJsonObject root;
        root.insert("formatVersion", 1);
        root.insert("uid", "net.minecraft");
        root.insert("version", "../../../etc/passwd");
        root.insert("releaseTime", "2026-01-01T00:00:00+00:00");

        Meta::Version version("net.minecraft", "placeholder");
        bool threw = false;
        try {
            Meta::parseVersion(root, &version);
        } catch (const Meta::ParseException&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void test_indexAcceptsLegitimateUid()
    {
        QJsonObject pkg;
        pkg.insert("uid", "net.minecraftforge");
        pkg.insert("name", "Forge");
        QJsonArray packages;
        packages.append(pkg);
        QJsonObject root;
        root.insert("formatVersion", 1);
        root.insert("packages", packages);

        Meta::Index index;
        Meta::parseIndex(root, &index);
        QVERIFY(index.hasUid("net.minecraftforge"));
    }

    // Sesión 39: Require::uid (campo "uid" dentro de "requires"/"conflicts")
    // no pasaba por Meta::isSafePathComponent. Esto es explotable sin
    // interacción del usuario vía inyección automática de dependencias
    // (ComponentUpdateTask::resolveDependencies -> Index::get(dep.uid)).
    void test_requiresRejectsTraversalUid_data()
    {
        QTest::addColumn<QString>("uid");
        QTest::newRow("dotdot slash") << "../../../home/user/.ssh";
        QTest::newRow("absolute-ish") << "..";
        QTest::newRow("embedded dotdot") << "org.lwjgl/../../evil";
        QTest::newRow("backslash") << "org.lwjgl\\..\\evil";
        QTest::newRow("empty") << "";
    }

    void test_requiresRejectsTraversalUid()
    {
        QFETCH(QString, uid);

        QJsonObject req;
        req.insert("uid", uid);
        QJsonArray requiresArray;
        requiresArray.append(req);
        QJsonObject root;
        root.insert("requires", requiresArray);

        Meta::RequireSet reqs;
        bool threw = false;
        try {
            Meta::parseRequires(root, &reqs);
        } catch (const Meta::ParseException&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void test_requiresAcceptsLegitimateUid()
    {
        QJsonObject req;
        req.insert("uid", "org.lwjgl3");
        req.insert("suggests", "3.1.2");
        QJsonArray requiresArray;
        requiresArray.append(req);
        QJsonObject root;
        root.insert("requires", requiresArray);

        Meta::RequireSet reqs;
        Meta::parseRequires(root, &reqs);
        QCOMPARE(reqs.size(), size_t(1));
        QCOMPARE(reqs.begin()->uid, QString("org.lwjgl3"));
    }
};

QTEST_GUILESS_MAIN(MetaPathTraversalTest)

#include "MetaPathTraversal_test.moc"
