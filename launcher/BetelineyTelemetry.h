// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QTimer;

namespace Beteliney {

/**
 * Telemetria anonima de uso (Beteliney Fase 4, sesion post-hardening).
 *
 * Manda un heartbeat periodico a un endpoint propio (Cloudflare Worker)
 * con un UUID local generado en el primer arranque, la version del
 * launcher y el SO. Nunca manda IP explicita, nombre de usuario ni
 * ningun dato identificable de la cuenta de Minecraft.
 *
 * Con eso el backend calcula dos metricas publicas: "usuarios activos
 * ahora" (heartbeats recientes, ventana de 5 min) e "instalaciones
 * historicas" (UUIDs distintos vistos alguna vez) — esta ultima pensada
 * para celebrar hitos con la comunidad.
 *
 * Activado por defecto, con aviso una sola vez en el primer arranque;
 * el usuario lo puede desactivar en Configuracion > Launcher.
 */
class Telemetry : public QObject {
    Q_OBJECT
   public:
    static Telemetry& instance();

    /** Arranca el timer de heartbeat si esta habilitado. Llamar una vez al iniciar la app. */
    void start(QNetworkAccessManager* network);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    /** true solo la primera vez que se llama tras instalar/actualizar; marca el aviso como ya mostrado. */
    bool shouldShowFirstRunNotice();

   private slots:
    void sendHeartbeat();

   private:
    explicit Telemetry(QObject* parent = nullptr);
    QString uuid();
    /** Suma totalTimePlayed() de todas las instancias: tiempo jugado en Minecraft, no en el launcher. */
    qint64 aggregatePlaytimeSeconds() const;
    /** Acumula en un setting persistente cuanto tiempo ha estado abierto el launcher mismo (no Minecraft). */
    qint64 accumulateOpenSeconds();

    QNetworkAccessManager* m_network = nullptr;
    QTimer* m_timer = nullptr;
    QDateTime m_lastTick;
};

}  // namespace Beteliney
