// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#include "BetelineyTelemetry.h"

#include "Application.h"
#include "BaseInstance.h"
#include "BuildConfig.h"
#include "InstanceList.h"
#include "settings/SettingsObject.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSysInfo>
#include <QTimer>
#include <QUuid>

namespace Beteliney {

namespace {
constexpr const char* kEnabledSetting = "TelemetryEnabled";
constexpr const char* kUuidSetting = "TelemetryUuid";
constexpr const char* kNoticeShownSetting = "TelemetryFirstRunNoticeShown";
constexpr const char* kOpenSecondsSetting = "TelemetryTotalOpenSeconds";
constexpr int kHeartbeatIntervalMs = 4 * 60 * 1000;  // 4 minutos
// TODO(ElPibeCapo): reemplazar por la URL real una vez deployado el Worker.
constexpr const char* kHeartbeatUrl = "https://beteliney-telemetry.REEMPLAZAR.workers.dev/heartbeat";
}  // namespace

Telemetry& Telemetry::instance() {
    static Telemetry inst;
    return inst;
}

Telemetry::Telemetry(QObject* parent) : QObject(parent) {
    APPLICATION->settings()->registerSetting(kEnabledSetting, true);
    APPLICATION->settings()->registerSetting(kUuidSetting, QString());
    APPLICATION->settings()->registerSetting(kNoticeShownSetting, false);
    APPLICATION->settings()->registerSetting(kOpenSecondsSetting, 0);
}

bool Telemetry::isEnabled() const {
    return APPLICATION->settings()->get(kEnabledSetting).toBool();
}

void Telemetry::setEnabled(bool enabled) {
    APPLICATION->settings()->set(kEnabledSetting, enabled);
    if (!enabled && m_timer) {
        m_timer->stop();
    } else if (enabled && m_timer && !m_timer->isActive()) {
        m_lastTick = QDateTime::currentDateTimeUtc();  // evita contar como "abierto" el tiempo que estuvo deshabilitado
        m_timer->start(kHeartbeatIntervalMs);
        QTimer::singleShot(5000, this, &Telemetry::sendHeartbeat);
    }
}

bool Telemetry::shouldShowFirstRunNotice() {
    auto settings = APPLICATION->settings();
    if (settings->get(kNoticeShownSetting).toBool())
        return false;
    settings->set(kNoticeShownSetting, true);
    return true;
}

QString Telemetry::uuid() {
    auto settings = APPLICATION->settings();
    QString id = settings->get(kUuidSetting).toString();
    if (id.isEmpty()) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        settings->set(kUuidSetting, id);
    }
    return id;
}

void Telemetry::start(QNetworkAccessManager* network) {
    m_network = network;
    m_lastTick = QDateTime::currentDateTimeUtc();
    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &Telemetry::sendHeartbeat);
    }
    if (!isEnabled())
        return;
    m_timer->start(kHeartbeatIntervalMs);
    QTimer::singleShot(5000, this, &Telemetry::sendHeartbeat);  // primer heartbeat a los 5s de abrir
}

qint64 Telemetry::aggregatePlaytimeSeconds() const {
    qint64 total = 0;
    auto* list = APPLICATION->instances();
    if (!list)
        return 0;
    for (int i = 0; i < list->count(); ++i) {
        if (auto* inst = list->at(i))
            total += inst->totalTimePlayed();
    }
    return total;
}

qint64 Telemetry::accumulateOpenSeconds() {
    auto settings = APPLICATION->settings();
    QDateTime now = QDateTime::currentDateTimeUtc();
    if (m_lastTick.isValid()) {
        qint64 elapsed = m_lastTick.secsTo(now);
        if (elapsed > 0) {
            qint64 total = settings->get(kOpenSecondsSetting).toLongLong() + elapsed;
            settings->set(kOpenSecondsSetting, total);
        }
    }
    m_lastTick = now;
    return settings->get(kOpenSecondsSetting).toLongLong();
}

void Telemetry::sendHeartbeat() {
    if (!isEnabled() || !m_network)
        return;

    QJsonObject payload;
    payload["uuid"] = uuid();
    payload["version"] = QStringLiteral("%1.%2.%3")
                              .arg(BuildConfig.VERSION_MAJOR)
                              .arg(BuildConfig.VERSION_MINOR)
                              .arg(BuildConfig.VERSION_PATCH);
    payload["os"] = QSysInfo::prettyProductName();
    // Dos metricas de tiempo distintas, a proposito:
    //  - playtime_total_seconds: tiempo jugado DENTRO de Minecraft (suma de todas las instancias).
    //  - open_total_seconds: tiempo que el LAUNCHER ha estado abierto (no implica estar jugando).
    payload["playtime_total_seconds"] = aggregatePlaytimeSeconds();
    payload["open_total_seconds"] = accumulateOpenSeconds();

    QNetworkRequest req{ QUrl(QString::fromLatin1(kHeartbeatUrl)) };
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    auto* reply = m_network->post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    // Fire-and-forget: nunca le mostramos nada al usuario, ni exito ni error.
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

}  // namespace Beteliney
