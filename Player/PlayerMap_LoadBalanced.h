#pragma once
#include <unordered_map>

#define HASH_NUM 256
using namespace std;

namespace MyNetwork
{

	template <class User>
	class LoadBalancedUserMap
	{
	public:
		unordered_map<INT64, User> userMap_[HASH_NUM];

		explicit LoadBalancedUserMap() {}
		~LoadBalancedUserMap() {}

		bool Find(const INT64 key)
		{
			unsigned char mapIndex = (unsigned char)key;
			return (userMap_[mapIndex].find(key) != userMap_[mapIndex].end());

		}

		bool Find(User* playerBuf, const INT64 key)
		{
			unsigned char mapIndex = (unsigned char)key;

			auto playerIter = userMap_[mapIndex].find(key);
			if (playerIter != userMap_[mapIndex].end())
			{
				*playerBuf = playerIter->second;
				return true;
			}
			else
				return false;

		}

		bool Release(const INT64 key)
		{
			unsigned char mapIndex = (unsigned char)key;

			auto playerIter = userMap_[mapIndex].find(key);
			if (playerIter == userMap_[mapIndex].end())
				return false;

			userMap_[mapIndex].erase(key);
			return true;
		}

		bool Insert(User player, const INT64 key)
		{
			unsigned char mapIndex = (unsigned char)key;

			pair <unordered_map<INT64, User>::iterator, bool> ret = userMap_[mapIndex].insert(make_pair(key, player));
			return ret.second;
		}

		size_t Size(unsigned char index)
		{
			return userMap_[index].size();
		}

		int SizeAll()
		{

			int size = 0;
			for (int i = 0; i < HASH_NUM; i++)
			{
				size += userMap_[i].size();
			}

			return size;
		}

		void Clear()
		{
			for (int i = 0; i < HASH_NUM; i++)
			{
				userMap_[i].clear();
			}

		}

		void ClearAndSaveItems(User* array)
		{
			int index = 0;
			for (int i = 0; i < HASH_NUM; i++)
			{
				auto iter = userMap_[i].begin();
				for (; iter != userMap_[i].end(); ++iter)
				{
					User user = (*iter).second;
					array[index++] = user;
				}

				userMap_[i].clear();
			}
		}

	};


}