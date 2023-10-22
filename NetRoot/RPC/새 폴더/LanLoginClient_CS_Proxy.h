#pragma once
#include "NetRoot/LanServer/LanPacket.h"
#include "LanLoginClient.h"

namespace server_baby
{
	class LanLoginClient_CS_Proxy
	{
	public:
		explicit LanLoginClient_CS_Proxy(LanLoginClient* client) : client_(client) {}

		void ReqLoginLogin(INT64 accountNo, char* sessionKey, LanSessionID sessionID)
		{
			LanPacket* msg = LanPacket::Alloc();

			*msg << (unsigned short)0;
			*msg << accountNo;
			msg->EnqData((char*)sessionKey, 64);

			client_->SendPacket(msg);
			LanPacket::Free(msg);
		}

	private:
		LanLoginClient* client_;
	};
}
