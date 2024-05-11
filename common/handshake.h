#include <QString>
#include <functional>
#include <QJsonObject>

struct Handshake final
{
    enum class PeerType
    {
        CompNode,
        TaskNode
    };

    Handshake() = default;
    Handshake(const PeerType peerType, const QString &compPower, const QString &uuid)
        : peerType(peerType), compPower(compPower), uuid(uuid){};
    PeerType peerType;
    QString compPower;
    QString uuid;

    QJsonObject serialize() const;
    static bool deserialize(const QJsonObject &input, std::function<void(Handshake *)> handler);
    QString toQString() const
    {
        return QStringLiteral("Handshake(%1, %2, %3)").arg(static_cast<int>(peerType)).arg(compPower).arg(uuid);
    }
    bool operator==(const Handshake &other) const
    {
        return this == &other || (peerType == other.peerType && compPower == other.compPower && uuid == other.uuid);
    }
    bool operator!=(const Handshake &other) const
    {
        return !(*this == other);
    }
};