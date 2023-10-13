#pragma once
#include "../../NetRoot/NetServer/NetPacket.h"
#include "../../NetRoot/NetServer/NetSessionID.h"
#include "ChatServer.h"
#include "../../CommonProtocol.h"

namespace server_baby
{
	class ChatServer_SC_Proxy
	{
	public:
		explicit ChatServer_SC_Proxy(ChatServer* server) : server_(server) {}

		void ResLogin(BYTE status, INT64 accountNo, NetSessionID sessionID)
		{	
			
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<WORD>(en_PACKET_CS_CHAT_RES_LOGIN);
			*msg << status;
			*msg << accountNo;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<WORD>(en_PACKET_CS_CHAT_RES_SECTOR_MOVE);
			*msg << accountNo;
			*msg << sectorX;
			*msg << sectorY;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResMessage(Player* player, INT64 accountNo, WCHAR* accountID, WCHAR* nickName, WORD messageLen, WCHAR* message, NetSessionID sessionID)
		{

			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<WORD>(en_PACKET_CS_CHAT_RES_MESSAGE);
			*msg << accountNo;
			msg->EnqData((char*)accountID, 40);
			msg->EnqData((char*)nickName, 40);
			*msg << messageLen;
			msg->EnqData((char*)message, messageLen);

			server_->TrySendPacket_SectorAround(player, msg);
			NetPacket::Free(msg);
		}

	private:
		ChatServer* server_;
	};
}
