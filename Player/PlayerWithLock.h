#pragma once
#include "../NetRoot/Common/MemTLS.h"
#include "../NetRoot/NetServer/NetSessionID.h"
#include "../Sector/Sector.h"
#include "../PacketStruct.h"
#include "../NetRoot/Common/Lock.h"

namespace MyNetwork
{
	class PlayerWithLock
	{
		//락오브젝트 추가하기
		explicit PlayerWithLock() : accountNum_(NULL), sessionID_(NULL) {}
	public:
		~PlayerWithLock() {}

		static PlayerWithLock* Alloc(NetSessionID sessionID);
		static bool Free(PlayerWithLock* player);
		static int GetUsedCount();
		static int GetCapacity();

		NetSessionID GetSessionID();
		bool UpdateSector(short x, short y);
		bool UpdateInfo(PACKET_CS_CHAT_REQ_LOGIN* loginPacket);
		bool UpdateInfo(INT64 accountNo, WCHAR* accountID, WCHAR* nickName);
		SectorPos GetCurSector();
		SectorPos GetOldSector();
		INT64 GetAccountNumber();
		WCHAR* GetID();
		WCHAR* GetNickName();

	private:
		void Initialize(NetSessionID sessionID);
		void Destroy();

	private:
		//SRWLockObject lock_accountInfo_;
		INT64 accountNum_;
		NetSessionID sessionID_;
		WCHAR ID_[20] = { 0 };
		WCHAR nickName_[20] = { 0 };
	public:
		SRWLockObject lock_sector_;
		SectorPos oldSector_;
		SectorPos curSector_;

		static MyNetwork::MemTLS<PlayerWithLock>* playerPool_;
	};

	inline int PlayerWithLock::GetUsedCount()
	{
		return playerPool_->GetTotalUseCount();
	}

	inline int PlayerWithLock::GetCapacity()
	{
		return playerPool_->GetTotalCapacity();
	}

	inline NetSessionID PlayerWithLock::GetSessionID()
	{
		return sessionID_;
	}

	inline bool PlayerWithLock::UpdateSector(short x, short y)
	{
		if (x >= SECTOR_X_MAX || x < 0 || y >= SECTOR_Y_MAX || y < 0)
			return false;

		//lock_sector_.Lock_Exclusive();
		oldSector_ = curSector_;
		curSector_._xPos = x;
		curSector_._yPos = y;
		//lock_sector_.Unlock_Exclusive();

		return true;
	}

	inline bool PlayerWithLock::UpdateInfo(PACKET_CS_CHAT_REQ_LOGIN* loginPacket)
	{
		//lock_accountInfo_.Lock_Exclusive();
		if (accountNum_ != NULL)
		{
			//lock_accountInfo_.Unlock_Exclusive();
			return false;
		}

		accountNum_ = loginPacket->accountNum_;
		wcscpy_s(ID_, loginPacket->ID_);
		wcscpy_s(nickName_, loginPacket->nickName_);
		//lock_accountInfo_.Unlock_Exclusive();
		return true;
	}

	inline bool PlayerWithLock::UpdateInfo(INT64 accountNo, WCHAR* accountID, WCHAR* nickName)
	{  /*
		lock_accountInfo_.Lock_Exclusive();*/
		if (accountNum_ != NULL)
		{/*
			lock_accountInfo_.Unlock_Exclusive();*/
			return false;
		}

		accountNum_ = accountNo;
		wcscpy_s(ID_, accountID);
		wcscpy_s(nickName_, nickName);
		//lock_accountInfo_.Unlock_Exclusive();
		return true;
	}

	inline SectorPos PlayerWithLock::GetCurSector()
	{

		lock_sector_.Lock_Shared();
		SectorPos ret = curSector_;
		lock_sector_.Unlock_Shared();

		return ret;
	}

	inline SectorPos PlayerWithLock::GetOldSector()
	{
		SectorPos ret = oldSector_;
		return ret;
	}

	inline INT64 PlayerWithLock::GetAccountNumber()
	{
		//lock_accountInfo_.Lock_Shared();
		INT64 accountNum = accountNum_;
		//lock_accountInfo_.Unlock_Shared();

		return accountNum;
	}

	inline WCHAR* PlayerWithLock::GetID()
	{
		return ID_;
	}

	inline WCHAR* PlayerWithLock::GetNickName()
	{
		return nickName_;
	}

	inline PlayerWithLock* PlayerWithLock::Alloc(NetSessionID sessionID)
	{
		PlayerWithLock* player = playerPool_->Alloc();
		player->Initialize(sessionID);
		return player;
	}

	inline bool PlayerWithLock::PlayerWithLock::Free(PlayerWithLock* player)
	{
		player->Destroy();
		return playerPool_->Free(player);
	}

}