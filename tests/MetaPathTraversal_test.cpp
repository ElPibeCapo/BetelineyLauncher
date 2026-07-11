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
};

QTEST_GUILESS_MAIN(MetaPathTraversalTest)

#include "MetaPathTraversal_test.moc"
