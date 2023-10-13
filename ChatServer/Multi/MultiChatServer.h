#pragma once
#include "../../NetRoot/NetServer/NetServer.h"
#include "../../NetRoot/NetServer/NetPacketSet.h"
#include "../../Player/PlayerMapWithLock.h"
#include "../../Sector/SectorMapWithLock.h"
#include "../../Player/Player.h"
#include "../../PacketStruct.h"
#include "../../NetRoot/Common/JobQueue.h"
#include "MultiChatClient.h"
#include "../../RedisJob.h"


using namespace std;

namespace server_baby
{
    class MultiChatServer_SC_Proxy;

    class MultiChatServer final : public NetRoot
    {
        enum Setting
        {
            PACKETQ_Q_CODE = 0x1010,
            ACCOUNT_NUMBER_MAX = 95000
        };
    public:
        explicit MultiChatServer();
        ~MultiChatServer();

        int GetOnlinePlayerSize();
        int GetConnectedPlayerSize();

        bool DisconnectAbnormalPlayer(NetSessionID ID);
        bool DisconnectDuplicatedPlayer(INT64 accountNum);
        void TrySendPacket_SectorAround(PlayerWithLock* player, NetPacket* packet);
        void TrySendPacket_Sector(PlayerWithLock* player, NetPacket* packet);
        void RequireAuth(RedisJob* job);
        bool ReleasePlayer(NetSessionID ID);

        void ZeroTPS();
        void IncrementUpdateTPS();
        DWORD* GetUpdateTLS();
        unsigned long long GetUpdateTPS();
        DWORD* MakeUpdateTLS();

    private:
        //==========================
        //가상함수들
        //==========================
        bool OnConnectionRequest(const SOCKADDR_IN* const addr) override; //Accept 직후. return false시 클라이언트 거부, true시 접속 허용
        void OnSend(NetSessionID sessionID, int sendSize) override; //패킷 송신 완료 후
        void OnWorkerThreadBegin() override; //워커스레드 GQCS 하단에서 호출
        void OnWorkerThreadEnd() override; //워커스레드 1루프 종료 후
        void OnError(int errCode, WCHAR*) override;
        void OnMonitor(const MonitoringInfo* const info) override;

        static DWORD WINAPI AuthThread(LPVOID arg);

        void GetNetSessionIDSet_Sector(PlayerWithLock* player, NetSessionIDSet* set);

    public:
        UserMapWithLock<NetSessionID> onlineMap_;
        UserMapWithLock<PlayerWithLock*> connectedMap_;
        SectorMapWithLock<PlayerWithLock*> sectorMap_;

        MultiChatServer_SC_Proxy* proxy_;

        HANDLE authEvent_;
        HANDLE authThread_;
        
        unsigned int authThreadID_;
        
        LockFreeEnqJobQ<RedisJob*, PACKETQ_Q_CODE> jobQ_Redis_;

        vector<DWORD*> TPSArray_;
        DWORD TPS_TLS_ = NULL;
        SHORT TPSIndex_ = 0;


        MultiChatClient lanClient_;
    };

    inline void server_baby::MultiChatServer::OnSend(NetSessionID NetSessionID, int sendSize)
    {

    }

    inline void server_baby::MultiChatServer::OnWorkerThreadBegin()
    {

    }

    inline void server_baby::MultiChatServer::OnWorkerThreadEnd()
    {

    }

    inline int MultiChatServer::GetOnlinePlayerSize()
    {
        return static_cast<int>(onlineMap_.Size_SharedLock());
    }

    inline int MultiChatServer::GetConnectedPlayerSize()
    {
        return static_cast<int>(connectedMap_.Size_SharedLock());
    }

    inline unsigned long long MultiChatServer::GetUpdateTPS()
    {
        DWORD tps = 0;
        for (int i = 0; i < TPSIndex_; i++)
        {
            tps += *(TPSArray_[i]);
        }

        return tps;
    }

    inline bool server_baby::MultiChatServer::DisconnectAbnormalPlayer(NetSessionID ID)
    {
        return Disconnect(ID);
    }

    inline bool server_baby::MultiChatServer::OnConnectionRequest(const SOCKADDR_IN* const addr)
    {
        return true;
    }

    inline void server_baby::MultiChatServer::RequireAuth(RedisJob* job)
    {
        jobQ_Redis_.Enqueue(job);
        SetEvent(authEvent_);
    }

    inline void MultiChatServer::ZeroTPS()
    {

        for (int i = 0; i < TPSIndex_; i++)
        {
            *TPSArray_[i] = 0;
        }
    }

    inline void MultiChatServer::IncrementUpdateTPS()
    {
        DWORD* tps = GetUpdateTLS();
        (*tps)++;
    }

    inline DWORD* MultiChatServer::MakeUpdateTLS()
    {
        DWORD* newTps = new DWORD();
        short tempIndex = 0;
        short newIndex = 0;
        do {
            tempIndex = TPSIndex_;
            newIndex = tempIndex + 1;

        } while (InterlockedCompareExchange16(
            (SHORT*)&TPSIndex_,
            newIndex,
            tempIndex) != tempIndex);

        TPSArray_[tempIndex] = newTps;

        if (TlsSetValue(TPS_TLS_, newTps) == false)
            ErrorQuit(L"TlsSetValue");

        return newTps;
    }

    inline DWORD* MultiChatServer::GetUpdateTLS()
    {
        DWORD* tps = (DWORD*)TlsGetValue(TPS_TLS_);
        if (tps != nullptr)
            return tps;

        return MakeUpdateTLS();
    }
}
