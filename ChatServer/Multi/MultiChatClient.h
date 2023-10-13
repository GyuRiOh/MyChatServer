#pragma once
#include "../../NetRoot/LanClient/LanClient.h"
#include "../../MonitorProtocol.h"

namespace server_baby
{
	class MultiChatClient : public LanClient
	{
	protected:
		bool OnRecv(LanPacketSet* packetList)
		{
			LanPacketSet::Free(packetList);
			return true;
		}

		void OnConnect()
		{
			LanPacket* packet = LanPacket::Alloc();

			*packet << (WORD)en_PACKET_SS_MONITOR_LOGIN;
			*packet << (int)eCHAT_SERVER_NO;

			SendPacket(packet);
			LanPacket::Free(packet);

		}
	};
}
