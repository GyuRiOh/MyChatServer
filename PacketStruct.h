#pragma once
#define _WINSOCKAPI_
#include <Windows.h>

#define MESSAGE_MAX 512
#define SECTOR_X_MAX 100
#define SECTOR_Y_MAX 50
#define SECTOR_MONITOR_X_MAX 50
#define SECTOR_MONITOR_Y_MAX 25
#define SECTOR_MONITOR 2

#pragma pack(push, 1)

struct PACKET_CS_CHAT_REQ_LOGIN
{
	WORD type_;
	INT64 accountNum_;
	WCHAR ID_[20] = { 0 };
	WCHAR nickName_[20] = { 0 };
	char sessionKey_[64] = { 0 };
};

struct PACKET_CS_CHAT_RES_LOGIN
{
	WORD type_;
	BYTE status_; // 0 실패, 1 성공
	INT64 accountNum_;
};

struct PACKET_CS_CHAT_REQ_SECTOR_MOVE
{
	WORD type_;
	INT64 accountNum_;
	WORD sectorX_;
	WORD sectorY_;
};

struct PACKET_CS_CHAT_RES_SECTOR_MOVE
{
	WORD type_;
	INT64 accountNum_;
	WORD sectorX_;
	WORD sectorY_;
};

struct PACKET_CS_CHAT_REQ_MESSAGE
{
	WORD type_;
	INT64 accountNum_;
	WORD messageLen_;
	WCHAR message[MESSAGE_MAX];
};

struct PACKET_CS_CHAT_RES_MESSAGE
{
	WORD type_;
	INT64 accountNum_;
	WCHAR ID_[20];
	WCHAR nickName_[20];
	WORD messageLen_;
	WCHAR message[MESSAGE_MAX];
};

struct SECTOR_MONITOR_CS_CHAT_REQ_UPDATE
{
	WORD oldSectorX;
	WORD oldSectorY;
	WORD curSectorX;
	WORD curSectorY;
};

struct PACKET_CS_GAME_REQ_ECHO
{
	WORD type;
	INT64 AccountNo;
	LONGLONG SendTick;
};
#pragma pack(pop)