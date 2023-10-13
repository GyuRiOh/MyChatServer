#pragma once
#include <unordered_map>
#include "../NetRoot/NetServer/NetSessionID.h"

using namespace std;

namespace server_baby
{

	template <class User>
	class UserMap
	{
		//Key : SessionID, Value : Player*
		unordered_map<INT64, User> userMap_;
	public:
		explicit UserMap() {}
		~UserMap(){}

		bool Find(const INT64 key)
		{
			return (userMap_.find(key) != userMap_.end());
		}

		bool Find(User* playerBuf, const INT64 key)
		{
			auto player = userMap_.find(key);
			if (player != userMap_.end())
			{
				*playerBuf = player->second;
				return true;
			}
			else
				return false;

		}

		bool Exchange(User user, const INT64 key)
		{
			Release(key);
			return Insert(user, key);
		}

		bool Release(const INT64 key)
		{
			auto playerIter = userMap_.find(key);
			if (playerIter == userMap_.end())
				return false;

			userMap_.erase(key);
			return true;
		}

		bool Insert(User player, const INT64 key)
		{
			pair <unordered_map<INT64, User>::iterator, bool> ret = userMap_.insert(make_pair(key, move(player)));
			return ret.second;
		}

		size_t Size()
		{
			return userMap_.size();
		}

		void Clear()
		{
			userMap_.clear();
		}

		void ClearAndSaveItems(User* array)
		{
			int index = 0;
			auto iter = userMap_.begin();
			for (; iter != userMap_.end(); ++iter)
			{
				User user = (*iter).second;
				array[index++] = user;
			}

			userMap_.clear();
		}


	};


}