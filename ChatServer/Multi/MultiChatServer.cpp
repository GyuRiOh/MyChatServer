
#include "MultiChatServer.h"
#include "../../CommonProtocol.h"
#include "../../MonitorProtocol.h"
#include "../../NetRoot/Common/PDHMonitor.h"
#include "../../NetRoot/Common/SystemLogger.h"
#include "MultiChatServer_CS_Stub.h"
#include "MultiChatServer_SC_Proxy.h"
#include "../../NetRoot/Common/SizedMemoryPool.h"
#include "../../NetRoot/Common/Parser.h"
using namespace std;
using namespace server_baby;

server_baby::MultiChatServer::MultiChatServer()
    : authEvent_(CreateEvent(NULL, FALSE, NULL, NULL)), 
    authThread_(INVALID_HANDLE_VALUE), authThreadID_(NULL)
{

    TPSArray_.reserve(20);
    RegisterStub(new MultiChatServer_CS_Stub(this));
    proxy_ = new MultiChatServer_SC_Proxy(this);

    authThread_ = (HANDLE)_beginthreadex(
        NULL,
        0,
        (_beginthreadex_proc_type)&AuthThread,
        (LPVOID)this,
        0,
        (unsigned int*)&authThreadID_);

    if (!authThread_)
        ErrorQuit(L"MonitorThread Start Failed");

    int relayPort = 0;
    Parser::GetInstance()->GetValue("RelayServerPort", (int*)&relayPort);
    SystemLogger::GetInstance()->Console(L"NetServer", LEVEL_DEBUG, L"Relay Server Port : %d", relayPort);

    char IP[16] = "127.0.0.1";
    lanClient_.Start(IP, relayPort); 

}

server_baby::MultiChatServer::~MultiChatServer()
{
    delete proxy_;
}

void server_baby::MultiChatServer::OnError(int errCode, WCHAR*)
{

}

void server_baby::MultiChatServer::OnMonitor(const MonitoringInfo* const info)
{
    HardwareMonitor::CpuUsageForProcessor::GetInstance()->UpdateCpuTime();
    ProcessMonitor::CpuUsageForProcess::GetInstance()->UpdateCpuTime();
    ProcessMonitor::MemoryForProcess::GetInstance()->Update();


    time_t timer;
    timer = time(NULL);

    LanPacket* packet = LanPacket::Alloc();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN);
    *packet << true;
    *packet << static_cast<int>(timer);
    
    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU);
    *packet << static_cast<int>(ProcessMonitor::CpuUsageForProcess::GetInstance()->ProcessTotal());
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM);
    *packet << static_cast<int>(ProcessMonitor::MemoryForProcess::GetInstance()->GetPrivateBytes(L"ChatServer") / eMEGA_BYTE);
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_CHAT_SESSION);
    *packet << static_cast<int>(connectedMap_.Size_SharedLock());
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_CHAT_PLAYER);
    *packet << static_cast<int>(onlineMap_.Size_SharedLock());
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS);
    *packet << static_cast<int>(GetUpdateTPS());
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL);
    *packet << static_cast<int>(info->packetCount_);
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL);
    *packet << static_cast<int>(HardwareMonitor::CpuUsageForProcessor::GetInstance()->ProcessorTotal());
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY);
    *packet << static_cast<int>(ProcessMonitor::MemoryForProcess::GetInstance()->GetNonPagedByte() / eMEGA_BYTE);
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV);
    *packet << static_cast<int>(ProcessMonitor::MemoryForProcess::GetInstance()->GetTotalRecvBytes() / eKILO_BYTE);
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND);
    *packet << static_cast<int>(ProcessMonitor::MemoryForProcess::GetInstance()->GetTotalSendBytes() / eKILO_BYTE);
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY);
    *packet << static_cast<int>(ProcessMonitor::MemoryForProcess::GetInstance()->GetAvailableBytes());
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);

    LanPacket::Free(packet);


    ProcessMonitor::MemoryForProcess::GetInstance()->ZeroEthernetValue();

    ZeroTPS();

}

DWORD __stdcall server_baby::MultiChatServer::AuthThread(LPVOID arg)
{
    //Redis 스레드로 활용
    WORD version = MAKEWORD(2, 2);
    WSADATA data;
    WSAStartup(version, &data);

    cpp_redis::client client;
    client.connect();

    MultiChatServer* server = reinterpret_cast<MultiChatServer*>(arg);
    while (server->isServerCreated())
    {
        WaitForSingleObject(server->authEvent_, INFINITE);
        while (!server->jobQ_Redis_.isEmpty())
        {
            RedisJob* redisJob = nullptr;
            server->jobQ_Redis_.Dequeue(&redisJob);

            char accountNum[32] = { 0 };
            _itoa(redisJob->acoountNum, accountNum, 10);

            client.get(accountNum, [redisJob](cpp_redis::reply& reply) {
                if (reply.as_string().compare(redisJob->sessionKey) != 0)
                    cout << "Redis Not Equal" << endl;
            });

            client.sync_commit();

            server->proxy_->ResLogin(
                true,
                redisJob->acoountNum,
                redisJob->sessionID
            );

            SizedMemoryPool::GetInstance()->Free(redisJob);
        }
    }
    
    return 0;
}

void server_baby::MultiChatServer::TrySendPacket_Sector(PlayerWithLock* player, NetPacket* packet)
{
    short x = player->curSector_._xPos;
    short y = player->curSector_._yPos;

    auto iter = sectorMap_.sectorMap_[y][x].begin();
    for (; iter != sectorMap_.sectorMap_[y][x].end();)
    {
        PlayerWithLock* receiver = (*iter).second;
        AsyncSendPacket(receiver->GetSessionID(), packet);
    }

}

void server_baby::MultiChatServer::TrySendPacket_SectorAround(PlayerWithLock* player, NetPacket* packet)
{

    AsyncSendPacket(player->GetSessionID(), packet);

    NetSessionIDSet* idSet = NetSessionIDSet::Alloc();
    GetNetSessionIDSet_Sector(player, idSet);
    if (idSet->GetSize() == 0)
    {
        NetSessionIDSet::Free(idSet);
        return;
    }

    AsyncSendPacket(idSet, packet);

}

void server_baby::MultiChatServer::GetNetSessionIDSet_Sector(PlayerWithLock* player, NetSessionIDSet* set)
{
    SectorAround sectorAround;
    sectorMap_.GetSectorAround(player, &sectorAround);
    for (int iCnt = 0; iCnt < sectorAround._count; iCnt++)
    {
        sectorMap_.LockSector_Shared(sectorAround._around[iCnt]._xPos, sectorAround._around[iCnt]._yPos);
        auto sectorSet = &sectorMap_.sectorMap_[sectorAround._around[iCnt]._yPos][sectorAround._around[iCnt]._xPos];
        for (auto setIter = sectorSet->begin(); setIter != sectorSet->end(); ++setIter)
        {

            NetSessionID ID;
            ID.total_ = (*setIter).first;

            if(ID.element_.unique_ != player->GetSessionID().element_.unique_)
               set->Enqueue(ID);
        }
        sectorMap_.UnlockSector_Shared(sectorAround._around[iCnt]._xPos, sectorAround._around[iCnt]._yPos);
    }

}

bool server_baby::MultiChatServer::DisconnectDuplicatedPlayer(INT64 accountNum)
{
    NetSessionID duplicatedNetSessionID;
    if (!onlineMap_.Find_SharedLock(&duplicatedNetSessionID, accountNum))
    {
        SystemLogger::GetInstance()->LogText(L"ChattingServer",
            LEVEL_ERROR, L"Login - Duplicated, Not Found in Online Map, %d", accountNum);

        return false;
    }

    return Disconnect(duplicatedNetSessionID);
}

bool server_baby::MultiChatServer::ReleasePlayer(NetSessionID ID)
{
    PlayerWithLock* player = nullptr;
    if (!connectedMap_.Find_SharedLock(&player, ID.total_))
        return false;

    SectorPos curSector = player->GetCurSector();
    INT64 accountNum = player->GetAccountNumber();

    if (accountNum != NULL)
    {
        if (curSector._xPos != X_DEFAULT)
        {
            sectorMap_.LockSector_Exclusive(curSector._xPos, curSector._yPos);
            sectorMap_.RemoveFromSector(curSector._xPos, curSector._yPos, accountNum);
            sectorMap_.UnlockSector_Exclusive(curSector._xPos, curSector._yPos);
        }

        onlineMap_.Release_ExclusiveLock(accountNum);
    }

    connectedMap_.Release_ExclusiveLock(ID.total_);
    PlayerWithLock::Free(player);

    return true;
}
