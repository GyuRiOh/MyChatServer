
#include "ChatServer.h"
#include "../../CommonProtocol.h"
#include "../../MonitorProtocol.h"
#include "../../NetRoot/Common/PDHMonitor.h"
#include "../../NetRoot/Common/SystemLogger.h"
#include "ChatServer_CS_Stub.h"
#include "ChatServer_SC_Proxy.h"
#include "../../NetRoot/Common/SizedMemoryPool.h"
#include "../../NetRoot/Common/Parser.h"
#include <cpp_redis/cpp_redis>

using namespace std;
using namespace server_baby;

server_baby::ChatServer::ChatServer() 
    : updateThreadID_(NULL), isUpdateThreadRunning_(true), updateTPS_(0), updateEvent_(CreateEvent(NULL, FALSE, NULL, NULL)),
        authEvent_(CreateEvent(NULL, FALSE, NULL, NULL)), updateBlockedTime_(NULL),
        updateThread_(INVALID_HANDLE_VALUE), authThread_(INVALID_HANDLE_VALUE), authThreadID_(NULL)
{

    RegisterStub(new ChatServer_CS_Stub(this)); 
    proxy_ = new ChatServer_SC_Proxy(this);

    updateThread_ = (HANDLE)_beginthreadex(
        NULL,
        0,
        (_beginthreadex_proc_type)&UpdateThread,
        (LPVOID)this,
        0,
        (unsigned int*)&updateThreadID_);

    if (!updateThread_)
        ErrorQuit(L"UpdateThread Start Failed");

    DWORD dwThreadPri = GetThreadPriority(updateThread_);
    SystemLogger::GetInstance()->Console(L"Priority", LEVEL_DEBUG, L"Logic Thread - 0x%x", dwThreadPri);
    SetThreadPriority(updateThread_, THREAD_PRIORITY_ABOVE_NORMAL);

    dwThreadPri = GetThreadPriority(updateThread_);
    SystemLogger::GetInstance()->Console(L"Priority", LEVEL_DEBUG, L"Logic Thread - 0x%x", dwThreadPri);

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

server_baby::ChatServer::~ChatServer()
{
    delete proxy_;
}

void server_baby::ChatServer::OnError(int errCode, WCHAR*)
{

}

void server_baby::ChatServer::OnMonitor(const MonitoringInfo* const info)
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
    *packet << static_cast<int>(connectedMap_.Size());
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_CHAT_PLAYER);
    *packet << static_cast<int>(onlineMap_.Size());
    *packet << static_cast<int>(timer);

    lanClient_.SendPacket(packet);
    packet->Clear();

    *packet << static_cast<WORD>(en_PACKET_SS_MONITOR_DATA_UPDATE);
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS);
    *packet << static_cast<int>(updateTPS_);
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
    *packet << static_cast<BYTE>(dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL);
    *packet << static_cast<int>(jobQ_PacketQ_.GetSize());
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

    SystemLogger::GetInstance()->Console(L"ChatServer", LEVEL_DEBUG, L"Recv TPS : %d", info->recvTPS_);
    SystemLogger::GetInstance()->Console(L"ChatServer", LEVEL_DEBUG, L"Send TPS : %d", info->sendTPS_);
    SystemLogger::GetInstance()->Console(L"ChatServer", LEVEL_DEBUG, L"Accept TPS : %d", info->acceptTPS_);

    updateTPS_ = 0;
    updateBlockedTime_ = 0;

}

DWORD __stdcall server_baby::ChatServer::UpdateThread(LPVOID arg)
{
    ChatServer* server = reinterpret_cast<ChatServer*>(arg);
    while (server->isServerCreated())
    {
        ULONGLONG timeStart = GetTickCount64();
        WaitForSingleObject(server->updateEvent_, INFINITE);
        server->updateBlockedTime_ += (GetTickCount64() - timeStart);
        server->MyLogic_PacketProc();
    }
    return 0;
}

DWORD __stdcall server_baby::ChatServer::AuthThread(LPVOID arg)
{
    //Redis 스레드로 활용
    WORD version = MAKEWORD(2, 2);
    WSADATA data;
    WSAStartup(version, &data);

    cpp_redis::client client;
    client.connect();

    ChatServer* server = reinterpret_cast<ChatServer*>(arg);
    while (server->isServerCreated())
    {
        WaitForSingleObject(server->authEvent_, INFINITE);
        while (!server->jobQ_Redis_.isEmpty())
        {
            RedisJob* redisJob = nullptr;
            server->jobQ_Redis_.Dequeue(&redisJob);

            char accountNum[32] = { 0 };
            _itoa(redisJob->acoountNum, accountNum, 10);

            bool redisResult = false;

            client.get(accountNum, [&redisResult, redisJob](cpp_redis::reply& reply) {
                if (reply.as_string().compare(redisJob->sessionKey) != 0)
                    cout << "Redis Not Equal" << endl;
                else
                    redisResult = true;
            
            });

            client.sync_commit();

            server->proxy_->ResLogin(
                redisResult,
                redisJob->acoountNum,
                redisJob->sessionID
            );

            SizedMemoryPool::GetInstance()->Free(redisJob);
        }
    }
    
    return 0;
}

void server_baby::ChatServer::MyLogic_PacketProc()
{
    while (!jobQ_PacketQ_.isEmpty())
    {
        NetPacketSet* packetQ = nullptr; 
        jobQ_PacketQ_.Dequeue(&packetQ);

        PacketProc(packetQ);        
    }

}

void server_baby::ChatServer::TrySendPacket_Sector(Player* player, NetPacket* packet)
{
    short x = player->curSector_._xPos;
    short y = player->curSector_._yPos;

    auto iter = sectorMap_.sectorMap_[y][x].begin();
    for (; iter != sectorMap_.sectorMap_[y][x].end();)
    {
        Player* receiver = (*iter).second;
        AsyncSendPacket(receiver->GetSessionID(), packet);
    }

}

void server_baby::ChatServer::TrySendPacket_SectorAround(Player* player, NetPacket* packet)
{

    NetSessionIDSet* idSet = NetSessionIDSet::Alloc();
    GetNetSessionIDSet_Sector(player, idSet);
    if (idSet->GetSize() == 0)
    {
        NetSessionIDSet::Free(idSet);
        return;
    }

    AsyncSendPacket(idSet, packet);

}

void server_baby::ChatServer::GetNetSessionIDSet_Sector(Player* player, NetSessionIDSet* set)
{
    SectorAround sectorAround;
    sectorMap_.GetSectorAround(player, &sectorAround);
    for (int iCnt = 0; iCnt < sectorAround._count; iCnt++)
    {
        auto sectorSet = &sectorMap_.sectorMap_[sectorAround._around[iCnt]._yPos][sectorAround._around[iCnt]._xPos];
        for (auto setIter = sectorSet->begin(); setIter != sectorSet->end(); ++setIter)
        {
            NetSessionID ID;
            ID.total_ = (*setIter).first;
            set->Enqueue(ID);
        }
    }

}
