#include "LanSessionID.h"
#include "LanEnums.h"
using namespace MyNetwork;

MemTLS<LanSessionIDSet>* LanSessionIDSet::idSetPool_ = new MemTLS<LanSessionIDSet>(500, 1, eLAN_SESSION_ID_SET_POOL_CODE);
