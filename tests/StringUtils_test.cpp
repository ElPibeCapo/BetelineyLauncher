// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Tests para StringUtils::splitFirst
 */

#include <QTest>
#include "StringUtils.h"

class StringUtilsTest : public QObject {
    Q_OBJECT

   private slots:

    // --- splitFirst(QString, QString) ---

    void test_splitFirst_QString_normal()
    {
        auto [left, right] = StringUtils::splitFirst("Name: valor", ": ");
        QCOMPARE(left,  QString("Name"));
        QCOMPARE(right, QString("valor"));
    }

    void test_splitFirst_QString_sepNotFound()
    {
        auto [left, right] = StringUtils::splitFirst("sin separador", ": ");
        QCOMPARE(left,  QString("sin separador"));
        QCOMPARE(right, QString(""));
    }

    void test_splitFirst_QString_emptyString()
    {
        auto [left, right] = StringUtils::splitFirst("", ": ");
        QCOMPARE(left,  QString(""));
        QCOMPARE(right, QString(""));
    }

    void test_splitFirst_QString_sepAtStart()
    {
        auto [left, right] = StringUtils::splitFirst(": valor", ": ");
        QCOMPARE(left,  QString(""));
        QCOMPARE(right, QString("valor"));
    }

    void test_splitFirst_QString_sepAtEnd()
    {
        auto [left, right] = StringUtils::splitFirst("valor: ", ": ");
        QCOMPARE(left,  QString("valor"));
        QCOMPARE(right, QString(""));
    }

    void test_splitFirst_QString_multipleSeps_onlyFirst()
    {
        auto [left, right] = StringUtils::splitFirst("a:b:c", ":");
        QCOMPARE(left,  QString("a"));
        QCOMPARE(right, QString("b:c"));
    }

    // --- splitFirst(QString, QChar) ---

    void test_splitFirst_QChar_normal()
    {
        auto [left, right] = StringUtils::splitFirst("hello world", QChar(' '));
        QCOMPARE(left,  QString("hello"));
        QCOMPARE(right, QString("world"));
    }

    void test_splitFirst_QChar_notFound()
    {
        auto [left, right] = StringUtils::splitFirst("helloworld", QChar(' '));
        QCOMPARE(left,  QString("helloworld"));
        QCOMPARE(right, QString(""));
    }

    void test_splitFirst_QChar_emptyString()
    {
        auto [left, right] = StringUtils::splitFirst("", QChar(' '));
        QCOMPARE(left,  QString(""));
        QCOMPARE(right, QString(""));
    }

    void test_splitFirst_QChar_atStart()
    {
        auto [left, right] = StringUtils::splitFirst(" hello", QChar(' '));
        QCOMPARE(left,  QString(""));
        QCOMPARE(right, QString("hello"));
    }

    // --- splitFirst(QString, QRegularExpression) ---

    void test_splitFirst_Regex_normal()
    {
        QRegularExpression re("\\s+");
        auto [left, right] = StringUtils::splitFirst("hello   world", re);
        QCOMPARE(left,  QString("hello"));
        QCOMPARE(right, QString("world"));
    }

    void test_splitFirst_Regex_notFound()
    {
        QRegularExpression re("\\d+");
        auto [left, right] = StringUtils::splitFirst("nonnumeric", re);
        QCOMPARE(left,  QString("nonnumeric"));
        QCOMPARE(right, QString(""));
    }

    void test_splitFirst_Regex_emptyString()
    {
        QRegularExpression re(":");
        auto [left, right] = StringUtils::splitFirst("", re);
        QCOMPARE(left,  QString(""));
        QCOMPARE(right, QString(""));
    }

    // Caso real del updater: parsear línea "Version Name: 1.2.3\n"
    void test_splitFirst_updaterParsePattern()
    {
        QString line = "Version Name: 1.2.3";
        auto [key, val] = StringUtils::splitFirst(line, ": ");
        QCOMPARE(key.trimmed(), QString("Version Name"));
        QCOMPARE(val.trimmed(), QString("1.2.3"));
    }

    // Caso sin separador en el updater — antes producía string incorrecto
    void test_splitFirst_updaterNoSep_returnsSafeEmpty()
    {
        QString line = "línea sin formato esperado";
        auto [key, val] = StringUtils::splitFirst(line, ": ");
        // key = línea completa, val = vacío — no basura
        QCOMPARE(key,  line);
        QCOMPARE(val,  QString(""));
    }
};

QTEST_GUILESS_MAIN(StringUtilsTest)
#include "StringUtils_test.moc"
