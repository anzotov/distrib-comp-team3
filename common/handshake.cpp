#include "handshake.h"

QJsonObject Handshake::serialize() const
{
    auto object = QJsonObject();
    object.insert("class", "Handshake");
    object.insert("peerType", static_cast<int>(peerType));
    object.insert("compPower", compPower);
    object.insert("uuid", uuid);
    return object;
}

bool Handshake::deserialize(const QJsonObject &input, std::function<void(Handshake *)> handler)
{
    if (input["class"].toString() != "Handshake" || !input["peerType"].isDouble() || !input["compPower"].isString() || !input["uuid"].isString())
        return false;
    auto task = new Handshake(
        PeerType{input["peerType"].toInt()},
        input["compPower"].toString(),
        input["uuid"].toString());
    handler(task);
    return true;
}
