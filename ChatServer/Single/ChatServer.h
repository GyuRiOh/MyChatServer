#pragma once
#include "../../NetRoot/NetServer/NetServer.h"
#include "../../NetRoot/NetServer/NetPacketSet.h"
#include "../../Player/PlayerMap.h"
#include "../../Sector/SectorMap.h"
#include "../../Player/Player.h"
#include "../../PacketStruct.h"
#include "../../NetRoot/Common/JobQueue.h"
#include "ChatClient.h"
#include "../../RedisJob.h"


using namespace std;

namespace server_baby
{
    class ChatServer_SC_Proxy;

    class ChatServer final : public NetRoot
    {
        enum Setting
        {
            PACKETQ_Q_CODE = 0x1010,
            ACCOUNT_NUMBER_MAX = 95000
        };
    public:
        explicit ChatServer();
        ~ChatServer();

        long GetJobQSize();
        int GetOnlinePlayerSize();
        int GetConnectedPlayerSize();
        unsigned long long GetUpdateTPS();

        bool DisconnectAbnormalPlayer(NetSessionID ID);
        bool DisconnectDuplicatedPlayer(INT64 accountNum);
        void TrySendPacket_SectorAround(Player* player, NetPacket* packet);
        void TrySendPacket_Sector(Player* player, NetPacket* packet);
        void RequireAuth(RedisJob* job);
        bool ReleasePlayer(NetSessionID ID);

    private:
        bool OnConnectionRequest(const SOCKADDR_IN* const addr) override; 
        void OnClientJoin(const NetSessionID NetSessionID) override {}
        void OnClientLeave(const NetSessionID NetSessionID) override {}
        void OnRecv(NetPacketSet* const packetList) override {}
        void OnSend(NetSessionID sessionID, int sendSize) override; 
        void OnWorkerThreadBegin() override; 
        void OnWorkerThreadEnd() override;
        void OnError(int errCode, WCHAR*) override;
        void OnMonitor(const MonitoringInfo* const info) override;
        void OnStart() override {}
        void OnStop() override {}

        static DWORD WINAPI UpdateThread(LPVOID arg);
        static DWORD WINAPI AuthThread(LPVOID arg);

        void MyLogic_PacketProc(); 
        void GetNetSessionIDSet_Sector(Player* player, NetSessionIDSet* set);

    public:
        UserMap<NetSessionID> onlineMap_;
        UserMap<Player*> connectedMap_;
        ChatServer_SC_Proxy* proxy_;

        SectorMap<Player*> sectorMap_;

        HANDLE updateEvent_;
        HANDLE authEvent_;

        HANDLE updateThread_;
        HANDLE authThread_;
        
        unsigned int updateTPS_;
        unsigned int updateThreadID_;
        unsigned int authThreadID_;
        
        ULONGLONG updateBlockedTime_;
        bool isUpdateThreadRunning_;

        LockFreeEnqJobQ<NetPacketSet*, PACKETQ_Q_CODE> jobQ_PacketQ_;
        LockFreeEnqJobQ<RedisJob*, PACKETQ_Q_CODE> jobQ_Redis_;

        ChatClient lanClient_;
    };


    inline long ChatServer::GetJobQSize()
    {
        return jobQ_PacketQ_.GetSize();
    }

    inline int ChatServer::GetOnlinePlayerSize()
    {
        return static_cast<int>(onlineMap_.Size());
    }

    inline int ChatServer::GetConnectedPlayerSize()
    {
        return static_cast<int>(connectedMap_.Size());
    }

    inline unsigned long long ChatServer::GetUpdateTPS()
    {
        return updateTPS_;
    }

    inline bool server_baby::ChatServer::DisconnectAbnormalPlayer(NetSessionID ID)
    {
        return Disconnect(ID);
    }

    inline bool server_baby::ChatServer::DisconnectDuplicatedPlayer(INT64 accountNum)
    {
        NetSessionID duplicatedNetSessionID;
        if (!onlineMap_.Find(&duplicatedNetSessionID, accountNum))
        {
            SystemLogger::GetInstance()->LogText(L"ChattingServer",
                LEVEL_ERROR, L"Login - Duplicated, Not Found in Online Map, %d", accountNum);

            return false;
        }

        return Disconnect(duplicatedNetSessionID);
    }

    inline bool server_baby::ChatServer::ReleasePlayer(NetSessionID ID)
    {
        Player* player = nullptr;
        if (!connectedMap_.Find(&player, ID.total_))
        {
            ErrorQuit(L"Connected Map Release Fail");
            return false;
        }

        sectorMap_.RemoveFromCurSector(player);
        connectedMap_.Release(ID.total_);
        onlineMap_.Release(player->GetAccountNumber());
        Player::Free(player);

        return true;
    }

    inline bool server_baby::ChatServer::OnConnectionRequest(const SOCKADDR_IN* const addr)
    {
        return true;
    }

    inline void server_baby::ChatServer::OnSend(NetSessionID NetSessionID, int sendSize)
    {

    }

    inline void server_baby::ChatServer::OnWorkerThreadBegin()
    {

    }

    inline void server_baby::ChatServer::OnWorkerThreadEnd()
    {

    }

    inline void server_baby::ChatServer::RequireAuth(RedisJob* job)
    {
        jobQ_Redis_.Enqueue(job);
        SetEvent(authEvent_);
    }

}
