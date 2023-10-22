#pragma once
#include <Windows.h>
#include "NetRoot/LanServer/LanSessionID.h"
#include "NetRoot/LanServer/LanPacketSet.h"
#include "NetRoot/Common/RPCBuffer.h"
#include "LanLoginClient_CS_Proxy.h"
#include "LanLoginClient.h"

namespace server_baby
{
	class LanLoginClient_SC_Stub
	{
	public:
		explicit LanLoginClient_SC_Stub(LanLoginClient* client) : client_(client) {}

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
				INT64 accountNum;
				BYTE status;
				USHORT gameServerPort;
				USHORT charServerPort;
				*msg >> accountNum;
				*msg >> status;
				RPCBuffer IDBuf(20*2);
				msg->DeqData((char*)IDBuf.data, 20*2);
				WCHAR* ID = (WCHAR*)IDBuf.Data();
				RPCBuffer nickNameBuf(20*2);
				msg->DeqData((char*)nickNameBuf.data, 20*2);
				WCHAR* nickName = (WCHAR*)nickNameBuf.Data();
				RPCBuffer gameServerIPBuf(16*2);
				msg->DeqData((char*)gameServerIPBuf.data, 16*2);
				WCHAR* gameServerIP = (WCHAR*)gameServerIPBuf.Data();
				*msg >> gameServerPort;
				RPCBuffer chatServerIPBuf(16*2);
				msg->DeqData((char*)chatServerIPBuf.data, 16*2);
				WCHAR* chatServerIP = (WCHAR*)chatServerIPBuf.Data();
				*msg >> charServerPort;
				return ResLoginLogin(accountNum, status, ID, nickName, gameServerIP, gameServerPort, chatServerIP, charServerPort, sessionID);
			}
			}
			return false;
		}

		bool ResLoginLogin(INT64 accountNum, BYTE status, WCHAR* ID, WCHAR* nickName, WCHAR* gameServerIP, USHORT gameServerPort, WCHAR* chatServerIP, USHORT charServerPort, LanSessionID sessionID)
		{
			return false;
		}

	private:
		LanLoginClient* client_;
	};
}
