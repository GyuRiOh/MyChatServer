
#include "Player.h"
#include "../NetRoot/NetServer/NetEnums.h"

using namespace MyNetwork;

MemTLS<Player>* Player::playerPool_ = new MemTLS<Player>(200, 1, ePLAYER_POOL_CODE);

