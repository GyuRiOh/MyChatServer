
#include "LanPacket.h"

using namespace MyNetwork;

MemTLS<LanPacket>* LanPacket::packetPool_ = new MemTLS<LanPacket>(500, 1, eLAN_PACKET_POOL_CODE);

LanPacket::~LanPacket(){}
