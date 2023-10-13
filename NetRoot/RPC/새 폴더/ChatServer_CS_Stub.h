#pragma once
#include <Windows.h>
#include "NetRoot/NetServer/NetSessionID.h"
#include "NetRoot/NetServer/NetPacketSet.h"
#include "NetRoot/Common/RPCBuffer.h"
#include "NetRoot/NetServer/NetStub.h"
#include "ChatServer_SC_Proxy.h"
#include "ChatServer.h"

namespace server_baby
{
	class ChatServer_CS_Stub : public NetStub
	{
	public:
		explicit ChatServer_CS_Stub(ChatServer* server) : server_(server) {}

		void OnRecv(NetPacketSet* msgPack) override
		{

		}

		void OnContentsUserJoin(NetSessionID sessionID) override
		{

		}

		void OnContentsUserLeave(NetSessionID sessionID) override
		{

		}

		void OnWorkerClientJoin(NetSessionID sessionID) override
		{

		}

		void OnWorkerClientLeave(NetSessionID sessionID) override
		{

		}

		bool PacketProc(NetSessionID sessionID, NetDummyPacket* msg) override
		{
			WORD type;
			*msg >> type;
			switch (type)
			{
			case 0:
			{
				INT64 accountNo;
				*msg >> accountNo;
				RPCBuffer accountIDBuf(40);
				msg->DeqData((char*)accountIDBuf.data, 40);
				WCHAR* accountID = (WCHAR*)accountIDBuf.Data();
				RPCBuffer nickNameBuf(40);
				msg->DeqData((char*)nickNameBuf.data, 40);
				WCHAR* nickName = (WCHAR*)nickNameBuf.Data();
				RPCBuffer sessionKeyBuf(64);
				msg->DeqData((char*)sessionKeyBuf.data, 64);
				char* sessionKey = (char*)sessionKeyBuf.Data();
				return ReqLogin(accountNo, accountID, nickName, sessionKey, sessionID);
			}
			case 1:
			{
				INT64 accountNo;
				WORD sectorX;
				WORD sectorY;
				*msg >> accountNo;
				*msg >> sectorX;
				*msg >> sectorY;
				return ReqSectorMove(accountNo, sectorX, sectorY, sessionID);
			}
			case 2:
			{
				INT64 accountNo;
				WORD messageLen;
				*msg >> accountNo;
				*msg >> messageLen;
				RPCBuffer messageBuf(messageLen);
				msg->DeqData((char*)messageBuf.data, messageLen);
				WCHAR* message = (WCHAR*)messageBuf.Data();
				return ReqMessage(accountNo, messageLen, message, sessionID);
			}
			}
			return false;
		}

		bool ReqLogin(INT64 accountNo, WCHAR* accountID, WCHAR* nickName, char* sessionKey, NetSessionID sessionID)
		{
			return false;
		}

		bool ReqSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY, NetSessionID sessionID)
		{
			return false;
		}

		bool ReqMessage(INT64 accountNo, WORD messageLen, WCHAR* message, NetSessionID sessionID)
		{
			return false;
		}

	private:
		ChatServer* server_;
	};
}
