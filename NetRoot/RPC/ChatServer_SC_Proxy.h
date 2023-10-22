#pragma once
#include "NetRoot/NetServer/NetPacket.h"
#include "NetRoot/NetServer/NetSessionID.h"
#include "ChatServer.h"

namespace server_baby
{
	class ChatServer_SC_Proxy
	{
	public:
		explicit ChatServer_SC_Proxy(ChatServer* server) : server_(server) {}

		void ResLogin(BYTE status, INT64 accountNo, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)0;
			*msg << status;
			*msg << accountNo;

			server_->SendPacket_Async(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ReqSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)1;
			*msg << accountNo;
			*msg << sectorX;
			*msg << sectorY;

			server_->SendPacket_Async(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ReqMessage(INT64 accountNo, WCHAR* accountID, WCHAR* nickName, WORD messageLen, WCHAR* message, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)2;
			*msg << accountNo;
			msg->EnqData((char*)accountID, 40);
			msg->EnqData((char*)nickName, 40);
			*msg << messageLen;
			msg->EnqData((char*)message, messageLen);

			server_->SendPacket_Async(sessionID, msg);
			NetPacket::Free(msg);
		}

	private:
		ChatServer* server_;
	};
}
