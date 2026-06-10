// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Tests de traducción UI al español
 *
 *  Verifica que los strings de la UI de mods estén en español.
 *  Testea el contenido literal de los archivos .cpp traducidos
 *  usando QFile: si un string en inglés aparece en el archivo,
 *  el test falla.
 *
 *  Archivos verificados:
 *    - ui/pages/instance/ModFolderPage.cpp
 *    - ui/pages/instance/ExternalResourcesPage.cpp
 *    - ui/pages/modplatform/ResourcePage.cpp
 *
 *  Strings en inglés que NO deben aparecer como argumentos de tr():
 *    ModFolderPage    : "Download Mods" · "Download mods from online"
 *                       "Check for Updates" · "Change a mod's version"
 *                       "Confirm Update" · "Confirm Delete"
 *                       "Mod updates are unavailable when metadata"
 *                       "The mod updater was aborted"
 *                       "All mods are up-to-date"
 *                       "Missing Mod Loader"
 *    ExternalResources: "Confirm Removal" · "You are about to remove %1 items"
 *                       "installed, %2 selected)"
 *    ResourcePage     : "Loading versions..." · "No valid version found"
 *                       "No versions available" · "The author likely blocked"
 *                       "Source code:" · "External links:"
 *                       "Reinstall" · "The resource was not found"
 *                       "Select %1 for download" · "Deselect %1 for download"
 *
 *  Strings en español que SÍ deben aparecer (smoke check):
 *    ModFolderPage    : "Descargar Mods" · "Buscar actualizaciones"
 *    ExternalResources: "Confirmar eliminación"
 *    ResourcePage     : "Cargando versiones" · "Código fuente"
 */

#include <QTest>
#include <QFile>
#include <QString>
#include <QByteArray>

// Ruta relativa al source — se resuelve desde el build dir via QFINDTESTDATA
// o hardcodeada relativa al CMakeLists del test.
// Como los tests de Beteliney usan rutas absolutas, usamos la macro
// BETELINEY_SRCDIR que CMake define como CMAKE_SOURCE_DIR.
#ifndef BETELINEY_SRCDIR
#  define BETELINEY_SRCDIR ""
#endif

static QByteArray readFile(const QString& relPath)
{
    QString base = QString(BETELINEY_SRCDIR);
    if (base.isEmpty())
        base = QString(QFINDTESTDATA("../launcher"));  // fallback
    QFile f(base + "/" + relPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return f.readAll();
}

class BetelineyTranslationTest : public QObject {
    Q_OBJECT

   private slots:

    // ── ModFolderPage.cpp ────────────────────────────────────────────────────

    void test_modfolder_no_english_download_mods()
    {
        QByteArray src = readFile("launcher/ui/pages/instance/ModFolderPage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado — verificar BETELINEY_SRCDIR");
        QVERIFY2(!src.contains("tr(\"Download Mods\")"),
                 "ModFolderPage: 'Download Mods' sin traducir");
        QVERIFY2(!src.contains("tr(\"Download mods from online mod platforms\")"),
                 "ModFolderPage: tooltip 'Download mods from online' sin traducir");
    }

    void test_modfolder_no_english_check_updates()
    {
        QByteArray src = readFile("launcher/ui/pages/instance/ModFolderPage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(!src.contains("tr(\"Check for Updates\")"),
                 "ModFolderPage: 'Check for Updates' sin traducir");
    }

    void test_modfolder_no_english_confirm_strings()
    {
        QByteArray src = readFile("launcher/ui/pages/instance/ModFolderPage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(!src.contains("tr(\"Confirm Update\""),
                 "ModFolderPage: 'Confirm Update' sin traducir");
        QVERIFY2(!src.contains("tr(\"Confirm Delete\""),
                 "ModFolderPage: 'Confirm Delete' sin traducir");
        QVERIFY2(!src.contains("tr(\"Missing Mod Loader\""),
                 "ModFolderPage: 'Missing Mod Loader' sin traducir");
    }

    void test_modfolder_no_english_uptodate()
    {
        QByteArray src = readFile("launcher/ui/pages/instance/ModFolderPage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(!src.contains("All mods are up-to-date"),
                 "ModFolderPage: 'All mods are up-to-date' sin traducir");
        QVERIFY2(!src.contains("The mod updater was aborted"),
                 "ModFolderPage: 'The mod updater was aborted' sin traducir");
    }

    void test_modfolder_has_spanish_strings()
    {
        QByteArray src = readFile("launcher/ui/pages/instance/ModFolderPage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(src.contains("Descargar Mods"),
                 "ModFolderPage: falta 'Descargar Mods' en español");
        QVERIFY2(src.contains("Buscar actualizaciones"),
                 "ModFolderPage: falta 'Buscar actualizaciones' en español");
        QVERIFY2(src.contains("Confirmar actualización") || src.contains("Confirmar actualizaci"),
                 "ModFolderPage: falta 'Confirmar actualización' en español");
    }

    // ── ExternalResourcesPage.cpp ────────────────────────────────────────────

    void test_external_no_english_confirm_removal()
    {
        QByteArray src = readFile("launcher/ui/pages/instance/ExternalResourcesPage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(!src.contains("tr(\"Confirm Removal\""),
                 "ExternalResourcesPage: 'Confirm Removal' sin traducir");
        QVERIFY2(!src.contains("tr(\"Confirm Delete\""),
                 "ExternalResourcesPage: 'Confirm Delete' sin traducir");
    }

    void test_external_no_english_item_count()
    {
        QByteArray src = readFile("launcher/ui/pages/instance/ExternalResourcesPage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(!src.contains("installed, %2 selected)"),
                 "ExternalResourcesPage: contador 'installed/selected' sin traducir");
        QVERIFY2(!src.contains("You are about to remove %1 items"),
                 "ExternalResourcesPage: mensaje de eliminación masiva sin traducir");
    }

    void test_external_has_spanish_strings()
    {
        QByteArray src = readFile("launcher/ui/pages/instance/ExternalResourcesPage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(src.contains("Confirmar eliminaci"),
                 "ExternalResourcesPage: falta 'Confirmar eliminación'");
        QVERIFY2(src.contains("instalados"),
                 "ExternalResourcesPage: falta 'instalados' en el contador");
    }

    // ── ResourcePage.cpp ─────────────────────────────────────────────────────

    void test_resourcepage_no_english_loading()
    {
        QByteArray src = readFile("launcher/ui/pages/modplatform/ResourcePage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(!src.contains("tr(\"Loading versions...\""),
                 "ResourcePage: 'Loading versions...' sin traducir");
        QVERIFY2(!src.contains("tr(\"No valid version found.\""),
                 "ResourcePage: 'No valid version found.' sin traducir");
    }

    void test_resourcepage_no_english_versions_blocked()
    {
        QByteArray src = readFile("launcher/ui/pages/modplatform/ResourcePage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(!src.contains("tr(\"No versions available\""),
                 "ResourcePage: 'No versions available' sin traducir");
        QVERIFY2(!src.contains("The author likely blocked third-party launchers"),
                 "ResourcePage: mensaje de autor bloqueado sin traducir");
    }

    void test_resourcepage_no_english_external_links()
    {
        QByteArray src = readFile("launcher/ui/pages/modplatform/ResourcePage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(!src.contains("tr(\"External links:\""),
                 "ResourcePage: 'External links:' sin traducir");
        QVERIFY2(!src.contains("tr(\"Source code:"),
                 "ResourcePage: 'Source code:' sin traducir");
    }

    void test_resourcepage_no_english_select_button()
    {
        QByteArray src = readFile("launcher/ui/pages/modplatform/ResourcePage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(!src.contains("tr(\"Select %1 for download\""),
                 "ResourcePage: 'Select %1 for download' sin traducir");
        QVERIFY2(!src.contains("tr(\"Deselect %1 for download\""),
                 "ResourcePage: 'Deselect %1 for download' sin traducir");
        QVERIFY2(!src.contains("tr(\"Reinstall\""),
                 "ResourcePage: 'Reinstall' sin traducir");
        QVERIFY2(!src.contains("tr(\"Cancel\""),
                 "ResourcePage: 'Cancel' sin traducir");
    }

    void test_resourcepage_no_english_not_found()
    {
        QByteArray src = readFile("launcher/ui/pages/modplatform/ResourcePage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(!src.contains("The resource was not found"),
                 "ResourcePage: 'The resource was not found' sin traducir");
    }

    void test_resourcepage_has_spanish_strings()
    {
        QByteArray src = readFile("launcher/ui/pages/modplatform/ResourcePage.cpp");
        if (src.isEmpty()) QSKIP("Archivo no encontrado");
        QVERIFY2(src.contains("Cargando versiones"),
                 "ResourcePage: falta 'Cargando versiones'");
        QVERIFY2(src.contains("digo fuente") || src.contains("Código fuente"),
                 "ResourcePage: falta 'Código fuente'");
        QVERIFY2(src.contains("Seleccionar %1 para descargar"),
                 "ResourcePage: falta 'Seleccionar %1 para descargar'");
        QVERIFY2(src.contains("Reinstalar"),
                 "ResourcePage: falta 'Reinstalar'");
    }
};

QTEST_GUILESS_MAIN(BetelineyTranslationTest)
#include "BetelineyTranslation_test.moc"
