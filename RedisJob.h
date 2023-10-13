#pragma once
#include "NetRoot/NetServer/NetSessionID.h"

namespace server_baby
{
	struct RedisJob
	{
		char sessionKey[64] = { 0 };
		INT64 acoountNum = NULL;
		NetSessionID sessionID;
	};
}
