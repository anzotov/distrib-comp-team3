#pragma once

using PeerHandlerType = QString;

struct PeerInfo final
{
    PeerHandlerType peerHandler;
    QString compPower;
    QString connectionSpeed;

    QString toQString() const
    {
        return QStringLiteral("PeerInfo(%1, %2, %3)").arg(peerHandler).arg(compPower).arg(connectionSpeed);
    }

    bool operator==(const PeerInfo &other) const
    {
        return this == &other || (peerHandler == other.peerHandler && compPower == other.compPower && connectionSpeed == other.connectionSpeed);
    }

    bool operator!=(const PeerInfo &other) const
    {
        return !(*this == other);
    }
};
