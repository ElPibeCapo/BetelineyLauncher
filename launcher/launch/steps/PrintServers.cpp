// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
//  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 *  Copyright (c) 2024 Leia uwu <leia@tutamail.com>
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

#include "PrintServers.h"
#include <QHostInfo>
#include <QTimer>

// Tiempo máximo de espera por DNS lookup antes de reportar el servidor como no resuelto
static constexpr int DNS_TIMEOUT_MS = 10000;

PrintServers::PrintServers(LaunchTask* parent, const QStringList& servers) : LaunchStep(parent)
{
    m_servers = servers;
}

void PrintServers::executeTask()
{
    if (m_servers.isEmpty()) {
        emitSucceeded();
        return;
    }

    // Deduplicar: evita lookups redundantes si el mismo server aparece varias veces
    QStringList uniqueServers = m_servers;
    uniqueServers.removeDuplicates();

    for (const QString& server : uniqueServers) {
        int id = QHostInfo::lookupHost(server, this, &PrintServers::resolveServer);
        m_pendingLookups.insert(id, server);
    }

    // Timeout global: si algún lookup no responde en DNS_TIMEOUT_MS, completamos igual
    auto* timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, timer]() {
        timer->deleteLater();
        if (!m_pendingLookups.isEmpty()) {
            // Reportar los servidores pendientes como no resueltos
            for (const QString& server : m_pendingLookups.values()) {
                QString msg = server + " resolves to:\n    [timeout - DNS no respondió]\n\n";
                m_server_to_address.insert(server, msg);
                emit logLine(msg, MessageLevel::Warning);
            }
            // Cancelar lookups pendientes
            for (int id : m_pendingLookups.keys())
                QHostInfo::abortHostLookup(id);
            m_pendingLookups.clear();
            emitSucceeded();
        }
    });
    timer->start(DNS_TIMEOUT_MS);
}

void PrintServers::resolveServer(const QHostInfo& host_info)
{
    m_pendingLookups.remove(host_info.lookupId());

    QString server = host_info.hostName();
    QString addresses = server + " resolves to:\n    [";

    if (!host_info.addresses().isEmpty()) {
        QStringList addrStrings;
        for (const QHostAddress& address : host_info.addresses())
            addrStrings << address.toString();
        addresses += addrStrings.join(", ");
    } else {
        addresses += "N/A";
    }
    addresses += "]\n\n";

    m_server_to_address.insert(server, addresses);

    if (m_pendingLookups.isEmpty()) {
        // Imprimir en el orden original de m_servers (deduplicado)
        QStringList seen;
        for (const QString& serv : m_servers) {
            if (!seen.contains(serv)) {
                seen << serv;
                emit logLine(m_server_to_address.value(serv), MessageLevel::Launcher);
            }
        }
        emitSucceeded();
    }
}

bool PrintServers::abort()
{
    for (int id : m_pendingLookups.keys())
        QHostInfo::abortHostLookup(id);
    m_pendingLookups.clear();
    emitAborted();
    return true;
}

bool PrintServers::canAbort() const
{
    return true;
}
