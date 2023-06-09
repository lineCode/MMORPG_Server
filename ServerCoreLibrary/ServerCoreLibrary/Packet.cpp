#include "pch.h"
#include "Packet.h"

void Packet::WriteData() {
    const int dataSize = 0;
    const int packetSize = dataSize + sizeof(PacketHeader);

    send_buffer = MakeShared<SendBuffer>(packetSize);
    PacketHeader* header = reinterpret_cast<PacketHeader*>(send_buffer->Buffer());

    header->size = packetSize;
    header->groupId = groupId;
    header->id = packetId;

    send_buffer->Close(packetSize);
}
