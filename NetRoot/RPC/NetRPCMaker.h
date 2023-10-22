
#ifndef __NET__RPC__MAKER__
#define __NET__RPC__MAKER__

#include <Windows.h>
#include "Crash.h"
#include "SystemLogger.h"
#include "Parser.h"

namespace server_baby
{
	class NetRPCMaker
	{	
		struct ProcedureInfo
		{
			struct Param
			{
				char type[32] = { 0 };
				char name[128] = { 0 };
				char charaLen[32] = { 0 };
			};

			char functionName_[256] = { 0 };
			Param param_[32] = { 0 };
			int paramNum_ = 0;
		};

	public:
		explicit NetRPCMaker() : procCount_SC_(NULL), procCount_CS_(NULL)
		{
			//파서 싱글톤 좀 오버엔지니어링
			//SC Protocol 파싱
			Parser::GetInstance()->LoadFile("Net_RPC_SC_Protocol.ini");
			Parser::GetInstance()->GetString("ServerName", serverName_);
			Parser::GetInstance()->GetString("ClientName", clientName_);

			char* buffer = Parser::GetInstance()->GetBuffer();

			char* tok = strtok_s(buffer, "$", &buffer);
			tok = strtok_s(buffer, "$", &buffer);
			while (tok != NULL)
			{
				char* functionName = strtok_s(tok, "(", &tok);
				strcpy_s(procInfo_ServerClient[procCount_SC_].functionName_, functionName);
				
				char* paramType = nullptr;
				char* paramName = nullptr;
				char* paramBuf = nullptr;
				
				int paramNum = 0;
				while (*tok != NULL)
				{
					paramType = strtok_s(tok, "[", &tok);
					paramType = strtok_s(paramType,"]", &paramName);
					strcpy_s(procInfo_ServerClient[procCount_SC_].param_[paramNum].type, paramType);

					paramName = strtok_s(paramName, ",", &paramBuf);
					strcpy_s(procInfo_ServerClient[procCount_SC_].param_[paramNum].name, paramName);
					paramNum++;
				}

				procInfo_ServerClient[procCount_SC_].paramNum_ = paramNum;

				procCount_SC_++;
				tok = strtok_s(buffer, "$", &buffer);
			}	

			//CS Protocol 파싱
			Parser::GetInstance()->LoadFile("Net_RPC_CS_Protocol.ini");
			char newServer[512] = { 0 };
			char newClient[512] = { 0 };
			Parser::GetInstance()->GetString("ServerName", newServer);
			Parser::GetInstance()->GetString("ClientName", newClient);

			if (strcmp(newServer, serverName_) != 0)
			{
				SystemLogger::GetInstance()->LogText(L"NetRPCMaker", LEVEL_ERROR, L"SC_CS Protocol Name - Not Same");
				CrashDump::Crash();
			}

			if (strcmp(newClient, clientName_) != 0)
			{
				SystemLogger::GetInstance()->LogText(L"NetRPCMaker", LEVEL_ERROR, L"SC_CS Protocol Name - Not Same");
				CrashDump::Crash();
			}


			buffer = Parser::GetInstance()->GetBuffer();
			tok = strtok_s(buffer, "$", &buffer);
			tok = strtok_s(buffer, "$", &buffer);
			while (tok != NULL)
			{
				char* functionName = strtok_s(tok, "(", &tok);
				strcpy_s(procInfo_ClientServer[procCount_CS_].functionName_, functionName);

				char* paramType = nullptr;
				char* paramName = nullptr;
				char* paramBuf = nullptr;

				int paramNum = 0;
				while (*tok != NULL)
				{
					paramType = strtok_s(tok, "[", &tok);
					paramType = strtok_s(paramType, "]", &paramName);
					strcpy_s(procInfo_ClientServer[procCount_CS_].param_[paramNum].type, paramType);

					paramName = strtok_s(paramName, ",", &paramBuf);
					strcpy_s(procInfo_ClientServer[procCount_CS_].param_[paramNum].name, paramName);
					paramNum++;
				}

				procInfo_ClientServer[procCount_CS_].paramNum_ = paramNum;

				procCount_CS_++;
				tok = strtok_s(buffer, "$", &buffer);
			}

		}

		~NetRPCMaker()
		{

		}

		void CreateCode()
		{
			CreateRequestCode();
			CreateResponseCode();
		}

	private:
		void CreateRequestCode()
		{
			FILE* stream = nullptr;
			char fileName[128] = { 0 };
			sprintf_s(fileName, "%s_SC_Proxy.h", serverName_);
			if (fopen_s(&stream, fileName, "w") == 0)
			{
				//헤더
				fprintf(stream, "#pragma once\n");
				fprintf(stream, "#include <Windows.h>\n");
				fprintf(stream, "#include \"NetRoot/NetServer/NetPacket.h\"\n");
				fprintf(stream, "#include \"NetRoot/NetServer/NetSessionID.h\"\n");
				fprintf(stream, "#include \"%s.h\"\n", serverName_);

				fprintf(stream, "\n");
				fprintf(stream, "namespace server_baby\n");
				fprintf(stream, "{\n");

				//생성자
				fprintf(stream, "\tclass %s_SC_Proxy\n", serverName_);
				fprintf(stream, "\t{\n");
				fprintf(stream, "\tpublic:\n");
				fprintf(stream, "\t\texplicit %s_SC_Proxy(%s* server) : server_(server) {}\n", serverName_, serverName_);
				fprintf(stream, "\n");

				for (int i = 0; i < procCount_SC_; i++)
				{
					//개별 함수
					fprintf(stream, "\t\tvoid %s(", procInfo_ServerClient[i].functionName_);

					
					for (int j = 0; j < procInfo_ServerClient[i].paramNum_; j++)
					{
						char* typeBuf = nullptr;
						char* type = strtok_s(procInfo_ServerClient[i].param_[j].type, "=", &typeBuf);
						fprintf(stream, "%s %s, ",
							type, procInfo_ServerClient[i].param_[j].name);

						if (*typeBuf != NULL)
							strcpy_s(procInfo_ServerClient[i].param_[j].charaLen, typeBuf);
					}

					fprintf(stream, "NetSessionID sessionID)\n");
					fprintf(stream, "\t\t{\n");
					fprintf(stream, "\t\t\tNetPacket* msg = NetPacket::Alloc();\n");
					fprintf(stream, "\n");
					fprintf(stream, "\t\t\t*msg << (unsigned short)%d;\n", i);


					for (int j = 0; j < procInfo_ServerClient[i].paramNum_; j++)
					{
						if (*procInfo_ServerClient[i].param_[j].charaLen != NULL)
						{
							fprintf(stream, "\t\t\tmsg->EnqData((char*)%s, %s);\n", procInfo_ServerClient[i].param_[j].name,
								procInfo_ServerClient[i].param_[j].charaLen);
						}
						else
							fprintf(stream, "\t\t\t*msg << %s;\n", procInfo_ServerClient[i].param_[j].name);

					}


					fprintf(stream, "\n");

					fprintf(stream, "\t\t\tserver_->SendPacket_Async(sessionID, msg);\n");
					fprintf(stream, "\t\t\tNetPacket::Free(msg);\n");

					fprintf(stream, "\t\t}\n");
					fprintf(stream, "\n");
				}

				//멤버
				fprintf(stream, "\tprivate:\n");
				fprintf(stream, "\t\t%s* server_;\n", serverName_);
				fprintf(stream, "\t};\n");
				fprintf(stream, "}\n");


				fclose(stream);
			}
			else
			{
				SystemLogger::GetInstance()->LogText(L"CreateRequestCode", LEVEL_ERROR, L"fopen failed");
				CrashDump::Crash();
			}

			sprintf_s(fileName, "%s_CS_Proxy.h", clientName_);
			if (fopen_s(&stream, fileName, "w") == 0)
			{
				//헤더
				fprintf(stream, "#pragma once\n");
				fprintf(stream, "#include <Windows.h>\n");
				fprintf(stream, "#include \"NetRoot/NetServer/NetPacket.h\"\n");
				fprintf(stream, "#include \"%s.h\"\n", clientName_);

				fprintf(stream, "\n");
				fprintf(stream, "namespace server_baby\n");
				fprintf(stream, "{\n");

				//생성자
				fprintf(stream, "\tclass %s_CS_Proxy\n", clientName_);
				fprintf(stream, "\t{\n");
				fprintf(stream, "\tpublic:\n");
				fprintf(stream, "\t\texplicit %s_CS_Proxy(%s* client) : client_(client) {}\n", clientName_, clientName_);
				fprintf(stream, "\n");

				for (int i = 0; i < procCount_CS_; i++)
				{
					//개별 함수
					fprintf(stream, "\t\tvoid %s(", procInfo_ClientServer[i].functionName_);


					for (int j = 0; j < procInfo_ClientServer[i].paramNum_; j++)
					{
						char* typeBuf = nullptr;
						char* type = strtok_s(procInfo_ClientServer[i].param_[j].type, "=", &typeBuf);
						fprintf(stream, "%s %s, ",
							type, procInfo_ClientServer[i].param_[j].name);

						if (*typeBuf != NULL)
							strcpy_s(procInfo_ClientServer[i].param_[j].charaLen, typeBuf);
					}

					fprintf(stream, "NetSessionID sessionID)\n");
					fprintf(stream, "\t\t{\n");
					fprintf(stream, "\t\t\tNetPacket* msg = NetPacket::Alloc();\n");
					fprintf(stream, "\n");
					fprintf(stream, "\t\t\t*msg << (unsigned short)%d;\n", i);


					for (int j = 0; j < procInfo_ClientServer[i].paramNum_; j++)
					{
						if (*procInfo_ClientServer[i].param_[j].charaLen != NULL)
						{
							fprintf(stream, "\t\t\tmsg->EnqData((char*)%s, %s);\n", procInfo_ClientServer[i].param_[j].name,
								procInfo_ClientServer[i].param_[j].charaLen);
						}
						else
							fprintf(stream, "\t\t\t*msg << %s;\n", procInfo_ClientServer[i].param_[j].name);

					}


					fprintf(stream, "\n");

					fprintf(stream, "\t\t\tclient_->SendPacket(msg);\n");
					fprintf(stream, "\t\t\tNetPacket::Free(msg);\n");

					fprintf(stream, "\t\t}\n");
					fprintf(stream, "\n");
				}

				//멤버
				fprintf(stream, "\tprivate:\n");
				fprintf(stream, "\t\t%s* client_;\n", clientName_);
				fprintf(stream, "\t};\n");
				fprintf(stream, "}\n");


				fclose(stream);
			}
			else
			{
				SystemLogger::GetInstance()->LogText(L"CreateRequestCode", LEVEL_ERROR, L"fopen failed");
				CrashDump::Crash();
			}
		}

		void CreateResponseCode()
		{
			FILE* stream = nullptr;
			char fileName[128] = { 0 };
			sprintf_s(fileName, "%s_CS_Stub.h", serverName_);
			if (fopen_s(&stream, fileName, "w+") == 0)
			{
				//헤더
				fprintf(stream, "#pragma once\n");
				fprintf(stream, "#include <Windows.h>\n");
				fprintf(stream, "#include \"NetRoot/NetServer/NetSessionID.h\"\n");
				fprintf(stream, "#include \"NetRoot/NetServer/NetPacketSet.h\"\n");
				fprintf(stream, "#include \"NetRoot/Common/RPCBuffer.h\"\n");
				fprintf(stream, "#include \"%s_SC_Proxy.h\"\n", serverName_);
				fprintf(stream, "#include \"%s.h\"\n", serverName_);

				fprintf(stream, "\n");
				fprintf(stream, "namespace server_baby\n");
				fprintf(stream, "{\n");

				//생성자
				fprintf(stream, "\tclass %s_CS_Stub\n", serverName_);
				fprintf(stream, "\t{\n");
				fprintf(stream, "\tpublic:\n");
				fprintf(stream, "\t\texplicit %s_CS_Stub(%s* server) : server_(server) {}\n", serverName_, serverName_);
				fprintf(stream, "\n");
				
				//msgPack Proc
				fprintf(stream, "\t\tbool PacketProc(NetPacketSet* msgPack)\n");
				fprintf(stream, "\t\t{\n");
				fprintf(stream, "\t\t\tswitch (msgPack->GetType())\n");
				fprintf(stream, "\t\t\t{\n");
				fprintf(stream, "\t\t\tcase eNET_RECVED_PACKET_SET:\n");
				fprintf(stream, "\t\t\t{\n");
				fprintf(stream, "\t\t\t\twhile (msgPack->GetSize() > 0)\n");
				fprintf(stream, "\t\t\t\t{\n");
				fprintf(stream, "\t\t\t\t\tNetDummyPacket* packet = nullptr;\n");
				fprintf(stream, "\t\t\t\t\tif (msgPack->Dequeue(&packet) == false)\n");
				fprintf(stream, "\t\t\t\t\t\tCrashDump::Crash();\n");
				fprintf(stream, "\n");
				fprintf(stream, "\t\t\t\t\tif (!PacketProc(msgPack->GetSessionID(), packet))\n");
				fprintf(stream, "\t\t\t\t\t\treturn false;\n");
				fprintf(stream, "\t\t\t\t}\n");
				fprintf(stream, "\t\t\t\tbreak;\n");
				fprintf(stream, "\t\t\t}\n");
				fprintf(stream, "\t\t\tdefault:\n");
				fprintf(stream, "\t\t\t\tCrashDump::Crash();\n");
				fprintf(stream, "\t\t\t\tbreak;\n");
				fprintf(stream, "\t\t\t}\n");
				fprintf(stream, "\n");
				fprintf(stream, "\t\t\treturn true;\n");
				fprintf(stream, "\t\t}\n");
				fprintf(stream, "\n");
			

				//나눠주기
				fprintf(stream, "\t\tbool PacketProc(NetSessionID sessionID, NetDummyPacket* msg)\n");
				fprintf(stream, "\t\t{\n");
				fprintf(stream, "\t\t\tWORD type;\n");
				fprintf(stream, "\t\t\t*msg >> type;\n");
				fprintf(stream, "\t\t\tswitch (type)\n");
				fprintf(stream, "\t\t\t{\n");

				for (int i = 0; i < procCount_SC_; i++)
				{
					fprintf(stream, "\t\t\tcase %d:\n", i);

					fprintf(stream, "\t\t\t{\n");
					for (int j = 0; j < procInfo_ClientServer[i].paramNum_; j++)
					{
						char* typeBuf = nullptr;
						char tempType[64] = { 0 };
						strcpy_s(tempType, procInfo_ClientServer[i].param_[j].type);

						char* type = strtok_s(tempType, "*", &typeBuf);

						if (*procInfo_ClientServer[i].param_[j].charaLen == NULL)
							fprintf(stream, "\t\t\t\t%s %s;\n", type, procInfo_ClientServer[i].param_[j].name);
					}

					for (int j = 0; j < procInfo_ClientServer[i].paramNum_; j++)
					{
						if (*procInfo_ClientServer[i].param_[j].charaLen != NULL)
						{

							fprintf(stream, "\t\t\t\tRPCBuffer %sBuf(%s);\n", procInfo_ClientServer[i].param_[j].name,
								procInfo_ClientServer[i].param_[j].charaLen);

							fprintf(stream, "\t\t\t\tmsg->DeqData((char*)%sBuf.data, %s);\n", procInfo_ClientServer[i].param_[j].name,
								procInfo_ClientServer[i].param_[j].charaLen);

							char* typeBuf = nullptr;
							char* type = strtok_s(procInfo_ClientServer[i].param_[j].type, "=", &typeBuf);

							fprintf(stream, "\t\t\t\t%s %s = (%s)%sBuf.Data();\n", 
								type,
								procInfo_ClientServer[i].param_[j].name,
								type,
								procInfo_ClientServer[i].param_[j].name);
						}
						else
							fprintf(stream, "\t\t\t\t*msg >> %s;\n", procInfo_ClientServer[i].param_[j].name);
					}

					fprintf(stream, "\t\t\t\treturn %s(", procInfo_ClientServer[i].functionName_);


					for (int j = 0; j < procInfo_ClientServer[i].paramNum_; j++)
					{
						fprintf(stream, "%s, ", procInfo_ClientServer[i].param_[j].name);
					}

					fprintf(stream, "sessionID);\n");
					fprintf(stream, "\t\t\t}\n");

				}
				
				fprintf(stream, "\t\t\t}\n");

				fprintf(stream, "\t\t\treturn false;\n");
				fprintf(stream, "\t\t}\n");


				fprintf(stream, "\n");



				for (int i = 0; i < procCount_SC_; i++)
				{
					//개별 함수
					fprintf(stream, "\t\tbool %s(", procInfo_ClientServer[i].functionName_);


					for (int j = 0; j < procInfo_ClientServer[i].paramNum_; j++)
					{
						char* typeBuf = nullptr;
						char* type = strtok_s(procInfo_ClientServer[i].param_[j].type, "=", &typeBuf);
						fprintf(stream, "%s %s, ",
							type, procInfo_ClientServer[i].param_[j].name);

						if (*typeBuf != NULL)
							strcpy_s(procInfo_ClientServer[i].param_[j].charaLen, typeBuf);
					}

					fprintf(stream, "NetSessionID sessionID)\n");
					fprintf(stream, "\t\t{\n");
					fprintf(stream, "\t\t\treturn false;\n");
					fprintf(stream, "\t\t}\n");
					fprintf(stream, "\n");
				}

				//멤버
				fprintf(stream, "\tprivate:\n");
				fprintf(stream, "\t\t%s* server_;\n", serverName_);
				fprintf(stream, "\t};\n");
				fprintf(stream, "}\n");


				fclose(stream);
			}
			else
			{
				SystemLogger::GetInstance()->LogText(L"CreateResponseCode", LEVEL_ERROR, L"fopen failed");
				CrashDump::Crash();
			}

			sprintf_s(fileName, "%s_SC_Stub.h", clientName_);
			if (fopen_s(&stream, fileName, "w+") == 0)
			{
				//헤더
				fprintf(stream, "#pragma once\n");
				fprintf(stream, "#include <Windows.h>\n");
				fprintf(stream, "#include \"NetRoot/NetServer/NetSessionID.h\"\n");
				fprintf(stream, "#include \"NetRoot/NetServer/NetPacketSet.h\"\n");
				fprintf(stream, "#include \"NetRoot/Common/RPCBuffer.h\"\n");
				fprintf(stream, "#include \"%s_CS_Proxy.h\"\n", clientName_);
				fprintf(stream, "#include \"%s.h\"\n", clientName_);

				fprintf(stream, "\n");
				fprintf(stream, "namespace server_baby\n");
				fprintf(stream, "{\n");

				//생성자
				fprintf(stream, "\tclass %s_SC_Stub\n", clientName_);
				fprintf(stream, "\t{\n");
				fprintf(stream, "\tpublic:\n");
				fprintf(stream, "\t\texplicit %s_SC_Stub(%s* client) : client_(client) {}\n", clientName_, clientName_);
				fprintf(stream, "\n");

				//msgPack Proc
				fprintf(stream, "\t\tbool PacketProc(NetPacketSet* msgPack)\n");
				fprintf(stream, "\t\t{\n");
				fprintf(stream, "\t\t\tswitch (msgPack->GetType())\n");
				fprintf(stream, "\t\t\t{\n");
				fprintf(stream, "\t\t\tcase eNET_RECVED_PACKET_SET:\n");
				fprintf(stream, "\t\t\t{\n");
				fprintf(stream, "\t\t\t\twhile (msgPack->GetSize() > 0)\n");
				fprintf(stream, "\t\t\t\t{\n");
				fprintf(stream, "\t\t\t\t\tNetDummyPacket* packet = nullptr;\n");
				fprintf(stream, "\t\t\t\t\tif (msgPack->Dequeue(&packet) == false)\n");
				fprintf(stream, "\t\t\t\t\t\tCrashDump::Crash();\n");
				fprintf(stream, "\n");
				fprintf(stream, "\t\t\t\t\tif (!PacketProc(msgPack->GetSessionID(), packet))\n");
				fprintf(stream, "\t\t\t\t\t\treturn false;\n");
				fprintf(stream, "\t\t\t\t}\n");
				fprintf(stream, "\t\t\t\tbreak;\n");
				fprintf(stream, "\t\t\t}\n");
				fprintf(stream, "\t\t\tdefault:\n");
				fprintf(stream, "\t\t\t\tCrashDump::Crash();\n");
				fprintf(stream, "\t\t\t\tbreak;\n");
				fprintf(stream, "\t\t\t}\n");
				fprintf(stream, "\n");
				fprintf(stream, "\t\t\treturn true;\n");
				fprintf(stream, "\t\t}\n");
				fprintf(stream, "\n");

				//나눠주기
				fprintf(stream, "\t\tbool PacketProc(NetSessionID sessionID, NetDummyPacket* msg)\n");
				fprintf(stream, "\t\t{\n");
				fprintf(stream, "\t\t\tWORD type;\n");
				fprintf(stream, "\t\t\t*msg >> type;\n");
				fprintf(stream, "\t\t\tswitch (type)\n");
				fprintf(stream, "\t\t\t{\n");

				for (int i = 0; i < procCount_CS_; i++)
				{
					fprintf(stream, "\t\t\tcase %d:\n", i);

					fprintf(stream, "\t\t\t{\n");
					for (int j = 0; j < procInfo_ServerClient[i].paramNum_; j++)
					{
						char* typeBuf = nullptr;
						char tempType[64] = { 0 };
						strcpy_s(tempType, procInfo_ServerClient[i].param_[j].type);

						char* type = strtok_s(tempType, "*", &typeBuf);
						if (*procInfo_ServerClient[i].param_[j].charaLen == NULL)
							fprintf(stream, "\t\t\t\t%s %s;\n", type, procInfo_ServerClient[i].param_[j].name);
					}

					for (int j = 0; j < procInfo_ServerClient[i].paramNum_; j++)
					{
						if (*procInfo_ServerClient[i].param_[j].charaLen != NULL)
						{
							fprintf(stream, "\t\t\t\tRPCBuffer %sBuf(%s);\n", procInfo_ServerClient[i].param_[j].name,
								procInfo_ServerClient[i].param_[j].charaLen);

							fprintf(stream, "\t\t\t\tmsg->DeqData((char*)%sBuf.data, %s);\n", procInfo_ServerClient[i].param_[j].name,
								procInfo_ServerClient[i].param_[j].charaLen);

							char* typeBuf = nullptr;
							char* type = strtok_s(procInfo_ServerClient[i].param_[j].type, "=", &typeBuf);

							fprintf(stream, "\t\t\t\t%s %s = (%s)%sBuf.Data();\n",
								type,
								procInfo_ServerClient[i].param_[j].name,
								type,
								procInfo_ServerClient[i].param_[j].name);
						}
						else
							fprintf(stream, "\t\t\t\t*msg >> %s;\n", procInfo_ServerClient[i].param_[j].name);
					}

					fprintf(stream, "\t\t\t\treturn %s(", procInfo_ServerClient[i].functionName_);


					for (int j = 0; j < procInfo_ServerClient[i].paramNum_; j++)
					{
						fprintf(stream, "%s, ", procInfo_ServerClient[i].param_[j].name);
					}

					fprintf(stream, "sessionID);\n");
					fprintf(stream, "\t\t\t}\n");

				}

				fprintf(stream, "\t\t\t}\n");

				fprintf(stream, "\t\t\treturn false;\n");
				fprintf(stream, "\t\t}\n");


				fprintf(stream, "\n");

				for (int i = 0; i < procCount_CS_; i++)
				{
					//개별 함수
					fprintf(stream, "\t\tbool %s(", procInfo_ServerClient[i].functionName_);


					for (int j = 0; j < procInfo_ServerClient[i].paramNum_; j++)
					{
						char* typeBuf = nullptr;
						char* type = strtok_s(procInfo_ServerClient[i].param_[j].type, "=", &typeBuf);
						fprintf(stream, "%s %s, ",
							type, procInfo_ServerClient[i].param_[j].name);

						if (*typeBuf != NULL)
							strcpy_s(procInfo_ServerClient[i].param_[j].charaLen, typeBuf);
					}

					fprintf(stream, "NetSessionID sessionID)\n");
					fprintf(stream, "\t\t{\n");
					fprintf(stream, "\t\t\treturn false;\n");
					fprintf(stream, "\t\t}\n");
					fprintf(stream, "\n");
				}

				//멤버
				fprintf(stream, "\tprivate:\n");
				fprintf(stream, "\t\t%s* client_;\n", clientName_);
				fprintf(stream, "\t};\n");
				fprintf(stream, "}\n");


				fclose(stream);
			}
			else
			{
				SystemLogger::GetInstance()->LogText(L"CreateResponseCode", LEVEL_ERROR, L"fopen failed");
				CrashDump::Crash();
			}

		}

	private:
		char serverName_[512] = { 0 };
		char clientName_[512] = { 0 };
		ProcedureInfo procInfo_ServerClient[256] = { 0 };
		ProcedureInfo procInfo_ClientServer[256] = { 0 };
		int procCount_SC_;
		int procCount_CS_;

	};

}

#endif
