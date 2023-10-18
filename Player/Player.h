#pragma once
#include "../NetRoot/Common/MemTLS.h"
#include "../NetRoot/NetServer/NetSessionID.h"
#include "../Sector/Sector.h"
#include "../PacketStruct.h"

using namespace std;
namespace MyNetwork
{
	class Player
	{
		//락오브젝트 추가하기
		explicit Player() = delete;
		~Player() = delete;
	public:

		static int GetUsedCount();
		static int GetCapacity();
		static Player* Alloc(const NetSessionID NetSessionID);

		static bool Free(Player* player);

		NetSessionID GetSessionID();

		bool UpdateSector(const short x, const short y);
		bool UpdateInfo(PACKET_CS_CHAT_REQ_LOGIN* loginPacket);
		bool UpdateInfo(INT64 accountNo, WCHAR* ID, WCHAR* nickName);

		SectorPos GetCurSector() const;
		SectorPos GetOldSector() const;
		INT64 GetAccountNumber() const;
		void SetAccountNum(INT64 num);
		WCHAR* GetID();
		WCHAR* GetNickName();


	private:
		void Initialize(const NetSessionID NetSessionID);
		void Destroy();


	public:
		SectorPos oldSector_;
		SectorPos curSector_;

	private:
		INT64 accountNum_;
		NetSessionID sessionID_;
		WCHAR ID_[20] = { 0 };
		WCHAR nickName_[20] = { 0 };
		static MyNetwork::MemTLS<Player>* playerPool_;
	};

	inline int Player::GetUsedCount()
	{
		return playerPool_->GetTotalUseCount();
	}

	inline int Player::GetCapacity()
	{
		return playerPool_->GetTotalCapacity();
	}

	inline Player* Player::Alloc(const NetSessionID NetSessionID)
	{
		Player* player = playerPool_->Alloc();
		player->Initialize(NetSessionID);
		return player;
	}

	inline bool Player::Free(Player* player)
	{
		player->Destroy();
		return playerPool_->Free(player);
	}

	inline NetSessionID Player::GetSessionID()
	{
		return sessionID_;
	}

	inline bool Player::UpdateSector(const short x, const short y)
	{
		if (x >= SECTOR_X_MAX || x < 0 || y >= SECTOR_Y_MAX || y < 0)
			return false;

		oldSector_ = curSector_;
		curSector_._xPos = x;
		curSector_._yPos = y;
		return true;
	}

	inline bool Player::UpdateInfo(PACKET_CS_CHAT_REQ_LOGIN* loginPacket)
	{
		if (accountNum_ != NULL)
			return false;

		accountNum_ = loginPacket->accountNum_;
		wcscpy_s(ID_, loginPacket->ID_);
		wcscpy_s(nickName_, loginPacket->nickName_);
		return true;
	}

	inline bool Player::UpdateInfo(INT64 accountNo, WCHAR* ID, WCHAR* nickName)
	{
		if (accountNum_ != NULL)
			return false;

		accountNum_ = accountNo;
		wcscpy_s(ID_, ID);
		wcscpy_s(nickName_, nickName);
		return true;
	}

	inline SectorPos Player::GetCurSector() const
	{
		return curSector_;
	}

	inline SectorPos Player::GetOldSector() const
	{
		return oldSector_;
	}

	inline INT64 Player::GetAccountNumber() const
	{
		return accountNum_;
	}

	inline void Player::SetAccountNum(INT64 num)
	{
		accountNum_ = num;
	}

	inline WCHAR* Player::GetID()
	{
		return ID_;
	}

	inline WCHAR* Player::GetNickName()
	{
		return nickName_;
	}

	inline void MyNetwork::Player::Initialize(const NetSessionID NetSessionID)
	{
		accountNum_ = NULL;
		sessionID_ = NetSessionID;
		curSector_._xPos = X_DEFAULT;
		curSector_._yPos = Y_DEFAULT;
		oldSector_ = curSector_;
	}

	inline void MyNetwork::Player::Destroy()
	{
		accountNum_ = NULL;
		curSector_._xPos = X_DEFAULT;
		curSector_._yPos = Y_DEFAULT;
		oldSector_ = curSector_;
		sessionID_ = 0;

	}

}