#include "DBConnector.h"
#include "SystemLogger.h"
#include "Crash.h"
#include <stdio.h>
#include <strsafe.h>

MyNetwork::DBConnector::DBConnector(const WCHAR* szDBIP, const WCHAR* szUser, const WCHAR* szPassword, const WCHAR* szDBName, const int iDBPort)
    :  DBPort_(NULL), infoArrayIndex_(NULL), TLSIndex_(NULL)
{
    wcscpy_s(DBIP_, szDBIP);
    wcscpy_s(DBUser_, szUser);
    wcscpy_s(DBPassword_, szPassword);
    wcscpy_s(DBName_, szDBName);
    DBPort_ = iDBPort;

    InitializeSRWLock(&initLock_);

    SetTLSIndex(&TLSIndex_);
}

MyNetwork::DBConnector::~DBConnector()
{
    for (int i = 0; i < infoArrayIndex_; i++)
    {
        mysql_close(&infoArray_[i]->MySQL_);
        delete infoArray_[i];
    }

    TlsFree(TLSIndex_);
}

bool MyNetwork::DBConnector::Connect(void)
{
    char DBIP[16] = { 0 };
    char DBUser[64] = { 0 };
    char DBPassword[64] = {0};
    char DBName[64] = {0};
    
    size_t size = 0;
    wcstombs_s(&size, DBIP, DBIP_, sizeof(DBIP));
    wcstombs_s(&size, DBUser, DBUser_, sizeof(DBUser));
    wcstombs_s(&size, DBPassword, DBPassword_, sizeof(DBPassword));
    wcstombs_s(&size, DBName, DBName_, sizeof(DBName));

    ConnectorInfo* info = GetTLSInfo();

    info->pMySQL_ = mysql_real_connect(
        &info->MySQL_,
        DBIP,
        DBUser,
        DBPassword,
        DBName,
        DBPort_,
        (char*)NULL,
        0);

    if (!info->pMySQL_)
    {
        SaveLastError();
        SystemLogger::GetInstance()->LogText(L"DBConnector", LEVEL_ERROR, L"DB Connect Failed!!");       
        CrashDump::Crash();
        return false;
    }
    else
        return true;
}

bool MyNetwork::DBConnector::Disconnect(void)
{
    for (int i = 0; i < infoArrayIndex_; i++)
    {
        mysql_close(&infoArray_[i]->MySQL_); 
    }

    return true;
}

bool MyNetwork::DBConnector::Query(const WCHAR* stringFormat, ...)
{
    ConnectorInfo* info = GetTLSInfo();

    va_list va;
    va_start(va, stringFormat);
    HRESULT hResult = StringCchVPrintfW(info->Query_,
        eQUERY_MAX_LEN,
        stringFormat,
        va);
    va_end(va);

    if (FAILED(hResult))
    {
      
        SystemLogger::GetInstance()->LogText(L"DBConnector", LEVEL_ERROR, L"Query Too Long... : %s", info->Query_);
        CrashDump::Crash();

    }

    size_t size = 0;
    wcstombs_s(&size, info->QueryUTF8_, info->Query_, eQUERY_MAX_LEN);

    ULONGLONG start = GetTickCount64();
    int query_stat = mysql_query(info->pMySQL_, info->QueryUTF8_);

    ULONGLONG end = GetTickCount64();
    ULONGLONG time = end - start;
    if (time > 850)
        SystemLogger::GetInstance()->LogText(L"DBConnector", LEVEL_ERROR, L"Query Time Over!!! [%d ms] : %s", time, info->Query_);

    info->SQLResult_ = mysql_store_result(info->pMySQL_);
    return true;
}

bool MyNetwork::DBConnector::Query_Save(const WCHAR* stringFormat, ...)
{
    ConnectorInfo* info = GetTLSInfo();

    va_list va;
    va_start(va, stringFormat);
    HRESULT hResult = StringCchVPrintfW(info->Query_,
        eQUERY_MAX_LEN,
        stringFormat,
        va);
    va_end(va);

    if (FAILED(hResult))
    {
        SystemLogger::GetInstance()->LogText(L"DBConnector", LEVEL_ERROR, L"Query Too Long... : %s", info->Query_);
        CrashDump::Crash();
    }

    size_t size = 0;
    wcstombs_s(&size, info->QueryUTF8_, info->Query_, eQUERY_MAX_LEN);

    ULONGLONG start = GetTickCount64();
    int query_stat = mysql_query(info->pMySQL_, info->QueryUTF8_);
    if (query_stat != 0)
    {
        SaveLastError();
        SystemLogger::GetInstance()->LogText(L"DBConnector", LEVEL_ERROR, L"Query Failed!! : %s", info->Query_);
        CrashDump::Crash();
    }
    ULONGLONG end = GetTickCount64();

    ULONGLONG time = end - start;
    if (time > 850)
        SystemLogger::GetInstance()->LogText(L"DBConnector", LEVEL_ERROR, L"Query Time Over!!! [%d ms] : %s", time, info->Query_);

    info->SQLResult_ = mysql_store_result(info->pMySQL_);
    FreeResult();

    return true;
}

MYSQL_ROW MyNetwork::DBConnector::FetchRow(void)
{
    ConnectorInfo* info = GetTLSInfo();
    if(info->SQLResult_)
        return mysql_fetch_row(info->SQLResult_);
    return nullptr;
}

void MyNetwork::DBConnector::FreeResult(void)
{
    ConnectorInfo* info = GetTLSInfo();
    mysql_free_result(info->SQLResult_);
}

int MyNetwork::DBConnector::GetLastError(void)
{
    ConnectorInfo* info = GetTLSInfo();
    return info->lastError_;
}

WCHAR* MyNetwork::DBConnector::GetLastErrorMsg(void)
{
    ConnectorInfo* info = GetTLSInfo();
    return info->lastErrorMsg_;
};

void MyNetwork::DBConnector::SaveLastError(void)
{
    ConnectorInfo* info = GetTLSInfo();
    char errorCode[128] = { 0 };
    strcpy_s(errorCode, mysql_error(&info->MySQL_));

    size_t size = 0;
    mbstowcs_s(&size, info->lastErrorMsg_, errorCode, 128);

    SystemLogger::GetInstance()->LogText(L"DBConnector", LEVEL_ERROR, L"Last Error : %s", info->lastErrorMsg_);
}

void MyNetwork::DBConnector::SetTLSIndex(DWORD* index)
{
    DWORD tempIndex = TlsAlloc();
    if (tempIndex == TLS_OUT_OF_INDEXES)
    {
        //TLS Alloc 함수가 비트 플래그 배열로부터 프리 상태인 플래그를 찾지 못했다.        
        SystemLogger::GetInstance()->LogText(L"DBConnector", LEVEL_ERROR, L"SetTLSIndex - Out of Index");
        CrashDump::Crash();
    }

    *index = tempIndex;
}

void MyNetwork::DBConnector::SetTLSInfo(ConnectorInfo* info)
{
    BOOL retval = TlsSetValue(TLSIndex_, info);

    if (retval == false)
    {
        SystemLogger::GetInstance()->LogText(L"DBConnector", LEVEL_ERROR, L"TLSSetValue Error Code : %d", GetLastError());
        CrashDump::Crash();
    }
}

MyNetwork::DBConnector::ConnectorInfo* MyNetwork::DBConnector::GetTLSInfo()
{
    ConnectorInfo* info = (ConnectorInfo*)TlsGetValue(TLSIndex_);
    if (info != nullptr)
        return info;

    ConnectorInfo* newInfo = new ConnectorInfo();
    AcquireSRWLockExclusive(&initLock_);
    newInfo->pMySQL_ = mysql_init(&newInfo->MySQL_);
    if (!newInfo->pMySQL_)
    {
        SystemLogger::GetInstance()->LogText(L"DBConnector", LEVEL_ERROR, L"Mysql_init failed, insufficient memory, Error Code : %d", GetLastError());
        CrashDump::Crash();
    }
    

    short tempIndex = 0;
    short newIndex = 0;
    do {
        tempIndex = infoArrayIndex_;
        newIndex = tempIndex + 1;

    } while (InterlockedCompareExchange16(
        (SHORT*)&infoArrayIndex_,
        newIndex,
        tempIndex) != tempIndex);

    infoArray_[tempIndex] = newInfo;

    SetTLSInfo(newInfo);
    Connect();

    ReleaseSRWLockExclusive(&initLock_);
    return newInfo;
}

