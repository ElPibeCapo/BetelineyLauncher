// SPDX-FileCopyrightText: 2026 El_PibeCapo
//
// SPDX-License-Identifier: GPL-3.0-only

#include "BetelineyTokens.h"

#include <QHash>
#include <QStringList>
#include <algorithm>

namespace BetelineyTokens {

static const QHash<QString, QString>& table()
{
    static const QHash<QString, QString> t = {
        { "@bg", kBg },
        { "@surface", kSurface },
        { "@raised", kRaised },
        { "@sunken", kSunken },
        { "@stroke", kStroke },
        { "@strokeDim", kStrokeDim },
        { "@strokeHi", kStrokeHi },
        { "@bright", kBright },
        { "@body", kBody },
        { "@secondary", kSecondary },
        { "@muted", kMuted },
        { "@disabled", kDisabled },
        { "@accent", kAccent },
        { "@accentHi", kAccentHi },
        { "@accentDim", kAccentDim },
        { "@accentBg", kAccentBg },
        { "@cyan", kCyan },
        { "@danger", kDanger },
        { "@warning", kWarning },
    };
    return t;
}

QString expand(QString css)
{
    // Orden descendente por longitud: sin esto, "@accent" pisaría el prefijo
    // de "@accentDim" y dejaría el sufijo "Dim" suelto en el QSS.
    static const QStringList keys = [] {
        QStringList k = table().keys();
        std::sort(k.begin(), k.end(), [](const QString& a, const QString& b) { return a.size() > b.size(); });
        return k;
    }();

    for (const auto& k : keys)
        css.replace(k, table().value(k));
    return css;
}

}  // namespace BetelineyTokens
