#pragma once
#include <unordered_map>
#include "../Player/PlayerWithLock.h"
#include "../PacketStruct.h"
#include "SectorMap.h"
#include "../NetRoot/Common/Lock.h"

namespace server_baby
{
	template<class User>
	class SectorMapWithLock : public SectorMap<User>
	{
		SRWLockObject sectorLock_[SECTOR_Y_MAX][SECTOR_X_MAX];
	public:
		bool Update(User player, short curX, short curY)
		{
			player->lock_sector_.Lock_Exclusive();

			if (!player->UpdateSector(curX, curY))
			{
				player->lock_sector_.Unlock_Exclusive();
				return false;
			}

			SectorPos oldSector = player->GetOldSector();

			if (oldSector._xPos != X_DEFAULT)
			{
				if (curX == oldSector._xPos &&
					curY == oldSector._yPos)
				{
					//SystemLogger::GetInstance()->LogText(L"SectorMap_Update", LEVEL_DEBUG, L"Same Sector, Not Updated");
					player->lock_sector_.Unlock_Exclusive();
					return true;
				}

				LockSector_Exclusive(oldSector._xPos, oldSector._yPos);
				this->RemoveFromOldSector(player);
				UnlockSector_Exclusive(oldSector._xPos, oldSector._yPos);

				LockSector_Exclusive(curX, curY);
				this->AddToCurSector(player);
				UnlockSector_Exclusive(curX, curY);

			}
			else
			{
				LockSector_Exclusive(curX, curY);
				this->AddToCurSector(player);
				UnlockSector_Exclusive(curX, curY);
			}

			player->lock_sector_.Unlock_Exclusive();
			return true;
		}

		void LockSector_Exclusive(short x, short y)
		{
			sectorLock_[y][x].Lock_Exclusive();
		}

		void UnlockSector_Exclusive(short x, short y)
		{
			sectorLock_[y][x].Unlock_Exclusive();
		}

		void LockSector_Shared(short x, short y)
		{
			sectorLock_[y][x].Lock_Shared();
		}

		void UnlockSector_Shared(short x, short y)
		{
			sectorLock_[y][x].Unlock_Shared();
		}

	};
}