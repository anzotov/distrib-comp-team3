#pragma once

#include "../common/transportService.h"

#include <functional>

struct TransportLayerMock final : TransportLayerBase
{
    TransportLayerMock(QObject *parent = nullptr) : TransportLayerBase(parent)
    {
        if (m_contructor)
            m_contructor(this);
    }
    ~TransportLayerMock()
    {
        if (m_destructor)
            m_destructor(this);
    }
    void sendData(const PeerHandlerType &peerHandler, const QByteArray &data) override final
    {
        ++m_sendDataCount;
        if (m_sendData)
            m_sendData(this, peerHandler, data);
    }
    void openConnection(const QString &peerInfo) override final
    {
        ++m_openConnectionCount;
        if (m_openConnection)
            m_openConnection(this, peerInfo);
    }
    void closeConnection(const PeerHandlerType &peerHandler) override final
    {
        ++m_closeConnectionCount;
        if (m_closeConnection)
            m_closeConnection(this, peerHandler);
    }
    void startListening()
    {
        ++m_startListeningCount;
        if (m_startListening)
            m_startListening(this);
    }
    void stopListening()
    {
        ++m_stopListeningCount;
        if (m_stopListening)
            m_stopListening(this);
    }

    std::function<void(TransportLayerMock *)> m_contructor;
    std::function<void(TransportLayerMock *)> m_destructor;
    std::function<void(TransportLayerMock *, const PeerHandlerType &, const QByteArray &)> m_sendData;
    int m_sendDataCount = 0;
    std::function<void(TransportLayerMock *, const QString &)> m_openConnection;
    int m_openConnectionCount = 0;
    std::function<void(TransportLayerMock *, const PeerHandlerType &)> m_closeConnection;
    int m_closeConnectionCount = 0;
    std::function<void(TransportLayerMock *)> m_startListening;
    int m_startListeningCount = 0;
    std::function<void(TransportLayerMock *)> m_stopListening;
    int m_stopListeningCount = 0;
};
