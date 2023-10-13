#pragma once
#include <Windows.h>
#include <ChatServer/Multi/cpp_redis/cpp_redis>
#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")
#include "../../NetRoot/NetServer/NetSessionID.h"
#include "../../NetRoot/NetServer/NetPacketSet.h"
#include "MultiChatServer_SC_Proxy.h"
#include "MultiChatServer.h"
#include "../../CommonProtocol.h"
#include "../../PacketStruct.h"
#include "../../NetRoot/Common/RPCBuffer.h"
#include "../../NetRoot/NetServer/NetStub.h"

namespace server_baby
{
	class MultiChatServer_CS_Stub : public NetStub
	{
	public:
		explicit MultiChatServer_CS_Stub(MultiChatServer* server) : server_(server) {}

		void OnRecv(NetPacketSet* msgPack) override
		{
			server_->PacketProc(msgPack);
		}
		
		void OnContentsUserJoin(NetSessionID sessionID) override
		{
		}
		
		void OnContentsUserLeave(NetSessionID sessionID) override
		{
		}

		void OnWorkerClientJoin(NetSessionID sessionID) override
		{

			PlayerWithLock* player = PlayerWithLock::Alloc(sessionID);
			server_->connectedMap_.Insert_ExclusiveLock(player, sessionID.total_);

		}

		void OnWorkerClientLeave(NetSessionID sessionID) override
		{
			server_->ReleasePlayer(sessionID);
		}

		bool PacketProc(NetSessionID sessionID, NetDummyPacket* msg) override
		{
			int packetSize = msg->GetSize();

			server_->IncrementUpdateTPS();

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
			}
			return false;
		}

		bool ReqLogin(INT64 accountNo, WCHAR* accountID, WCHAR* nickName, char* sessionKey, NetSessionID sessionID)
		{
			if (accountNo > 95000 || accountNo < 0)
				return false;

			PlayerWithLock* player = nullptr;
			if (!server_->connectedMap_.Find_SharedLock(&player, sessionID.total_))
			{
				SystemLogger::GetInstance()->LogText(L"ChatServer",
					LEVEL_ERROR, L"Not in connected map", player->GetAccountNumber());

				return false;
			}

			//중복로그인 체크 - 플레이어 정보가 이미 입력되었다면?
			if (!player->UpdateInfo(accountNo, accountID, nickName))
			{
				SystemLogger::GetInstance()->LogText(L"ChatServer",
					LEVEL_ERROR, L"Login - Already Info Exists. %d", player->GetAccountNumber());

				return false;
			}
			
			//중복로그인 체크 - 온라인 맵
			if (!server_->onlineMap_.Insert_ExclusiveLock(sessionID, accountNo))
			{
				SystemLogger::GetInstance()->LogText(L"ChattingServer",
					LEVEL_APC, L"Login - Duplicated. %d", accountNo);

				server_->DisconnectDuplicatedPlayer(accountNo);
				return false;
			}

			RedisJob* job = reinterpret_cast<RedisJob*>(SizedMemoryPool::GetInstance()->Alloc(sizeof(RedisJob)));
			job->acoountNum = accountNo;
			job->sessionID = sessionID;
			std::memmove(job->sessionKey, sessionKey, 64);
			server_->RequireAuth(job);

			server_->proxy_->ResLogin(
				true,
				accountNo,
				sessionID
			);


		}

		bool ReqSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY, NetSessionID sessionID)
		{
			PlayerWithLock* player = nullptr;
			if (!server_->connectedMap_.Find_SharedLock(&player, sessionID.total_))
			{
				SystemLogger::GetInstance()->LogText(L"ChatServer",
					LEVEL_ERROR, L"SectorMove Failed - Not exist in connected map. %d", player->GetAccountNumber());

				return false;
			}

			if (player->GetAccountNumber() != accountNo)
				return false;

			if (!player->UpdateSector(sectorX, sectorY))
				return false;

			server_->sectorMap_.Update(player,
				player->curSector_._xPos,
				player->curSector_._yPos);

			server_->proxy_->ResSectorMove(
				player->GetAccountNumber(),
				player->curSector_._xPos,
				player->curSector_._yPos,
				sessionID);

			return true;
		}

		bool ReqMessage(INT64 accountNo, WORD messageLen, WCHAR* message, NetSessionID sessionID)
		{

			PlayerWithLock* sender = nullptr;
			if (!server_->connectedMap_.Find_SharedLock(&sender, sessionID.total_))
			{
				SystemLogger::GetInstance()->LogText(L"ChatServer",
					LEVEL_ERROR, L"Message Failed - Not exist in connected map. %d", sender->GetAccountNumber());

				CrashDump::Crash();
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
		MultiChatServer* server_;
	};
}
