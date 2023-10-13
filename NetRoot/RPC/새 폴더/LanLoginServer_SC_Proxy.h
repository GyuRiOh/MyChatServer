#pragma once
#include "NetRoot/LanServer/LanPacket.h"
#include "NetRoot/LanServer/LanSessionID.h"
#include "LanLoginServer.h"

namespace server_baby
{
	class LanLoginServer_SC_Proxy
	{
	public:
		explicit LanLoginServer_SC_Proxy(LanLoginServer* server) : server_(server) {}

		void ResLoginLogin(INT64 accountNum, BYTE status, WCHAR* ID, WCHAR* nickName, WCHAR* gameServerIP, USHORT gameServerPort, WCHAR* chatServerIP, USHORT charServerPort, LanSessionID sessionID)
		{
			LanPacket* msg = LanPacket::Alloc();

			*msg << (unsigned short)0;
			*msg << accountNum;
			*msg << status;
			msg->EnqData((char*)ID, 20*2);
			msg->EnqData((char*)nickName, 20*2);
			msg->EnqData((char*)gameServerIP, 16*2);
			*msg << gameServerPort;
			msg->EnqData((char*)chatServerIP, 16*2);
			*msg << charServerPort;

			server_->SendPacket_Async(sessionID, msg);
			LanPacket::Free(msg);
		}

	private:
		LanLoginServer* server_;
	};
}
