#pragma once

#include "../common/transportServiceBase.h"

#include <QObject>
#include <QList>

#include <stdexcept>

class PeerService : public TransportServiceBase
{
public:
    PeerService(QObject *parent = nullptr) : TransportServiceBase(parent) {}

public:
    virtual void startListening() = 0;
    virtual void stopListening() = 0;
    virtual void startDiscovery() = 0;
    virtual void stopDiscovery() = 0;
};