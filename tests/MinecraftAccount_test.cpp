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

// Sesion 54: minecraft/auth/MinecraftAccount.cpp nunca tuvo test propio. Cubre
// solo la logica pura: creacion de cuentas (createOffline/createBlankMSA), el
// algoritmo de UUID offline (reimplementacion de Java UUID#nameUUIDFromBytes),
// el wrapper de (de)serializacion V3, y shouldRefresh()/fillSession(), que son
// puro calculo sobre AccountData sin tocar red ni Task. login()/refresh() (que
// si arrancan un AuthFlow real) quedan fuera a proposito -- eso requiere
// mockear la red, no es responsabilidad de esta clase.

#include <QTest>

#include <memory>

#include "Usable.h"
#include "minecraft/auth/AuthSession.h"
#include "minecraft/auth/MinecraftAccount.h"

class MinecraftAccountTest : public QObject {
    Q_OBJECT

   private slots:
    // createOffline() debe dejar la cuenta lista para jugar sin ningun paso
    // de red: token "0", perfil con id/nombre resueltos, y todo en
    // Validity::Certain (no hay nada que verificar contra un server).
    void test_createOfflineSetsExpectedDefaults()
    {
        auto account = MinecraftAccount::createOffline("Steve");

        QCOMPARE(account->accountType(), AccountType::Offline);
        QCOMPARE(account->accessToken(), QString("0"));
        QCOMPARE(account->profileName(), QString("Steve"));
        QVERIFY(account->hasProfile());
        QCOMPARE(account->typeString(), QString("nopremium"));
        QVERIFY(!account->ownsMinecraft());
    }

    // El profileId de una cuenta offline no es arbitrario: tiene que
    // coincidir exactamente con el UUID que el cliente vanilla de Minecraft
    // calcularia para el mismo username (mismo algoritmo, mismo input). Si
    // esto se desincroniza, mundos/stats guardados bajo el UUID vanilla
    // dejan de encontrarse.
    void test_createOfflineProfileIdMatchesUuidFromUsername()
    {
        auto account = MinecraftAccount::createOffline("Steve");
        auto expectedUuid = MinecraftAccount::uuidFromUsername("Steve").toString(QUuid::Id128);
        QCOMPARE(account->profileId(), expectedUuid);
    }

    // Cada cuenta debe tener su propio internalId (clave interna que usa
    // AccountList) -- si dos createOffline() sucesivos generaran el mismo
    // internalId, AccountList trataria dos cuentas distintas como una sola.
    void test_createOfflineGeneratesUniqueInternalIdPerAccount()
    {
        auto first = MinecraftAccount::createOffline("Steve");
        auto second = MinecraftAccount::createOffline("Steve");
        QVERIFY(!first->internalId().isEmpty());
        QVERIFY(!second->internalId().isEmpty());
        QVERIFY(first->internalId() != second->internalId());
    }

    // createBlankMSA() es el punto de partida antes de loguear: tipo MSA
    // fijado, pero sin ningun token ni perfil todavia -- si trajera algo
    // precargado, AuthFlow arrancaria con datos stale.
    void test_createBlankMSASetsTypeAndNoTokens()
    {
        auto account = MinecraftAccount::createBlankMSA();
        QCOMPARE(account->accountType(), AccountType::MSA);
        QCOMPARE(account->typeString(), QString("premium"));
        QVERIFY(!account->hasProfile());
        QCOMPARE(account->accessToken(), QString());
    }

    // uuidFromUsername() reimplementa Java UUID#nameUUIDFromBytes sobre
    // "OfflinePlayer:<username>" -- estos valores se calcularon
    // independientemente en Python (hashlib.md5 + los mismos bitmasks de
    // version/variant), no copiados de la implementacion bajo test, para que
    // el test pueda detectar un bug real y no solo confirmar lo que el
    // codigo ya hace.
    void test_uuidFromUsernameMatchesJavaOfflineUUIDAlgorithm()
    {
        QCOMPARE(MinecraftAccount::uuidFromUsername("Steve"), QUuid("5627dd98-e6be-3c21-b8a8-e92344183641"));
        QCOMPARE(MinecraftAccount::uuidFromUsername("ElPibeCapo"), QUuid("7617a150-e98b-3a43-857a-b9e476869a2e"));
        QCOMPARE(MinecraftAccount::uuidFromUsername("Alex"), QUuid("36532b5e-c442-3dbb-a24c-c7e55d0f979a"));
    }

    // saveToJson()/loadFromJsonV3() es el ciclo real que usa AccountList al
    // persistir accounts.json -- ya cubierto a nivel de AccountData en
    // AccountData_test.cpp, esto confirma que el wrapper de MinecraftAccount
    // (que crea el QObject y delega en AccountData) no pierde nada en el
    // camino.
    void test_saveAndLoadJsonV3RoundTripOffline()
    {
        auto original = MinecraftAccount::createOffline("ElPibeCapo");
        auto json = original->saveToJson();

        auto restored = MinecraftAccount::loadFromJsonV3(json);
        QVERIFY(restored != nullptr);
        QCOMPARE(restored->accountType(), AccountType::Offline);
        QCOMPARE(restored->profileName(), QString("ElPibeCapo"));
        QCOMPARE(restored->profileId(), original->profileId());
        QCOMPARE(restored->accessToken(), QString("0"));
    }

    // loadFromJsonV3() debe devolver nullptr limpio ante un JSON invalido o
    // con "type" no reconocido -- AccountList itera la lista de cuentas
    // cargadas y un puntero nulo aca es la señal para saltear esa entrada
    // sin crashear.
    void test_loadFromJsonV3ReturnsNullOnInvalidType()
    {
        QVERIFY(MinecraftAccount::loadFromJsonV3(QJsonObject()) == nullptr);

        QJsonObject badType;
        badType["type"] = "NotARealType";
        QVERIFY(MinecraftAccount::loadFromJsonV3(badType) == nullptr);
    }

    // ownsMinecraft() tiene una regla de negocio explicita: las cuentas
    // offline NUNCA son dueñas del juego, sin importar lo que diga el
    // entitlement -- esto protege contra el caso (deberia ser imposible, pero
    // "deberia" no es garantia) de que un dato corrupto o mal migrado deje
    // ownsMinecraft=true en una cuenta que en realidad es offline.
    void test_ownsMinecraftIsAlwaysFalseForOfflineAccounts()
    {
        auto account = MinecraftAccount::createOffline("Steve");
        account->accountData()->minecraftEntitlement.ownsMinecraft = true;
        account->accountData()->minecraftEntitlement.canPlayMinecraft = true;
        QVERIFY(!account->ownsMinecraft());
    }

    // shouldRefresh() nunca debe pedir refresh de una cuenta que el juego
    // esta usando activamente ahora mismo -- refrescar a mitad de partida
    // rompe la sesion. Esto tiene prioridad sobre cualquier otra condicion,
    // incluida Validity::Assumed que de otro modo diria que si.
    void test_shouldRefreshFalseWhenInUse()
    {
        auto account = MinecraftAccount::createOffline("Steve");
        account->accountData()->validity_ = Validity::Assumed;
        UseLock lock(account.get());
        QVERIFY(!account->shouldRefresh());
    }

    // Validity::None es una cuenta rota (token invalidado por un fallo
    // hard); no tiene sentido intentar refrescarla sola, el usuario tiene
    // que resolverlo relogueando.
    void test_shouldRefreshFalseWhenValidityNone()
    {
        auto account = MinecraftAccount::createOffline("Steve");
        account->accountData()->validity_ = Validity::None;
        QVERIFY(!account->shouldRefresh());
    }

    // Validity::Assumed = "todavia no se confirmo contra el server esta
    // sesion" -- siempre debe refrescarse para pasar a Certain o None.
    void test_shouldRefreshTrueWhenValidityAssumed()
    {
        auto account = MinecraftAccount::createOffline("Steve");
        account->accountData()->validity_ = Validity::Assumed;
        QVERIFY(account->shouldRefresh());
    }

    // Con Validity::Certain y notAfter todavia lejos (mas de 12hs), no hace
    // falta refrescar -- el token fresco de 24hs recien esta empezando.
    void test_shouldRefreshFalseWhenCertainAndFarFromExpiry()
    {
        auto account = MinecraftAccount::createOffline("Steve");
        auto* data = account->accountData();
        data->validity_ = Validity::Certain;
        data->yggdrasilToken.issueInstant = QDateTime::currentDateTimeUtc();
        data->yggdrasilToken.notAfter = QDateTime::currentDateTimeUtc().addSecs(20 * 3600);
        QVERIFY(!account->shouldRefresh());
    }

    // Con Validity::Certain y notAfter a menos de 12hs, hay que refrescar
    // antes de que expire en medio de una sesion de juego larga.
    void test_shouldRefreshTrueWhenCertainAndWithin12HoursOfExpiry()
    {
        auto account = MinecraftAccount::createOffline("Steve");
        auto* data = account->accountData();
        data->validity_ = Validity::Certain;
        data->yggdrasilToken.issueInstant = QDateTime::currentDateTimeUtc().addSecs(-15 * 3600);
        data->yggdrasilToken.notAfter = QDateTime::currentDateTimeUtc().addSecs(3 * 3600);
        QVERIFY(account->shouldRefresh());
    }

    // Si notAfter no es valido (nunca se seteo, ej. cuenta vieja migrada),
    // shouldRefresh() cae a asumir 24hs de vida desde issueInstant. Este test
    // fija ese fallback: token emitido hace 13hs (vencimiento asumido en
    // 11hs, menos de 12) debe pedir refresh igual, sin confiar ciegamente en
    // un notAfter ausente.
    void test_shouldRefreshFallsBackTo24hFromIssueInstantWhenNotAfterInvalid()
    {
        auto account = MinecraftAccount::createOffline("Steve");
        auto* data = account->accountData();
        data->validity_ = Validity::Certain;
        data->yggdrasilToken.issueInstant = QDateTime::currentDateTimeUtc().addSecs(-13 * 3600);
        data->yggdrasilToken.notAfter = QDateTime();  // invalido a proposito
        QVERIFY(account->shouldRefresh());
    }

    // fillSession() es lo ultimo que corre antes de lanzar el juego: arma el
    // AuthSession que Minecraft.jar recibe como argumentos de lanzamiento. Si
    // esto falla o llena mal un campo, el juego arranca con una sesion rota.
    void test_fillSessionPopulatesFieldsFromAccountData()
    {
        auto account = MinecraftAccount::createOffline("Steve");
        auto session = std::make_shared<AuthSession>();
        account->fillSession(session);

        QCOMPARE(session->access_token, QString("0"));
        QCOMPARE(session->player_name, QString("Steve"));
        QCOMPARE(session->uuid, account->profileId());
        QCOMPARE(session->user_type, QString("nopremium"));
        QCOMPARE(session->session, QString("token:0:") + account->profileId());
    }

    // Si por algun motivo el profileId quedo vacio (perfil nunca resuelto),
    // fillSession() no debe lanzar el juego con uuid="" -- cae a calcular el
    // UUID offline a partir del nombre, igual que hace el cliente vanilla
    // cuando no hay perfil online.
    void test_fillSessionFallsBackToUuidFromUsernameWhenProfileIdEmpty()
    {
        auto account = MinecraftAccount::createBlankMSA();
        account->accountData()->minecraftProfile.name = "Alex";
        // profileId queda vacio a proposito (sin perfil resuelto)

        auto session = std::make_shared<AuthSession>();
        account->fillSession(session);

        QCOMPARE(session->player_name, QString("Alex"));
        QCOMPARE(session->uuid, MinecraftAccount::uuidFromUsername("Alex").toString(QUuid::Id128));
    }

    // Sin access_token (cuenta recien creada, nunca logueada), el campo
    // combinado "session" debe quedar en "-" en vez de un string con token
    // vacio ("token::...") que rompe el parseo de argumentos de lanzamiento.
    void test_fillSessionSessionStringIsDashWhenNoAccessToken()
    {
        auto account = MinecraftAccount::createBlankMSA();
        auto session = std::make_shared<AuthSession>();
        account->fillSession(session);
        QCOMPARE(session->session, QString("-"));
    }
};

QTEST_GUILESS_MAIN(MinecraftAccountTest)

#include "MinecraftAccount_test.moc"
