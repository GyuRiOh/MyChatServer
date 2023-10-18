// MyChatServer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "ChatServer/Single/ChatServer.h"
#include "NetRoot/Common/Parser.h"
#include "NetRoot/Common/Crash.h"
#include "NetRoot/Common/SystemLogger.h"

using namespace MyNetwork;
using namespace std;

int main()
{

	int port = 0;
	int wait = 0;
	int run = 0;
	int nagle = false;
	int timeout = 0;
	int sendIOPending = false;
	int logicMultithread = true;

	if (!Parser::GetInstance()->GetValue("ServerPort", (int*)&port))
		CrashDump::Crash();

	SystemLogger::GetInstance()->Console(L"NetServer", LEVEL_DEBUG, L"Port : %d", port);

	Parser::GetInstance()->GetValue("WaitThread", (int*)&wait);
	Parser::GetInstance()->GetValue("RunThread", (int*)&run);
	Parser::GetInstance()->GetValue("Nagle", (int*)&nagle);
	Parser::GetInstance()->GetValue("Timeout", (int*)&timeout);
	Parser::GetInstance()->GetValue("SendIOPending", (int*)&sendIOPending);
	Parser::GetInstance()->GetValue("LogicMultithread", (int*)&logicMultithread);

	ChatServer server;

	server.Start(
		port,
		wait,
		run,
		true,
		timeout,
		false,
		false);


	Sleep(INFINITE);

	return 1;
}
