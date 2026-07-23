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

// Regresión sesión 53: minecraft/auth/ nunca había tenido test alguno pese a
// manejar credenciales en vivo. AccountList::data() tenía un off-by-one real
// heredado del fork ("index.row() > count()" en vez de ">="), que dejaba
// pasar index.row() == count() y llamaba at(count()), lectura fuera de rango
// sobre m_accounts. Corregido en el commit 611b50894 sin ningún test que lo
// respaldara -- este archivo cierra ese hueco.

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>

#include <minecraft/auth/AccountList.h>
#include <minecraft/auth/MinecraftAccount.h>

// createIndex() es protected en QAbstractItemModel -- correcto, no debería
// ser parte de la API pública del modelo. Pero para probar el guard de
// data() ante un índice fuera de rango hace falta poder construir uno
// (index() público ya filtra esos casos vía hasIndex(), así que no sirve
// para este test puntual). Acceso protegido vía subclase, patrón estándar de
// test, no cambia nada de la clase real.
class TestableAccountList : public AccountList {
   public:
    using AccountList::createIndex;
};

class AccountListTest : public QObject {
    Q_OBJECT

   private slots:
    // Control positivo: rowCount()/data() para índices válidos deben
    // funcionar. Si este falla, cualquier fallo de los de abajo es sospechoso
    // (fixture roto), no evidencia de que el fix esté mal.
    void test_dataValidIndexReturnsValue()
    {
        AccountList list;
        list.addAccount(MinecraftAccount::createOffline("Steve"));

        QCOMPARE(list.count(), 1);
        QCOMPARE(list.rowCount(QModelIndex()), 1);

        auto index = list.index(0, AccountList::ProfileNameColumn);
        QVERIFY(index.isValid());
        QCOMPARE(list.data(index, Qt::DisplayRole).toString(), QString("Steve"));
    }

    // El caso exacto del bug: index.row() == count() (uno más allá del
    // último elemento válido). Antes del fix ("> count()" en vez de
    // ">= count()") esto llamaba at(count()) -- lectura fuera de rango sobre
    // m_accounts -- en vez de devolver QVariant() limpio.
    void test_dataOneAfterLastRowReturnsEmptyNotOOB()
    {
        TestableAccountList list;
        list.addAccount(MinecraftAccount::createOffline("Steve"));
        QCOMPARE(list.count(), 1);

        QModelIndex oneAfterLast = list.createIndex(list.count(), AccountList::ProfileNameColumn);

        QVariant result = list.data(oneAfterLast, Qt::DisplayRole);
        QVERIFY(!result.isValid());
    }

    // Mismo caso pero bien más allá del final, para no depender de que
    // count()==1 sea el único valor que dispara el guard.
    void test_dataWellPastLastRowReturnsEmpty()
    {
        TestableAccountList list;
        list.addAccount(MinecraftAccount::createOffline("Steve"));
        list.addAccount(MinecraftAccount::createOffline("Alex"));
        QCOMPARE(list.count(), 2);

        QModelIndex wellPastLast = list.createIndex(100, AccountList::ProfileNameColumn);
        QVariant result = list.data(wellPastLast, Qt::DisplayRole);
        QVERIFY(!result.isValid());
    }

    // Control negativo del otro extremo: fila -1 explícita también debe
    // devolver vacío.
    void test_dataNegativeRowReturnsEmpty()
    {
        TestableAccountList list;
        list.addAccount(MinecraftAccount::createOffline("Steve"));

        QModelIndex negative = list.createIndex(-1, AccountList::ProfileNameColumn);
        QVariant result = list.data(negative, Qt::DisplayRole);
        QVERIFY(!result.isValid());
    }

    // QAbstractItemModelTester ejercita el modelo con el contrato genérico
    // que Qt espera de cualquier QAbstractItemModel -- si el off-by-one
    // volviera a aparecer bajo otra forma, esto lo agarraría sin necesidad
    // de que alguien escriba el caso específico.
    void test_modelContractHolds()
    {
        AccountList list;
        list.addAccount(MinecraftAccount::createOffline("Steve"));
        list.addAccount(MinecraftAccount::createOffline("Alex"));

        QAbstractItemModelTester tester(&list, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }
};

QTEST_GUILESS_MAIN(AccountListTest)

#include "AccountList_test.moc"
