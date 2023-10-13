#pragma once
#include "NetRoot/NetServer/NetPacket.h"
#include "ChatDummyClient.h"

namespace server_baby
{
	class ChatDummyClient_CS_Proxy
	{
	public:
		explicit ChatDummyClient_CS_Proxy(ChatDummyClient* client) : client_(client) {}

		void ReqLogin(INT64 accountNo, WCHAR* accountID, WCHAR* nickName, char* sessionKey, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)0;
			*msg << accountNo;
			msg->EnqData((char*)accountID, 40);
			msg->EnqData((char*)nickName, 40);
			msg->EnqData((char*)sessionKey, 64);

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

		void ReqSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)1;
			*msg << accountNo;
			*msg << sectorX;
			*msg << sectorY;

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

		void ReqMessage(INT64 accountNo, WORD messageLen, WCHAR* message, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)2;
			*msg << accountNo;
			*msg << messageLen;
			msg->EnqData((char*)message, messageLen);

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

	private:
		ChatDummyClient* client_;
	};
}
