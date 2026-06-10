// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Tests para BetelineyZip::collectFileListRecursively
 */

#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include "BetelineyZip.h"

class BetelineyZipTest : public QObject {
    Q_OBJECT

   private slots:

    // Lista todos los archivos recursivamente
    void test_collectAll()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        QFile(dir.filePath("a.txt")).open(QIODevice::WriteOnly);
        QFile(dir.filePath("b.txt")).open(QIODevice::WriteOnly);
        QDir(dir.path()).mkdir("sub");
        QFile(dir.filePath("sub/c.txt")).open(QIODevice::WriteOnly);

        QFileInfoList result;
        bool ok = BetelineyZip::collectFileListRecursively(dir.path(), nullptr, &result, nullptr);

        QVERIFY(ok);
        QCOMPARE(result.size(), 3);

        QStringList names;
        for (const auto& f : result)
            names << f.fileName();
        QVERIFY(names.contains("a.txt"));
        QVERIFY(names.contains("b.txt"));
        QVERIFY(names.contains("c.txt"));
    }

    // Directorio vacío → lista vacía, retorna true
    void test_emptyDir()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFileInfoList result;
        bool ok = BetelineyZip::collectFileListRecursively(dir.path(), nullptr, &result, nullptr);
        QVERIFY(ok);
        QCOMPARE(result.size(), 0);
    }

    // Directorio inexistente → retorna false
    void test_nonexistentDir()
    {
        QFileInfoList result;
        bool ok = BetelineyZip::collectFileListRecursively("/nonexistent/path/xyz", nullptr, &result, nullptr);
        QVERIFY2(!ok, "Should return false for nonexistent directory");
        QCOMPARE(result.size(), 0);
    }

    // Filtro de exclusión elimina archivos correctamente
    void test_withExcludeFilter()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        QFile(dir.filePath("keep.txt")).open(QIODevice::WriteOnly);
        QFile(dir.filePath("MANIFEST.MF")).open(QIODevice::WriteOnly);

        QFileInfoList result;
        bool ok = BetelineyZip::collectFileListRecursively(dir.path(), nullptr, &result,
            [](const QFileInfo& file) { return file.fileName().contains("MANIFEST"); });

        QVERIFY(ok);
        QCOMPARE(result.size(), 1);
        QCOMPARE(result[0].fileName(), QString("keep.txt"));
    }

    // Archivos ocultos no se incluyen (QDir::Files no los retorna por defecto)
    void test_filesOnly_noDirectories()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        QFile(dir.filePath("file.txt")).open(QIODevice::WriteOnly);
        QDir(dir.path()).mkdir("subdir");  // no debe aparecer en results

        QFileInfoList result;
        BetelineyZip::collectFileListRecursively(dir.path(), nullptr, &result, nullptr);

        for (const auto& f : result)
            QVERIFY2(f.isFile(), qPrintable(QString("Non-file in results: %1").arg(f.filePath())));
    }
};

QTEST_GUILESS_MAIN(BetelineyZipTest)
#include "BetelineyZip_test.moc"
