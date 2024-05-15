#pragma once

#include "../compnode/discoveryServiceBase.h"

#include <functional>

struct DiscoveryServiceMock final : DiscoveryServiceBase
{
    DiscoveryServiceMock(QObject *parent = nullptr) : DiscoveryServiceBase(parent)
    {
        if (m_contructor)
            m_contructor(this);
    }

    ~DiscoveryServiceMock() override final
    {
        if (m_destructor)
            m_destructor(this);
    }
    void startMulticast(const DiscoveryData &discoveryData) override final
    {
        ++m_startMulticastCount;
        if (m_startMulticast)
            m_startMulticast(this, discoveryData);
    }

    void startListening() override final
    {
        ++m_startListeningCount;
        if (m_startListening)
            m_startListening(this);
    }

    void stopListening() override final
    {
        ++m_stopListeningCount;
        if (m_stopListening)
            m_stopListening(this);
    }

    void stopMulticast() override final
    {
        ++m_stopMulticastCount;
        if (m_stopMulticast)
            m_stopMulticast(this);
    }

    void clearPeers() override final
    {
        ++m_clearPeersCount;
        if (m_clearPeers)
            m_clearPeers(this);
    }

    QList<DiscoveryData> peers() const override final
    {
        ++m_peersCount;
        return m_peers(this);
    }

    std::function<void(DiscoveryServiceMock *)> m_contructor;
    std::function<void(DiscoveryServiceMock *)> m_destructor;
    std::function<void(DiscoveryServiceMock *, const DiscoveryData &discoveryData)> m_startMulticast;
    int m_startMulticastCount = 0;
    std::function<void(DiscoveryServiceMock *)> m_startListening;
    int m_startListeningCount = 0;
    std::function<void(DiscoveryServiceMock *)> m_stopListening;
    int m_stopListeningCount = 0;
    std::function<void(DiscoveryServiceMock *)> m_stopMulticast;
    int m_stopMulticastCount = 0;
    std::function<void(DiscoveryServiceMock *)> m_clearPeers;
    int m_clearPeersCount = 0;
    std::function<QList<DiscoveryData>(const DiscoveryServiceMock *)> m_peers;
    mutable int m_peersCount = 0;
};
