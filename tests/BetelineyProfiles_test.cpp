// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Tests para BetelineyProfiles
 */

#include <QTest>
#include "BetelineyProfiles.h"

class BetelineyProfilesTest : public QObject {
    Q_OBJECT

   private slots:

    // Hay al menos un perfil
    void test_NotEmpty()
    {
        QVERIFY(BETELINEY_PROFILES.size() > 0);
    }

    // El primer perfil es el de limpiar (sin flags, mem=0)
    void test_ClearProfileIsFirst()
    {
        const auto& clear = BETELINEY_PROFILES[0];
        QVERIFY(clear.jvmArgs.isEmpty());
        QCOMPARE(clear.minMemMB, 0);
        QCOMPARE(clear.maxMemMB, 0);
        QVERIFY(!clear.name.isEmpty());
        QVERIFY(!clear.description.isEmpty());
    }

    // Los perfiles reales (índice > 0) tienen mem y args válidos
    void test_RealProfilesHaveMemAndArgs()
    {
        for (int i = 1; i < BETELINEY_PROFILES.size(); ++i) {
            const auto& p = BETELINEY_PROFILES[i];
            QVERIFY2(p.minMemMB > 0, qPrintable(QString("Profile %1: minMemMB <= 0").arg(i)));
            QVERIFY2(p.maxMemMB > p.minMemMB, qPrintable(QString("Profile %1: maxMemMB <= minMemMB").arg(i)));
            QVERIFY2(!p.jvmArgs.isEmpty(), qPrintable(QString("Profile %1: no JVM args").arg(i)));
            QVERIFY2(!p.name.isEmpty(), qPrintable(QString("Profile %1: empty name").arg(i)));
        }
    }

    // Todos los perfiles reales usan G1GC (excepto los perfiles ZGC que usan su propio GC)
    void test_AllRealProfilesUseG1GC()
    {
        for (int i = 1; i < BETELINEY_PROFILES.size(); ++i) {
            const auto& p = BETELINEY_PROFILES[i];
            // Perfil ZGC usa -XX:+UseZGC en vez de G1GC — excluir de este check
            if (p.jvmArgs.contains("-XX:+UseZGC"))
                continue;
            QVERIFY2(p.jvmArgs.contains("-XX:+UseG1GC"),
                     qPrintable(QString("Profile %1 '%2' missing -XX:+UseG1GC").arg(i).arg(p.name)));
        }
    }

    // Los perfiles ZGC usan -XX:+UseZGC y NO usan flags G1GC
    void test_ZGCProfilesAreValid()
    {
        for (int i = 1; i < BETELINEY_PROFILES.size(); ++i) {
            const auto& p = BETELINEY_PROFILES[i];
            if (!p.jvmArgs.contains("-XX:+UseZGC"))
                continue;
            // ZGC profiles deben tener UseZGC pero NO G1GC
            QVERIFY2(!p.jvmArgs.contains("-XX:+UseG1GC"),
                     qPrintable(QString("ZGC Profile %1 '%2' has conflicting -XX:+UseG1GC").arg(i).arg(p.name)));
            // Deben tener SoftMaxHeapSize (característica de ZGC)
            bool hasSoftMax = false;
            for (const auto& arg : p.jvmArgs) {
                if (arg.startsWith("-XX:SoftMaxHeapSize=")) { hasSoftMax = true; break; }
            }
            QVERIFY2(hasSoftMax,
                     qPrintable(QString("ZGC Profile %1 '%2' missing -XX:SoftMaxHeapSize=").arg(i).arg(p.name)));
            // No deben incluir -XX:+UnlockExperimentalVMOptions (ZGC es producción en Java 21+)
            QVERIFY2(!p.jvmArgs.contains("-XX:+UnlockExperimentalVMOptions"),
                     qPrintable(QString("ZGC Profile %1 '%2' should not have UnlockExperimentalVMOptions (ZGC is production in Java 21+)").arg(i).arg(p.name)));
        }
    }

    // Ningún perfil incluye flags redundantes (UseCompressedOops / UseCompressedClassPointers)
    void test_NoRedundantFlags()
    {
        for (int i = 0; i < BETELINEY_PROFILES.size(); ++i) {
            const auto& p = BETELINEY_PROFILES[i];
            QVERIFY2(!p.jvmArgs.contains("-XX:+UseCompressedOops"),
                     qPrintable(QString("Profile %1 '%2' has redundant -XX:+UseCompressedOops").arg(i).arg(p.name)));
            QVERIFY2(!p.jvmArgs.contains("-XX:+UseCompressedClassPointers"),
                     qPrintable(QString("Profile %1 '%2' has redundant -XX:+UseCompressedClassPointers").arg(i).arg(p.name)));
        }
    }

    // Todos los perfiles reales tienen PerfDisableSharedMem (obligatorio con Hyper-V)
    void test_AllRealProfilesHavePerfDisableSharedMem()
    {
        for (int i = 1; i < BETELINEY_PROFILES.size(); ++i) {
            const auto& p = BETELINEY_PROFILES[i];
            QVERIFY2(p.jvmArgs.contains("-XX:+PerfDisableSharedMem"),
                     qPrintable(QString("Profile %1 '%2' missing -XX:+PerfDisableSharedMem").arg(i).arg(p.name)));
        }
    }

    // El perfil ligero (índice 1) tiene Xss >= 512k
    void test_LightProfileStackSize()
    {
        QVERIFY(BETELINEY_PROFILES.size() > 1);
        const auto& light = BETELINEY_PROFILES[1];
        bool hasValidXss = false;
        for (const auto& arg : light.jvmArgs) {
            if (arg.startsWith("-Xss")) {
                // Extraer valor: Xss768k o Xss1M
                QString val = arg.mid(4).toLower();
                int kb = 0;
                if (val.endsWith('k'))
                    kb = val.chopped(1).toInt();
                else if (val.endsWith('m'))
                    kb = val.chopped(1).toInt() * 1024;
                // Mínimo razonable: 512k
                if (kb >= 512)
                    hasValidXss = true;
            }
        }
        QVERIFY2(hasValidXss, "Light profile Xss too small (< 512k) or missing");
    }

    // Los perfiles no mencionan hardware específico en nombre ni descripción
    void test_NoHardwareSpecificStrings()
    {
        const QStringList hardware = { "Vega", "3700U", "MEDAPR", "BOE", "1366", "2400 MT" };
        for (const auto& p : BETELINEY_PROFILES) {
            for (const auto& hw : hardware) {
                QVERIFY2(!p.name.contains(hw, Qt::CaseInsensitive),
                         qPrintable(QString("Profile '%1' name contains hardware ref '%2'").arg(p.name).arg(hw)));
                QVERIFY2(!p.description.contains(hw, Qt::CaseInsensitive),
                         qPrintable(QString("Profile '%1' description contains hardware ref '%2'").arg(p.name).arg(hw)));
            }
        }
    }

    // Los perfiles no tienen args duplicados dentro del mismo perfil
    void test_NoDuplicateArgs()
    {
        for (int i = 0; i < BETELINEY_PROFILES.size(); ++i) {
            const auto& p = BETELINEY_PROFILES[i];
            QStringList seen;
            for (const auto& arg : p.jvmArgs) {
                // Solo checar flags XX: (ignorar -Xss, -Dusing, etc. que son únicos)
                if (arg.startsWith("-XX:+") || arg.startsWith("-XX:-")) {
                    QVERIFY2(!seen.contains(arg),
                             qPrintable(QString("Profile %1 '%2' has duplicate arg: %3").arg(i).arg(p.name).arg(arg)));
                    seen.append(arg);
                }
            }
        }
    }

    // Los perfiles reales tienen los flags de Aikar
    void test_AikarsFlags()
    {
        for (int i = 1; i < BETELINEY_PROFILES.size(); ++i) {
            const auto& p = BETELINEY_PROFILES[i];
            QVERIFY2(p.jvmArgs.contains("-Dusing.aikars.flags=https://mcflags.emc.gs"),
                     qPrintable(QString("Profile %1 missing Aikar attribution flag").arg(i)));
        }
    }

    // Los perfiles tienen orden lógico: max del anterior <= min del siguiente (solo perfiles G1GC)
    void test_ProfileMemoryOrder()
    {
        for (int i = 2; i < BETELINEY_PROFILES.size(); ++i) {
            const auto& prev = BETELINEY_PROFILES[i - 1];
            const auto& curr = BETELINEY_PROFILES[i];
            // Perfiles ZGC son alternativos a G1GC — no siguen el orden de memoria de la cadena G1
            if (curr.jvmArgs.contains("-XX:+UseZGC") || prev.jvmArgs.contains("-XX:+UseZGC"))
                continue;
            if (prev.maxMemMB == 0 || curr.maxMemMB == 0)
                continue;
            QVERIFY2(curr.maxMemMB >= prev.maxMemMB,
                     qPrintable(QString("Profile %1 maxMem (%2) < Profile %3 maxMem (%4)")
                                    .arg(i).arg(curr.maxMemMB).arg(i-1).arg(prev.maxMemMB)));
        }
    }
};

QTEST_GUILESS_MAIN(BetelineyProfilesTest)
#include "BetelineyProfiles_test.moc"
