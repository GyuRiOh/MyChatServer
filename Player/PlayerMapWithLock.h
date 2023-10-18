#pragma once
#include <unordered_map>
#include "PlayerMap_LoadBalanced.h"
#include "../NetRoot/Common/Lock.h"

namespace MyNetwork
{
	template<class User>
	class UserMapWithLock : protected LoadBalancedUserMap<User>
	{
		SRWLockObject lock_[HASH_NUM];
	public:
		bool Find_SharedLock(User* user, const INT64 key)
		{
			unsigned char index = (unsigned char)key;

			lock_[index].Lock_Shared();
			bool ret = this->Find(user, key);
			lock_[index].Unlock_Shared();

			return ret;
		}

		bool Insert_ExclusiveLock(User user, const INT64 key)
		{
			unsigned char index = (unsigned char)key;

			lock_[index].Lock_Exclusive();
			bool ret = this->Insert(user, key);
			lock_[index].Unlock_Exclusive();

			return ret;
		}

		bool Release_ExclusiveLock(const INT64 key)
		{
			unsigned char index = (unsigned char)key;

			lock_[index].Lock_Exclusive();
			bool ret = this->Release(key);
			lock_[index].Unlock_Exclusive();

			return ret;
		}

		bool Exchange_ExclusiveLock(User user, const INT64 key)
		{
			unsigned char index = (unsigned char)key;

			lock_[index].Lock_Exclusive();
			bool ret = this->Release(key);
			ret = this->Insert(user, key);
			lock_[index].Unlock_Exclusive();

			return ret;
		}

		int Size_SharedLock()
		{
			int size = 0;

			for (int i = 0; i < HASH_NUM; i++)
			{
				lock_[i].Lock_Shared();
				size += static_cast<int>(this->Size(i));
				lock_[i].Unlock_Shared();
			}

			return size;
		}

		void Clear_Lock()
		{
			this->Clear();
		}

		void ClearAndFreeItems_Lock(User* array)
		{
			this->ClearAndSaveItems(array);
		}
	};

}