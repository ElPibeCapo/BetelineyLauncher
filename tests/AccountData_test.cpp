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

// Sesion 54: minecraft/auth/AccountData.cpp (saveState()/resumeStateFromV3(), las
// funciones libres de (de)serializacion V3 en su anonymous namespace) nunca tuvo
// test propio pese a ser el formato de guardado real de todas las cuentas del
// launcher (MSA y offline) -- si esto se rompe, se pierden sesiones guardadas en
// silencio. Es logica pura (QJsonObject <-> struct), sin red ni Qt event loop,
// asi que se puede probar de punta a punta sin mockear nada.

#include <QTest>

#include <QJsonArray>

#include <minecraft/auth/AccountData.h>

class AccountDataTest : public QObject {
    Q_OBJECT

   private slots:
    // Control positivo: una cuenta MSA con los 3 tokens (msa/utoken/xrp-mc),
    // perfil con piel+capa, y entitlement, sobrevive un round-trip completo
    // sin perder ningun campo.
    void test_msaRoundTripPreservesAllFields()
    {
        AccountData original;
        original.type = AccountType::MSA;
        original.msaClientID = "client-id-123";

        original.msaToken.token = "msa-token-value";
        original.msaToken.refresh_token = "msa-refresh-value";
        original.msaToken.issueInstant = QDateTime::fromSecsSinceEpoch(1700000000, Qt::UTC);
        original.msaToken.notAfter = QDateTime::fromSecsSinceEpoch(1700086400, Qt::UTC);
        original.msaToken.persistent = true;

        original.userToken.token = "user-token-value";
        original.userToken.persistent = true;

        original.mojangservicesToken.token = "xrp-token-value";
        original.mojangservicesToken.persistent = true;

        original.yggdrasilToken.token = "ygg-token-value";
        original.yggdrasilToken.persistent = true;

        original.minecraftProfile.id = "profile-uuid";
        original.minecraftProfile.name = "ElPibeCapo";
        original.minecraftProfile.skin.id = "skin-id";
        original.minecraftProfile.skin.url = "https://textures.minecraft.net/texture/skinurl";
        original.minecraftProfile.skin.variant = "classic";
        original.minecraftProfile.skin.data = QByteArray("fake-png-bytes");

        Cape cape;
        cape.id = "cape-id";
        cape.url = "https://textures.minecraft.net/texture/capeurl";
        cape.alias = "Migrator";
        original.minecraftProfile.capes["cape-id"] = cape;
        original.minecraftProfile.currentCape = "cape-id";

        original.minecraftEntitlement.ownsMinecraft = true;
        original.minecraftEntitlement.canPlayMinecraft = true;
        original.minecraftEntitlement.validity = Validity::Certain;

        auto saved = original.saveState();

        AccountData restored;
        QVERIFY(restored.resumeStateFromV3(saved));

        QCOMPARE(restored.type, AccountType::MSA);
        QCOMPARE(restored.msaClientID, QString("client-id-123"));

        QCOMPARE(restored.msaToken.token, QString("msa-token-value"));
        QCOMPARE(restored.msaToken.refresh_token, QString("msa-refresh-value"));
        QCOMPARE(restored.msaToken.issueInstant.toSecsSinceEpoch(), (qint64)1700000000);
        QCOMPARE(restored.msaToken.notAfter.toSecsSinceEpoch(), (qint64)1700086400);

        QCOMPARE(restored.userToken.token, QString("user-token-value"));
        QCOMPARE(restored.mojangservicesToken.token, QString("xrp-token-value"));
        QCOMPARE(restored.yggdrasilToken.token, QString("ygg-token-value"));

        QCOMPARE(restored.profileId(), QString("profile-uuid"));
        QCOMPARE(restored.profileName(), QString("ElPibeCapo"));
        QCOMPARE(restored.minecraftProfile.skin.id, QString("skin-id"));
        QCOMPARE(restored.minecraftProfile.skin.variant, QString("classic"));
        QCOMPARE(restored.minecraftProfile.skin.data, QByteArray("fake-png-bytes"));
        QVERIFY(restored.minecraftProfile.capes.contains("cape-id"));
        QCOMPARE(restored.minecraftProfile.capes["cape-id"].alias, QString("Migrator"));
        QCOMPARE(restored.minecraftProfile.currentCape, QString("cape-id"));

        QVERIFY(restored.minecraftEntitlement.ownsMinecraft);
        QVERIFY(restored.minecraftEntitlement.canPlayMinecraft);
    }

    // Cuenta offline: no debe llevar ninguno de los 3 tokens MSA (persistent
    // por defecto, pero el tipo no es MSA asi que saveState() ni los toca).
    void test_offlineAccountRoundTrip()
    {
        AccountData original;
        original.type = AccountType::Offline;
        original.yggdrasilToken.token = "0";
        original.yggdrasilToken.persistent = true;
        original.minecraftProfile.id = "offline-uuid";
        original.minecraftProfile.name = "Steve";

        auto saved = original.saveState();
        QVERIFY(!saved.contains("msa"));
        QVERIFY(!saved.contains("utoken"));
        QVERIFY(!saved.contains("xrp-mc"));
        QCOMPARE(saved.value("type").toString(), QString("Offline"));

        AccountData restored;
        QVERIFY(restored.resumeStateFromV3(saved));
        QCOMPARE(restored.type, AccountType::Offline);
        QCOMPARE(restored.profileName(), QString("Steve"));
        QCOMPARE(restored.accessToken(), QString("0"));
    }

    // Un token con persistent=false no debe escribirse en absoluto en el
    // JSON de salida -- es la diferencia real entre "recordame" y "solo esta
    // sesion".
    void test_nonPersistentTokenIsNotSaved()
    {
        AccountData original;
        original.type = AccountType::MSA;
        original.msaToken.token = "should-not-be-saved";
        original.msaToken.persistent = false;

        auto saved = original.saveState();
        QVERIFY(!saved.contains("msa"));
    }

    // Formato viejo (pre-7.2): el token yggdrasil literal "offline" debe
    // migrarse a "0" al cargar. Si esta migracion se rompe, cuentas offline
    // guardadas antes de esa version dejan de poder loguear.
    void test_legacyOfflineTokenMigratesToZero()
    {
        QJsonObject legacy;
        legacy["type"] = "Offline";
        QJsonObject yggToken;
        yggToken["token"] = "offline";
        legacy["ygg"] = yggToken;

        AccountData restored;
        QVERIFY(restored.resumeStateFromV3(legacy));
        QCOMPARE(restored.accessToken(), QString("0"));
    }

    // type ausente o con un valor no reconocido -> resumeStateFromV3() debe
    // fallar limpio (false), no crashear ni dejar el objeto a medio llenar
    // silenciosamente.
    void test_missingOrUnknownTypeFailsCleanly()
    {
        AccountData restored;
        QVERIFY(!restored.resumeStateFromV3(QJsonObject()));

        QJsonObject badType;
        badType["type"] = "NotARealType";
        AccountData restored2;
        QVERIFY(!restored2.resumeStateFromV3(badType));
    }

    // Si el entitlement no viene en el JSON (server viejo, o nunca se
    // consulto) pero el perfil si es valido, resumeStateFromV3() asume
    // ownsMinecraft/canPlayMinecraft=true con Validity::Assumed -- para no
    // bloquear cuentas que ya se sabe que son validas por tener perfil.
    void test_missingEntitlementFallsBackToAssumedFromValidProfile()
    {
        QJsonObject data;
        data["type"] = "MSA";
        QJsonObject profile;
        profile["id"] = "some-id";
        profile["name"] = "SomeName";
        QJsonObject skin;
        skin["id"] = "skin-id";
        skin["url"] = "https://textures.minecraft.net/texture/x";
        skin["variant"] = "classic";
        profile["skin"] = skin;
        profile["capes"] = QJsonArray();
        data["profile"] = profile;
        // sin bloque "entitlement" a proposito

        AccountData restored;
        QVERIFY(restored.resumeStateFromV3(data));
        QVERIFY(restored.minecraftEntitlement.ownsMinecraft);
        QVERIFY(restored.minecraftEntitlement.canPlayMinecraft);
        QCOMPARE(restored.minecraftEntitlement.validity, Validity::Assumed);
    }

    // Si tampoco hay perfil valido, no hay de donde asumir el entitlement --
    // debe quedar en su default (false/false/None), no en un true optimista
    // sin base.
    void test_missingEntitlementAndMissingProfileStaysNone()
    {
        QJsonObject data;
        data["type"] = "MSA";
        // sin "profile" ni "entitlement"

        AccountData restored;
        QVERIFY(restored.resumeStateFromV3(data));
        QVERIFY(!restored.minecraftEntitlement.ownsMinecraft);
        QVERIFY(!restored.minecraftEntitlement.canPlayMinecraft);
    }

    // Perfil con datos de piel en base64 invalido: profileFromJSONV3() no
    // valida el contenido (TODO documentado desde sesion 51,
    // docs/AUDITORIA_MODULOS.md), asi que este test fija el comportamiento
    // actual real -- no crashea, produce un QByteArray vacio o basura, pero
    // el resto del perfil (id/name/skin.url) sigue cargando bien. Si algun
    // dia se agrega validacion real, este test debe actualizarse para
    // reflejar el nuevo comportamiento (rechazo explicito), no borrarse.
    void test_corruptedBase64SkinDataDoesNotCrashOrBlockProfile()
    {
        QJsonObject data;
        data["type"] = "MSA";
        QJsonObject profile;
        profile["id"] = "some-id";
        profile["name"] = "SomeName";
        QJsonObject skin;
        skin["id"] = "skin-id";
        skin["url"] = "https://textures.minecraft.net/texture/x";
        skin["variant"] = "classic";
        skin["data"] = "%%% esto no es base64 valido %%%";
        profile["skin"] = skin;
        profile["capes"] = QJsonArray();
        data["profile"] = profile;

        AccountData restored;
        QVERIFY(restored.resumeStateFromV3(data));
        QCOMPARE(restored.profileId(), QString("some-id"));
        QCOMPARE(restored.profileName(), QString("SomeName"));
        // no assertion sobre el contenido exacto de skin.data -- QByteArray::fromBase64
        // con input invalido no tiene una salida "correcta" que verificar, el punto de
        // este test es que no tira excepcion ni deja profileId/profileName sin llenar.
    }

    // profileName() con perfil vacio debe caer al string localizado de
    // fallback, no a un QString() vacio silencioso -- lo consume la UI
    // directamente (lista de cuentas).
    void test_profileNameFallbackWhenEmpty()
    {
        AccountData data;
        QCOMPARE(data.profileName(), QObject::tr("No Minecraft profile"));
    }

    // Perfil sin el campo "skin" (obligatorio segun profileFromJSONV3) debe
    // devolver un MinecraftProfile() vacio con Validity::None, no un perfil
    // a medio llenar con solo id/name.
    void test_profileMissingMandatorySkinFieldRejectsWholeProfile()
    {
        QJsonObject data;
        data["type"] = "MSA";
        QJsonObject profile;
        profile["id"] = "some-id";
        profile["name"] = "SomeName";
        // sin "skin" a proposito
        profile["capes"] = QJsonArray();
        data["profile"] = profile;

        AccountData restored;
        QVERIFY(restored.resumeStateFromV3(data));
        QCOMPARE(restored.profileId(), QString(""));
        QCOMPARE(restored.minecraftProfile.validity, Validity::None);
    }
};

QTEST_GUILESS_MAIN(AccountDataTest)

#include "AccountData_test.moc"
