
#include "Player.h"
#include "../NetRoot/NetServer/NetEnums.h"

using namespace server_baby;

MemTLS<Player>* Player::playerPool_ = new MemTLS<Player>(200, 1, ePLAYER_POOL_CODE);

