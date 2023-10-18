#pragma once
#include <unordered_map>
#include "../PacketStruct.h"
#include "Sector.h"
#include "../NetRoot/Common/Crash.h"
#include "../NetRoot/NetServer/NetSessionID.h"

using namespace std;
namespace MyNetwork
{

	template<class User>
	class SectorMap
	{
	public:
		explicit SectorMap(){}
		~SectorMap(){}

		void AddToCurSector(User player)
		{
			sectorMap_[player->curSector_._yPos][player->curSector_._xPos].insert(make_pair(
					player->GetSessionID().total_, player));
		}

		void RemoveFromOldSector(User player) 
		{
			sectorMap_[player->oldSector_._yPos][player->oldSector_._xPos].erase(
				player->GetSessionID().total_);
		}

		bool RemoveFromCurSector(User player)
		{
			if (player->curSector_._xPos != X_DEFAULT)
			{
				if (sectorMap_[player->curSector_._yPos][player->curSector_._xPos].erase(
					player->GetSessionID().total_) != 1)
					ErrorQuit(L"RemoveFromCurSector - Not Erased");

				return true;
			}
			else
				return false;
		}

		void Update(User player)
		{
			SectorPos curSector = player->GetCurSector();
			SectorPos oldSector = player->GetOldSector();

			if (oldSector._xPos != X_DEFAULT)
			{
				if (curSector._xPos == oldSector._xPos &&
					curSector._yPos == oldSector._yPos)
					return;

				RemoveFromOldSector(player);
				AddToCurSector(player);

			}
			else
				AddToCurSector(player);

		}

		void GetSectorAround(User player, SectorAround* sectorAround) 
		{
			SectorPos curSector = player->GetCurSector();
			short sectorX = curSector._xPos;
			short sectorY = curSector._yPos;

			sectorX--;
			sectorY--;

			sectorAround->_count = 0;

			for (int iCntY = 0; iCntY < 3; iCntY++)
			{

				if (sectorY + iCntY < 0 || sectorY + iCntY >= SECTOR_Y_MAX)
					continue;

				for (int iCntX = 0; iCntX < 3; iCntX++)
				{
					if (sectorX + iCntX < 0 || sectorX + iCntX >= SECTOR_X_MAX)
						continue;

					sectorAround->_around[sectorAround->_count]._xPos = sectorX + iCntX;
					sectorAround->_around[sectorAround->_count]._yPos = sectorY + iCntY;
					sectorAround->_count++;
				}
			}

		}

		void Clear()
		{
			for (int iCntY = 0; iCntY < SECTOR_Y_MAX; iCntY++)
			{
				for (int iCntX = 0; iCntX < SECTOR_X_MAX; iCntX++)
				{
					sectorMap_[iCntY][iCntX].clear();
				}
			}
		}

		void ErrorQuit(const WCHAR* msg)
		{
			SystemLogger::GetInstance()->Console(L"SectorMap", LEVEL_SYSTEM, msg);
			SystemLogger::GetInstance()->LogText(L"SectorMap", LEVEL_SYSTEM, msg);

			CrashDump::Crash();
		}	

		//Key: SessionID, Value : Player*
		unordered_map<INT64, User> sectorMap_[SECTOR_Y_MAX][SECTOR_X_MAX];
		
	};
}