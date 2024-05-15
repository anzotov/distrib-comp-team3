#pragma once

#include "discoveryData.h"
#include <QObject>

class DiscoveryServiceBase : public QObject
{
    Q_OBJECT

public:
    DiscoveryServiceBase(QObject *parent = nullptr) : QObject(parent) {}
    
    virtual void startMulticast(const DiscoveryData &discoveryData) = 0;
    virtual void startListening() = 0;
    virtual void stopListening() = 0;
    virtual void stopMulticast() = 0;
    virtual void clearPeers() = 0;
    virtual QList<DiscoveryData> peers() const = 0;

signals:
    void peersChanged(const QList<DiscoveryData> peers);
};