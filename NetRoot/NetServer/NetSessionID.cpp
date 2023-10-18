#include "NetSessionID.h"
#include "NetEnums.h"
using namespace MyNetwork;

MemTLS<NetSessionIDSet>* NetSessionIDSet::idSetPool_ = new MemTLS<NetSessionIDSet>(100, 1, eSESSION_ID_SET_POOL_CODE);
