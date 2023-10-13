#pragma once
#include <Windows.h>
#include "NetRoot/LanServer/LanSessionID.h"
#include "NetRoot/LanServer/LanPacketSet.h"
#include "NetRoot/Common/RPCBuffer.h"
#include "LanLoginServer_SC_Proxy.h"
#include "LanLoginServer.h"

namespace server_baby
{
	class LanLoginServer_CS_Stub
	{
	public:
		explicit LanLoginServer_CS_Stub(LanLoginServer* server) : server_(server) {}

		bool PacketProc(LanPacketSet* msgPack)
		{
			switch (msgPack->GetType())
			{
			case eLAN_RECVED_PACKET_SET:
			{
				while (msgPack->GetSize() > 0)
				{
					LanDummyPacket* packet = nullptr;
					if (msgPack->Dequeue(&packet) == false)
						CrashDump::Crash();

					if (!PacketProc(msgPack->GetSessionID(), packet))
						return false;
				}
				break;
			}
			default:
				CrashDump::Crash();
				break;
			}

			return true;
		}

		bool PacketProc(LanSessionID sessionID, LanDummyPacket* msg)
		{
			WORD type;
			*msg >> type;
			switch (type)
			{
			case 0:
			{
				INT64 accountNo;
				*msg >> accountNo;
				RPCBuffer sessionKeyBuf(64);
				msg->DeqData((char*)sessionKeyBuf.data, 64);
				char* sessionKey = (char*)sessionKeyBuf.Data();
				return ReqLoginLogin(accountNo, sessionKey, sessionID);
			}
			}
			return false;
		}

		bool ReqLoginLogin(INT64 accountNo, char* sessionKey, LanSessionID sessionID)
		{
			return false;
		}

	private:
		LanLoginServer* server_;
	};
}
