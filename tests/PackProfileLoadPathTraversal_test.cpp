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

// Cierra el hueco documentado en MetaPathTraversal_test.cpp: componentFromJsonV1
// (PackProfile.cpp) valida el 'uid' de nivel superior de cada componente en
// mmc-pack.json con Meta::isSafePathComponent, pero esa función es
// estática/interna al .cpp y no se puede invocar directamente desde un test
// unitario sin construir una instancia real. Este test la ejerce de punta a
// punta: crea una MinecraftInstance real sobre un directorio temporal en
// disco, con un mmc-pack.json malicioso, y confirma que PackProfile::load()
// lo rechaza en vez de dejarlo llegar a Component::getFilename()
// (patchFilePathForUid), que es donde ocurriría el path traversal real:
// lectura/escritura/borrado arbitrario de archivos vía customize()/revert().

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

#include <FileSystem.h>
#include <minecraft/MinecraftInstance.h>
#include <minecraft/PackProfile.h>
#include <settings/INISettingsObject.h>

class PackProfileLoadPathTraversalTest : public QObject {
    Q_OBJECT

    // BaseInstance::BaseInstance() (launcher/BaseInstance.cpp) asume que estas
    // claves ya existen en el globalSettings real de la Application y las
    // pasa sin chequear null a registerOverride()/registerPassthrough() ->
    // SIGSEGV si faltan. En producción las registra
    // Application::initGlobalSettings() (launcher/Application.cpp); un
    // INISettingsObject "pelado" como el que arma este test no las trae por
    // defecto. Se replican los mismos defaults acá para poder construir una
    // MinecraftInstance real. Nota: loadSpecificSettings() (Java/ventana/
    // memoria/etc.) no corre en el constructor -- es lazy y PackProfile::load()
    // no la dispara -- así que esas claves no hacen falta acá.
    static void registerMinimalGlobalSettings(SettingsObject* globalSettings)
    {
        globalSettings->registerSetting("ShowGameTime", true);
        globalSettings->registerSetting("RecordGameTime", true);
        globalSettings->registerSetting("WrapperCommand", "");
        globalSettings->registerSetting({ "PreLaunchCommand", "PreLaunchCmd" }, "");
        globalSettings->registerSetting({ "PostExitCommand", "PostExitCmd" }, "");
        globalSettings->registerSetting("ShowConsole", false);
        globalSettings->registerSetting("AutoCloseConsole", true);
        globalSettings->registerSetting("ShowConsoleOnError", true);
        globalSettings->registerSetting("LogPrePostOutput", true);
        globalSettings->registerSetting("ConsoleMaxLines", 100000);
        globalSettings->registerSetting("ConsoleOverflowStop", true);
    }

    static void writeMmcPack(const QString& rootDir, const QString& uid)
    {
        QJsonObject component;
        component.insert("uid", uid);
        component.insert("version", "1.0");
        QJsonArray components;
        components.append(component);
        QJsonObject root;
        root.insert("formatVersion", 1);
        root.insert("components", components);

        FS::ensureFolderPathExists(FS::PathCombine(rootDir, "patches"));

        QFile file(FS::PathCombine(rootDir, "mmc-pack.json"));
        QVERIFY(file.open(QFile::WriteOnly));
        file.write(QJsonDocument(root).toJson());
        file.close();
    }

   private slots:
    void test_loadRejectsTraversalUid_data()
    {
        QTest::addColumn<QString>("uid");
        QTest::newRow("dotdot slash") << "../../../etc/cron.d/evil";
        QTest::newRow("absolute-ish") << "..";
        QTest::newRow("embedded dotdot") << "net.minecraft/../../evil";
        QTest::newRow("backslash") << "net.minecraft\\..\\evil";
    }

    // Regresión sesión 39/40: un mmc-pack.json de un modpack importado
    // (CurseForge/Modrinth/GDLauncher/zip manual) con un 'uid' malicioso no
    // debe llegar a instanciarse como Component ni tocar el disco fuera del
    // directorio de la instancia.
    void test_loadRejectsTraversalUid()
    {
        QFETCH(QString, uid);

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        writeMmcPack(tempDir.path(), uid);

        auto globalSettings = std::make_unique<INISettingsObject>(FS::PathCombine(tempDir.path(), "global.ini"));
        registerMinimalGlobalSettings(globalSettings.get());
        auto instanceSettings = std::make_unique<INISettingsObject>(FS::PathCombine(tempDir.path(), "instance.ini"));
        MinecraftInstance instance(globalSettings.get(), std::move(instanceSettings), tempDir.path());

        auto result = instance.getPackProfile()->load();

        QVERIFY(!result);
        QCOMPARE(instance.getPackProfile()->rowCount(), 0);
    }

    // Control positivo: un uid legítimo sí debe cargar y quedar accesible,
    // para que el rechazo de arriba no sea un falso positivo por un fixture
    // roto (mmc-pack.json malformado, instancia mal construida, etc.).
    void test_loadAcceptsLegitimateUid()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        writeMmcPack(tempDir.path(), "net.minecraftforge");

        auto globalSettings = std::make_unique<INISettingsObject>(FS::PathCombine(tempDir.path(), "global.ini"));
        registerMinimalGlobalSettings(globalSettings.get());
        auto instanceSettings = std::make_unique<INISettingsObject>(FS::PathCombine(tempDir.path(), "instance.ini"));
        MinecraftInstance instance(globalSettings.get(), std::move(instanceSettings), tempDir.path());

        auto result = instance.getPackProfile()->load();

        QVERIFY(result);
        QCOMPARE(instance.getPackProfile()->rowCount(), 1);
        QCOMPARE(instance.getPackProfile()->getComponent(size_t(0))->getID(), QString("net.minecraftforge"));
    }
};

QTEST_GUILESS_MAIN(PackProfileLoadPathTraversalTest)

#include "PackProfileLoadPathTraversal_test.moc"
