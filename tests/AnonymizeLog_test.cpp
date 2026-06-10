// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Tests para AnonymizeLog
 */

#include <QTest>
#include "logs/AnonymizeLog.h"

class AnonymizeLogTest : public QObject {
    Q_OBJECT

   private slots:

    // Rutas de usuario Windows deben anonimizarse
    void test_windowsUserPath()
    {
        QString log = "Loading from C:\\Users\\juanperez\\AppData\\Roaming\\beteliney";
        anonymizeLog(log);
        QVERIFY2(!log.contains("juanperez"), "Username not anonymized in Windows path");
        QVERIFY2(log.contains("********"), "Expected mask not found");
    }

    // Rutas con forward slash en Windows
    void test_windowsForwardSlash()
    {
        QString log = "Path: C:/Users/juanperez/something";
        anonymizeLog(log);
        QVERIFY2(!log.contains("juanperez"), "Username not anonymized in Windows forward slash path");
    }

    // Rutas Linux
    void test_linuxHomePath()
    {
        QString log = "Loading /home/juanperez/.minecraft/logs/latest.log";
        anonymizeLog(log);
        QVERIFY2(!log.contains("juanperez"), "Username not anonymized in Linux path");
    }

    // Token de sesión
    void test_sessionToken()
    {
        QString log = "Auth response (Session ID is abc123xyz789TOKEN)";
        anonymizeLog(log);
        QVERIFY2(!log.contains("abc123xyz789TOKEN"), "Session token not anonymized");
        QVERIFY2(log.contains("<SESSION_TOKEN>"), "Expected SESSION_TOKEN placeholder missing");
    }

    // Access token
    void test_accessToken()
    {
        QString log = R"({"accessToken": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9abcdefghijklmnopqrst"})";
        anonymizeLog(log);
        QVERIFY2(log.contains("<ACCESS_TOKEN>"), "Access token not anonymized");
    }

    // Username en args de launch
    void test_launchUsername()
    {
        QString log = "Launching with --username JuanPerez --version 1.21";
        anonymizeLog(log);
        QVERIFY2(!log.contains("JuanPerez"), "Username in --username not anonymized");
        QVERIFY2(log.contains("<USERNAME>"), "Expected USERNAME placeholder missing");
        // --version debe quedar intacto
        QVERIFY2(log.contains("--version 1.21"), "--version arg incorrectly stripped");
    }

    // UUID genérico (mod, mundo) NO debe eliminarse
    void test_genericUUIDPreserved()
    {
        QString worldUUID = "550e8400-e29b-41d4-a716-446655440000";
        QString log = "Loading world with seed id=" + worldUUID;
        anonymizeLog(log);
        QVERIFY2(log.contains(worldUUID),
                 "Generic world UUID was incorrectly stripped (breaks debugging)");
    }

    // UUID de jugador SÍ debe eliminarse (aparece como "uuid: ...")
    void test_playerUUIDAnonymized()
    {
        QString log = R"({"uuid": "550e8400-e29b-41d4-a716-446655440000", "name": "Player"})";
        anonymizeLog(log);
        QVERIFY2(!log.contains("550e8400-e29b-41d4-a716-446655440000"),
                 "Player UUID was not anonymized");
        QVERIFY2(log.contains("<UUID>"), "Expected UUID placeholder missing");
    }

    // Log limpio no debe cambiar
    void test_cleanLogUnchanged()
    {
        QString log = "[Client thread/INFO]: Loading Minecraft 1.21.5";
        QString original = log;
        anonymizeLog(log);
        QCOMPARE(log, original);
    }

    // Log vacío no explota
    void test_emptyLog()
    {
        QString log = "";
        anonymizeLog(log);
        QCOMPARE(log, QString(""));
    }

    // AppData subfolder se anonimiza (Roaming, Local, etc.)
    void test_appDataSubdirAnonymized()
    {
        QString log = "Config at C:\\Users\\user\\AppData\\Roaming\\beteliney\\config.ini";
        anonymizeLog(log);
        QVERIFY2(!log.contains("Roaming"), "AppData subdir not anonymized");
        QVERIFY2(log.contains("<SUBDIR>"), "Expected SUBDIR placeholder missing");
    }

    // Bearer token en cabecera HTTP
    void test_bearerToken()
    {
        QString log = "Authorization: Bearer eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJhYmMxMjMifQ.signatureXYZABC";
        anonymizeLog(log);
        QVERIFY2(!log.contains("eyJhbGciOiJSUzI1NiJ9"), "Bearer token not anonymized");
        QVERIFY2(log.contains("<BEARER_TOKEN>"), "Expected BEARER_TOKEN placeholder missing");
    }

    // client_secret nunca debe aparecer en logs
    void test_clientSecretAnonymized()
    {
        QString log = "client_secret=supersecretvalue123abc";
        anonymizeLog(log);
        QVERIFY2(!log.contains("supersecretvalue123abc"), "client_secret value not anonymized");
        QVERIFY2(log.contains("<CLIENT_SECRET>"), "Expected CLIENT_SECRET placeholder missing");
    }

    // IP privada 192.168.x.x debe anonimizarse
    void test_privateIP_192()
    {
        QString log = "Connecting to server at 192.168.1.105:25565";
        anonymizeLog(log);
        QVERIFY2(!log.contains("192.168.1.105"), "Private IP 192.168.x.x not anonymized");
        QVERIFY2(log.contains("<LOCAL_IP>"), "Expected LOCAL_IP placeholder missing");
    }

    // IP privada 10.x.x.x debe anonimizarse
    void test_privateIP_10()
    {
        QString log = "Server address: 10.0.0.5";
        anonymizeLog(log);
        QVERIFY2(!log.contains("10.0.0.5"), "Private IP 10.x.x.x not anonymized");
        QVERIFY2(log.contains("<LOCAL_IP>"), "Expected LOCAL_IP placeholder missing");
    }

    // IP privada 172.16–31.x.x debe anonimizarse
    void test_privateIP_172()
    {
        QString log = "Connecting to 172.20.10.3";
        anonymizeLog(log);
        QVERIFY2(!log.contains("172.20.10.3"), "Private IP 172.16-31.x.x not anonymized");
        QVERIFY2(log.contains("<LOCAL_IP>"), "Expected LOCAL_IP placeholder missing");
    }

    // IP pública NO debe anonimizarse (127.x y 8.8.8.8 son públicas o loopback)
    void test_publicIPPreserved()
    {
        QString log = "DNS resolved to 8.8.8.8";
        QString original = log;
        anonymizeLog(log);
        QCOMPARE(log, original);
    }

    // --accessToken en args de launch
    void test_launchAccessToken()
    {
        QString log = "Launching with --accessToken eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9longtoken --version 1.21";
        anonymizeLog(log);
        QVERIFY2(!log.contains("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9longtoken"), "--accessToken value not anonymized");
        QVERIFY2(log.contains("<ACCESS_TOKEN>"), "Expected ACCESS_TOKEN placeholder missing");
        QVERIFY2(log.contains("--version 1.21"), "--version arg incorrectly stripped");
    }

    // Email debe anonimizarse
    void test_emailAnonymized()
    {
        QString log = "Account login error for user@example.com";
        anonymizeLog(log);
        QVERIFY2(!log.contains("user@example.com"), "Email not anonymized");
        QVERIFY2(log.contains("<EMAIL>"), "Expected EMAIL placeholder missing");
    }

    // Refresh token debe anonimizarse
    void test_refreshToken()
    {
        QString log = "Storing new refresh token: \"M.R3_BL2-some_very_long_refresh_token_value_here\"";
        anonymizeLog(log);
        QVERIFY2(!log.contains("M.R3_BL2"), "Refresh token not anonymized");
        QVERIFY2(log.contains("<TOKEN>"), "Expected TOKEN placeholder missing");
    }

    // Device code debe anonimizarse
    void test_deviceCode()
    {
        QString log = R"({"device_code" :  "GMMhmXSbSaOeEDqD4jHmjkpnrxyz"})";
        anonymizeLog(log);
        QVERIFY2(!log.contains("GMMhmXSbSaOeEDqD4jHmjkpnrxyz"), "Device code not anonymized");
        QVERIFY2(log.contains("<DEVICE_CODE>"), "Expected DEVICE_CODE placeholder missing");
    }

    // Múltiples datos sensibles en el mismo log — todos se enmascaran
    void test_multipleSecretsInOneLine()
    {
        QString log = "User user@example.com launched with --username PlayerName from 192.168.0.10";
        anonymizeLog(log);
        QVERIFY2(!log.contains("user@example.com"), "Email in multi-secret line not anonymized");
        QVERIFY2(!log.contains("PlayerName"), "Username in multi-secret line not anonymized");
        QVERIFY2(!log.contains("192.168.0.10"), "Local IP in multi-secret line not anonymized");
    }
};

QTEST_GUILESS_MAIN(AnonymizeLogTest)
#include "AnonymizeLog_test.moc"
