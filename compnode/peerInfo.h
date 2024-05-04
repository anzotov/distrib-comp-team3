#pragma once

#include <QString>

using PeerHandlerType = QString;

struct PeerInfo
{
    PeerHandlerType peerHandler;
    QString compPower;
    QString connectionSpeed;
};
