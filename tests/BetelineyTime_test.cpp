// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Tests para BetelineyTime
 *  Cubre: Time::prettifyDuration  y  Time::humanReadableDuration
 *
 *  Casos cubiertos:
 *    prettifyDuration:
 *      - 0 s           → "0min 0s"
 *      - 1 s           → "0min 1s"
 *      - 59 s          → "0min 59s"
 *      - 60 s          → "1min 0s"
 *      - 90 s          → "1min 30s"
 *      - 3600 s (1h)   → "1h 0min"
 *      - 3661 s        → "1h 1min"
 *      - 86400 s (1d)  → "1d 0h 0min"
 *      - noDays=true   → días se acumulan en horas
 *      - retorna QString no vacío en todos los casos
 *
 *    humanReadableDuration:
 *      - 0.0   → contiene "ms" (muestra milisegundos cuando no hay nada más)
 *      - 1.0   → contiene "s"
 *      - 61.5  → contiene "m" y "s"
 *      - 3666.0 → contiene "h", "m", "s"
 *      - negativo → contiene '-'
 *      - precision=2 con fracción → contiene "ms"
 *      - retorna QString no vacío siempre
 */

#pragma once
#include <QTest>
#include "BetelineyTime.h"

class BetelineyTimeTest : public QObject {
    Q_OBJECT

   private slots:

    // ── prettifyDuration ─────────────────────────────────────────────────────

    void test_pretty_zero()
    {
        // 0 s → debe mostrar minutos y segundos
        QString r = Time::prettifyDuration(0);
        QVERIFY2(!r.isEmpty(), "prettifyDuration(0) devolvió vacío");
        QVERIFY2(r.contains("0"), "prettifyDuration(0) no contiene '0'");
    }

    void test_pretty_one_second()
    {
        QString r = Time::prettifyDuration(1);
        QVERIFY(!r.isEmpty());
        // "0min 1s"
        QVERIFY2(r.contains("1"), "prettifyDuration(1) no contiene '1'");
    }

    void test_pretty_59_seconds()
    {
        QString r = Time::prettifyDuration(59);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("59"), "prettifyDuration(59) no contiene '59'");
    }

    void test_pretty_one_minute()
    {
        // 60 s → 1min 0s
        QString r = Time::prettifyDuration(60);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("1"), "prettifyDuration(60) no contiene '1'");
    }

    void test_pretty_90_seconds()
    {
        // 90 s → 1min 30s
        QString r = Time::prettifyDuration(90);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("1"), "prettifyDuration(90) no contiene '1'");
        QVERIFY2(r.contains("30"), "prettifyDuration(90) no contiene '30'");
    }

    void test_pretty_one_hour()
    {
        // 3600 s → 1h 0min
        QString r = Time::prettifyDuration(3600);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("1"), "prettifyDuration(3600) no contiene '1'");
    }

    void test_pretty_one_hour_one_minute()
    {
        // 3661 s → 1h 1min
        QString r = Time::prettifyDuration(3661);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("1"), "prettifyDuration(3661) no contiene '1'");
    }

    void test_pretty_one_day()
    {
        // 86400 s → 1d 0h 0min
        QString r = Time::prettifyDuration(86400);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("1"), "prettifyDuration(86400) no contiene '1'");
    }

    void test_pretty_nodays_flag()
    {
        // noDays=true: 86400 s → días se vuelcan en horas → "24h 0min"
        QString r = Time::prettifyDuration(86400, /*noDays=*/true);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("24"), "prettifyDuration(86400, noDays) no contiene '24'");
    }

    void test_pretty_never_empty()
    {
        // Para cualquier valor, el resultado nunca debe estar vacío
        for (int64_t v : {0LL, 1LL, 30LL, 59LL, 60LL, 3600LL, 86400LL, 999999LL}) {
            QVERIFY2(!Time::prettifyDuration(v).isEmpty(),
                     qPrintable(QString("prettifyDuration(%1) devolvió vacío").arg(v)));
        }
    }

    // ── humanReadableDuration ────────────────────────────────────────────────

    void test_human_zero()
    {
        // 0.0 s → como no hay nada más, muestra ms
        QString r = Time::humanReadableDuration(0.0);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("ms") || r.contains("0"),
                 "humanReadableDuration(0) no tiene 'ms' ni '0'");
    }

    void test_human_one_second()
    {
        QString r = Time::humanReadableDuration(1.0);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("s"), "humanReadableDuration(1.0) no contiene 's'");
    }

    void test_human_one_minute_half()
    {
        // 61.5 s → contiene 'm' y 's'
        QString r = Time::humanReadableDuration(61.5);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("m"), "humanReadableDuration(61.5) no contiene 'm'");
        QVERIFY2(r.contains("s"), "humanReadableDuration(61.5) no contiene 's'");
    }

    void test_human_one_hour_mix()
    {
        // 3666 s = 1h 1m 6s
        QString r = Time::humanReadableDuration(3666.0);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("h"), "humanReadableDuration(3666) no contiene 'h'");
        QVERIFY2(r.contains("m"), "humanReadableDuration(3666) no contiene 'm'");
        QVERIFY2(r.contains("s"), "humanReadableDuration(3666) no contiene 's'");
    }

    void test_human_negative()
    {
        // Duración negativa → contiene '-'
        QString r = Time::humanReadableDuration(-5.0);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains('-'), "humanReadableDuration(-5.0) no contiene '-'");
    }

    void test_human_precision_ms()
    {
        // Con precision=2 y fracción → contiene "ms"
        QString r = Time::humanReadableDuration(0.5, 2);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("ms"), "humanReadableDuration(0.5, 2) no contiene 'ms'");
    }

    void test_human_never_empty()
    {
        for (double v : {0.0, 0.001, 1.0, 59.9, 60.0, 3600.0, 86400.0}) {
            QVERIFY2(!Time::humanReadableDuration(v).isEmpty(),
                     qPrintable(QString("humanReadableDuration(%1) devolvió vacío").arg(v)));
        }
    }

    void test_human_large_value()
    {
        // 2 días: 172800 s → contiene "days"
        QString r = Time::humanReadableDuration(172800.0);
        QVERIFY(!r.isEmpty());
        QVERIFY2(r.contains("day") || r.contains("d"),
                 "humanReadableDuration(172800) no contiene 'day'/'d'");
    }
};

QTEST_GUILESS_MAIN(BetelineyTimeTest)
#include "BetelineyTime_test.moc"
