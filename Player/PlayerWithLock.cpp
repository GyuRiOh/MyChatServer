#include "PlayerWithLock.h"
#include "../NetRoot/NetServer/NetEnums.h"

using namespace MyNetwork;

MemTLS<PlayerWithLock>* PlayerWithLock::playerPool_ = new MemTLS<PlayerWithLock>(200, 1, ePLAYER_POOL_CODE);

void MyNetwork::PlayerWithLock::Initialize(NetSessionID sessionID)
{
	lock_sector_.Initialize();
	//lock_accountInfo_.Initialize();

	//lock_accountInfo_.Lock_Exclusive();
	accountNum_ = NULL;
	sessionID_ = sessionID;
	//lock_accountInfo_.Unlock_Exclusive();

	lock_sector_.Lock_Exclusive();
	curSector_._xPos = X_DEFAULT;
	curSector_._yPos = Y_DEFAULT;
	oldSector_ = curSector_;
	lock_sector_.Unlock_Exclusive();
}

void MyNetwork::PlayerWithLock::Destroy()
{
	//lock_accountInfo_.Lock_Exclusive();
	accountNum_ = NULL;
	sessionID_ = 0;
	//lock_accountInfo_.Unlock_Exclusive();

	lock_sector_.Lock_Exclusive();
	curSector_._xPos = X_DEFAULT;
	curSector_._yPos = Y_DEFAULT;
	oldSector_ = curSector_;
	lock_sector_.Unlock_Exclusive();

}