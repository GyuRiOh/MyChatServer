#pragma once
#include <Windows.h>
#include "NetRoot/NetServer/NetSessionID.h"
#include "NetRoot/NetServer/NetPacketSet.h"
#include "NetRoot/Common/RPCBuffer.h"
#include "ChatDummyClient_CS_Proxy.h"
#include "ChatDummyClient.h"

namespace server_baby
{
	class ChatDummyClient_SC_Stub
	{
	public:
		explicit ChatDummyClient_SC_Stub(ChatDummyClient* client) : client_(client) {}

		bool PacketProc(NetPacketSet* msgPack)
		{
			switch (msgPack->GetType())
			{
			case eNET_RECVED_PACKET_SET:
			{
				while (msgPack->GetSize() > 0)
				{
					NetDummyPacket* packet = nullptr;
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

		bool PacketProc(NetSessionID sessionID, NetDummyPacket* msg)
		{
			WORD type;
			*msg >> type;
			switch (type)
			{
			case 0:
			{
				BYTE status;
				INT64 accountNo;
				*msg >> status;
				*msg >> accountNo;
				return ResLogin(status, accountNo, sessionID);
			}
			case 1:
			{
				INT64 accountNo;
				WORD sectorX;
				WORD sectorY;
				*msg >> accountNo;
				*msg >> sectorX;
				*msg >> sectorY;
				return ResSectorMove(accountNo, sectorX, sectorY, sessionID);
			}
			case 2:
			{
				INT64 accountNo;
				WORD messageLen;
				*msg >> accountNo;
				RPCBuffer accountIDBuf(40);
				msg->DeqData((char*)accountIDBuf.data, 40);
				WCHAR* accountID = (WCHAR*)accountIDBuf.Data();
				RPCBuffer nickNameBuf(40);
				msg->DeqData((char*)nickNameBuf.data, 40);
				WCHAR* nickName = (WCHAR*)nickNameBuf.Data();
				*msg >> messageLen;
				RPCBuffer messageBuf(messageLen);
				msg->DeqData((char*)messageBuf.data, messageLen);
				WCHAR* message = (WCHAR*)messageBuf.Data();
				return ResMessage(accountNo, accountID, nickName, messageLen, message, sessionID);
			}
			}
			return false;
		}

		bool ResLogin(BYTE status, INT64 accountNo, NetSessionID sessionID)
		{
			return false;
		}

		bool ResSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY, NetSessionID sessionID)
		{
			return false;
		}

		bool ResMessage(INT64 accountNo, WCHAR* accountID, WCHAR* nickName, WORD messageLen, WCHAR* message, NetSessionID sessionID)
		{
			return false;
		}

	private:
		ChatDummyClient* client_;
	};
}
