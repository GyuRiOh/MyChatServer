#pragma once
#include <Windows.h>
#include <ChatServer/Single/cpp_redis/cpp_redis>
#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")
#include "../../NetRoot/NetServer/NetSessionID.h"
#include "../../NetRoot/NetServer/NetPacketSet.h"
#include "ChatServer_SC_Proxy.h"
#include "ChatServer.h"
#include "../../CommonProtocol.h"
#include "../../PacketStruct.h"
#include "../../NetRoot/Common/RPCBuffer.h"
#include "../../NetRoot/NetServer/NetStub.h"

namespace server_baby
{
	class ChatServer_CS_Stub : public NetStub
	{
	public:
		explicit ChatServer_CS_Stub(ChatServer* server) : server_(server) {}

		void OnRecv(NetPacketSet* msgPack) override
		{
			server_->jobQ_PacketQ_.Enqueue(msgPack);
			SetEvent(server_->updateEvent_);
		}
		
		void OnContentsUserJoin(NetSessionID sessionID) override
		{
			Player* player = Player::Alloc(sessionID);
			server_->connectedMap_.Insert(player, sessionID.total_);
			server_->updateTPS_++;
		}
		
		void OnContentsUserLeave(NetSessionID sessionID) override
		{
			server_->ReleasePlayer(sessionID);
			server_->updateTPS_++;
		}

		void OnWorkerClientJoin(NetSessionID sessionID) override
		{
			NetPacketSet* packetQ = NetPacketSet::Alloc(sessionID, eNET_CONTENTS_USER_JOIN);
			server_->jobQ_PacketQ_.Enqueue(packetQ);
			SetEvent(server_->updateEvent_);
		}

		void OnWorkerClientLeave(NetSessionID sessionID) override
		{
			NetPacketSet* packetQ = NetPacketSet::Alloc(sessionID, eNET_CONTENTS_USER_LEAVE);
			server_->jobQ_PacketQ_.Enqueue(packetQ);
			SetEvent(server_->updateEvent_);
		}

		bool PacketProc(NetSessionID sessionID, NetDummyPacket* msg) override
		{
			int packetSize = msg->GetSize();

			server_->updateTPS_++;

			WORD type;
			*msg >> type;
			switch (type)
			{
			case en_PACKET_CS_CHAT_REQ_LOGIN:
			{
				if (packetSize != sizeof(PACKET_CS_CHAT_REQ_LOGIN))
					return false;

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
			case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
			{
				if (packetSize != sizeof(PACKET_CS_CHAT_REQ_SECTOR_MOVE))
					return false;
				
				INT64 accountNo;
				WORD sectorX;
				WORD sectorY;
				*msg >> accountNo;
				*msg >> sectorX;
				*msg >> sectorY;
				return ReqSectorMove(accountNo, sectorX, sectorY, sessionID);
			}
			case en_PACKET_CS_CHAT_REQ_MESSAGE:
			{
				if (packetSize > sizeof(PACKET_CS_CHAT_REQ_MESSAGE))
					return false;
				
				INT64 accountNo;
				WORD messageLen;
				*msg >> accountNo;
				*msg >> messageLen;
				RPCBuffer messageBuf(messageLen);
				msg->DeqData((char*)messageBuf.data, messageLen);
				WCHAR* message = (WCHAR*)messageBuf.Data();
				return ReqMessage(accountNo, messageLen, message, sessionID);
			}
			case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
				return true;

			}
			return false;
		}

		bool ReqLogin(INT64 accountNo, WCHAR* accountID, WCHAR* nickName, char* sessionKey, NetSessionID sessionID)
		{
			if (accountNo < 0)
				return false;

			Player* player = nullptr;
			if (!server_->connectedMap_.Find(&player, sessionID.total_))
			{
				SystemLogger::GetInstance()->LogText(L"ChatServer",
					LEVEL_ERROR, L"Not in connected map", player->GetAccountNumber());

				return false;
			}

			//�ߺ��α��� üũ - �÷��̾� ������ �̹� �ԷµǾ��ٸ�?
			if (!player->UpdateInfo(accountNo, accountID, nickName))
			{
				SystemLogger::GetInstance()->LogText(L"ChatServer",
					LEVEL_ERROR, L"Login - Already Info Exists. %d", player->GetAccountNumber());

				return false;
			}
			
			//�ߺ��α��� üũ - �¶��� ��
			if (!server_->onlineMap_.Insert(sessionID, accountNo))
			{
				SystemLogger::GetInstance()->LogText(L"ChattingServer",
					LEVEL_APC, L"Login - Duplicated. %d", accountNo);

				server_->DisconnectDuplicatedPlayer(accountNo);
				return false;
			}

			if (accountNo < 1000000)
				server_->proxy_->ResLogin(true, accountNo, sessionID);
			else
			{
				RedisJob* job = reinterpret_cast<RedisJob*>(SizedMemoryPool::GetInstance()->Alloc(sizeof(RedisJob)));
				job->acoountNum = accountNo;
				job->sessionID = sessionID;
				std::memmove(job->sessionKey, sessionKey, 64);
				server_->RequireAuth(job);
			}

			return true;
		}

		bool ReqSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY, NetSessionID sessionID)
		{
			Player* player = nullptr;
			if (!server_->connectedMap_.Find(&player, sessionID.total_))
			{
				SystemLogger::GetInstance()->LogText(L"ChatServer",
					LEVEL_ERROR, L"SectorMove Failed - Not exist in connected map. %d", player->GetAccountNumber());

				return false;
			}

			if (player->GetAccountNumber() != accountNo)
				return false;

			if (!player->UpdateSector(sectorX, sectorY))
				return false;

			server_->sectorMap_.Update(player);

			server_->proxy_->ResSectorMove(
				player->GetAccountNumber(),
				player->curSector_._xPos,
				player->curSector_._yPos,
				sessionID);

			return true;
		}

		bool ReqMessage(INT64 accountNo, WORD messageLen, WCHAR* message, NetSessionID sessionID)
		{

			Player* sender = nullptr;
			if (!server_->connectedMap_.Find(&sender, sessionID.total_))
			{
				SystemLogger::GetInstance()->LogText(L"ChatServer",
					LEVEL_ERROR, L"Message Failed - Not exist in connected map. %d", sender->GetAccountNumber());

				return false;
			}

			if (sender->GetAccountNumber() != accountNo)
				return false;

			server_->proxy_->ResMessage(
				sender,
				sender->GetAccountNumber(),
				sender->GetID(),
				sender->GetNickName(),
				messageLen,
				message,
				sessionID);

			return true;
		}

	private:
		ChatServer* server_;
	};
}
