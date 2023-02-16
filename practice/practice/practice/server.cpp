#include "Server.h"
#include "User.h"
#include "Room.h"
#include "Util.h"

#include <string>
#include <string.h>
#include <vector>
#include <list>
#include <map>

#define SERVERPORT 9000
#define BUFSIZE    512

using namespace std;

// 내부 구현용 함수
int _recv_ahead(SOCKET s, char* p);

// 사용자 정의 데이터 수신 함수
int recvline(SOCKET s, char* buf, int maxlen);

vector<string> split(string str, char delimiter);

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg);
void SendMessageToClient(SOCKET client_socket, string& message);

sPrintOrSend objPrintOrSend;
ServerManager objServerManager;

int main()
{
	Server objServer;

	objServer.InitServer();
	objServer.Run();
}

bool Server::InitServer()
{
	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		return true;

	// 소켓 생성
	m_listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (m_listen_sock == INVALID_SOCKET)
	{
		err_quit("socket()");
	}

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	m_retval = bind(m_listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	if (m_retval == SOCKET_ERROR)
	{
		err_quit("bind()");
	}

	// listen()
	m_retval = listen(m_listen_sock, SOMAXCONN);

	if (m_retval == SOCKET_ERROR)
	{
		err_quit("listen()");
	}

	// 서버 초기화 성공 메시지 출력
	printf("%s", objPrintOrSend.printMap[ePrintMapKey::INITIALIZE].c_str());

	return false;
}

bool Server::Run()
{
	HANDLE hThread;

	while (true)
	{
		// accept()
		m_addrlen = sizeof(m_clientaddr);
		m_client_sock = accept(m_listen_sock, (struct sockaddr*)&m_clientaddr, &m_addrlen);

		if (m_client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &m_clientaddr.sin_addr, addr, sizeof(addr));

		// 특정 클라이언트와 연결 성공 메시지 출력
		printf("%s:%d connected\n", addr, ntohs(m_clientaddr.sin_port));

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)m_client_sock, 0, NULL);
		if (hThread == NULL) { closesocket(m_client_sock); }
		else { CloseHandle(hThread); }
	}

	// 소켓 닫기
	closesocket(m_listen_sock);

	// 윈속 종료
	WSACleanup();
	return false;
}


string Login(SOCKET client_sock, int& retval, char* buf, char* addr, struct sockaddr_in clientaddr, User* objUser)
{
	// 로그인 요청 문구 클라이언트에게 전송
	SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::REQUESTLOGIN]);

	while (true)
	{
		bool bAlreadyExistID = false;
		string tempBufStr = "";

		// 데이터 받기
		retval = recvline(client_sock, buf, BUFSIZE + 1);
		if (retval == SOCKET_ERROR) 
		{
			err_display("recv()");
			break;
		}
		else if (retval == eRet::EMPTYMESSAGE)
		{
			break;
		}

		tempBufStr = buf;

		// 클라이언트 입력 형식이 로그인 가능한 형식인지 확인 => 가능하면 로그인 과정 수행
		if (tempBufStr.substr(0, 5) == "LOGIN" && tempBufStr[5] == ' ' && tempBufStr.size() >= eCommand::MINLENGTHLOGINPOSSIBLE)
		{
			// 현재 접속을 원하는 유저의 입력 ID가 기존 유저의 ID와 겹치는지 확인 
			bAlreadyExistID = objServerManager.GetUserInfoMap().find(tempBufStr.substr(6, tempBufStr.size() - eCommand::MINLENGTHLOGINPOSSIBLE-1)) != objServerManager.GetUserInfoMap().end();

			if (bAlreadyExistID) // 기존 유저와 ID가 겹칠 시
			{
				// 현재 접속을 원하는 유저에게 이미 존재하는 ID임을 알려주는 안내 문구 전송
				SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::ALREADYEXISTID]);
			}
			else // 기존 유저와 ID가 겹치지 않을 시
			{

				// 유저 객체를 통해 정보 저장 (id, ip, port, socket)
				objUser->SetUserInfo(tempBufStr.substr(6, tempBufStr.size() - eCommand::MINLENGTHLOGINPOSSIBLE - 1), addr, to_string(ntohs(clientaddr.sin_port)), (unsigned int)client_sock);

				// 서버측에 유저 입력 정보 출력
				printf("%s:%d [] %s", addr, ntohs(clientaddr.sin_port), buf);

				// ServerManager에서 관리하는 유저정보 맵에 유저 객체 삽입
				// key: 유저 ID
				// value: 유저 클래스 객체
				objServerManager.GetUserInfoMap()[objUser->GetID()] = *objUser;

				break;
			}
		}

		// 서버측에 유저 입력 정보 출력
		printf("%s:%d [] %s", addr, ntohs(clientaddr.sin_port), buf);
	}

	return objUser->GetID();
}

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	User objUser;
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE + 1];

	// 연결 종료 여부 플래그 변수
	bool bExit = false; 


	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	// 채팅서버 인사말 클라이언트에게 전송
	SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::WELCOME]);

	// 유저 로그인 ID
	string tempID = Login(client_sock, retval, buf, addr, clientaddr, &objUser);

	// 채팅 서버 입장 인사말 클라이언트에게 전송
	for (int i = eSendMapKey::COMPLETELOGIN_1; i <= eSendMapKey::COMPLETELOGIN_6; ++i)
	{
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[i]);
	}

	while (true)
	{
		// 명렁어 H and X 정보 클라이언트에게 전송
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::FIRSTINFO]);

		// 데이터 받기
		retval = recvline(client_sock, buf, BUFSIZE);
		if (retval == SOCKET_ERROR) 
		{
			// 접속 종료시 접속자 명단에서 삭제 
			// 접속 종료시 방 명단에서 삭제
			objServerManager.UserQuitServer(objUser); 

			err_display("recv()");
			break;
		}
		else if (retval == eRet::EMPTYMESSAGE)
		{
			// 접속 종료시 접속자 명단에서 삭제 
			// 접속 종료시 방 명단에서 삭제
			objServerManager.UserQuitServer(objUser);

			break;
		}

		string tempBuf = buf;

		if (tempBuf.size() == eCommand::ONLYCOMMAND_ONE) // 클라이언트에게서 한글자 명령이 들어왔을 경우
		{
			switch (tempBuf[0])
			{
			case 'H': // H 입력이 들어왔을 경우

				// 명령어 안내 전송
				SCommand::Command_H_Func((unsigned int)client_sock);

				break;

			case 'X': // X 입력이 들어왔을 경우
				bExit = true;
				break;

			default:
				break;
			}
		}
		else if (tempBuf.size() == eCommand::ONLYCOMMAND_TWO) // 클라이언트에게서 두글자 명령이 들어왔을 경우
		{
			if (tempBuf.substr(0, 2) == "US") // US 입력이 들어왔을 경우
			{
				// 이용자 목록 클라이언트에게 전송
				SCommand::Command_US_Func((unsigned int)client_sock);
			}
			else if (tempBuf.substr(0, 2) == "LT") // LT 입력이 들어왔을 경우
			{
				// 대화방 목록 출력 클라이언트에게 전송
				SCommand::Command_LT_Func((unsigned int)client_sock);
			}
		}
		else // 클라이언트에게서 세글자 이상의 명령이 들어왔을 경우
		{
			// 클라이언트 입력 값을 띄어쓰기 기준으로 분리하여 저장할 컨테이너
			vector<string> tempBufSplit_Vec;

			// 클라이언트 입력 값 분리 후 저장
			tempBufSplit_Vec = Util::split(tempBuf, ' ');

			// 1개 명령과 1개 이상의 인자값이 존재
			if (tempBufSplit_Vec.size() >= eCommand::ONECOMMANDONEPARAMETER)
			{
				// 명령: O && 2개 이상의 인자값이 존재 && 최대 인원수에 숫자가 아닌 문자가 들어오지 않았을 때
				// => 방 개설 명령
				if (tempBufSplit_Vec[0] == "O" && tempBufSplit_Vec.size() >= eCommand::ONECOMMANDTWOPARAMETER && Util::bCanUsestoi(tempBufSplit_Vec[1]))
				{
					SCommand::Command_O_Func(tempBufSplit_Vec, objUser, (unsigned int)client_sock, addr, clientaddr, buf, retval);
				}
				else if (tempBufSplit_Vec[0] == "ST" && tempBufSplit_Vec.size() == eCommand::ONECOMMANDONEPARAMETER) // 대화방 정보
				{
					SCommand::Command_ST_Func((unsigned int)client_sock, tempBufSplit_Vec);
				}
				else if (tempBufSplit_Vec[0] == "J" && tempBufSplit_Vec.size() == eCommand::ONECOMMANDONEPARAMETER) // 대화방 참여
				{
					SCommand::Command_J_Func(tempBufSplit_Vec, objUser, (unsigned int)client_sock, addr, clientaddr, buf, retval);
				}
				else if (tempBufSplit_Vec[0] == "PF" && tempBufSplit_Vec.size() == eCommand::ONECOMMANDONEPARAMETER) // 이용자 정보 확인
				{
					SCommand::Command_PF_Func(tempBufSplit_Vec, (unsigned int)client_sock, objUser);
				}
				else if (tempBufSplit_Vec[0] == "TO" && tempBufSplit_Vec.size() >= eCommand::ONECOMMANDTWOPARAMETER) // 유저에게 쪽지보내기
				{
					SCommand::Command_TO_Func(tempBufSplit_Vec, (unsigned int)client_sock, objUser);
				}
			}
		}

		// 받은 데이터 출력
		printf("%s:%d [%s] %s", addr, ntohs(clientaddr.sin_port), objUser.GetID().c_str(), buf);

		if (bExit)
		{
			break;
		}
	}

	// 접속 종료시 접속자 명단에서 삭제 
	// 접속 종료시 방 명단에서 삭제
	objServerManager.UserQuitServer(objUser);

	printf("%s:%d closed\n", addr, ntohs(clientaddr.sin_port));

	// 소켓 닫기
	closesocket(client_sock);

	return 0;
}

int _recv_ahead(SOCKET s, char* p)
{
	__declspec(thread) static int nbytes = 0;
	__declspec(thread) static char buf[1024];
	__declspec(thread) static char* ptr;

	if (nbytes == 0 || nbytes == SOCKET_ERROR) {
		nbytes = recv(s, buf, sizeof(buf), 0);
		if (nbytes == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
		else if (nbytes == 0)
			return 0;
		ptr = buf;
	}

	--nbytes;
	*p = *ptr++;
	return 1;
}

// 사용자 정의 데이터 수신 함수
int recvline(SOCKET s, char* buf, int maxlen)
{
	int n, nbytes;
	char c, * ptr = buf;

	for (n = 1; n < maxlen; n++) {
		nbytes = _recv_ahead(s, &c);
		if (nbytes == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;
		}
		else if (nbytes == 0) {
			*ptr = 0;
			return n - 1;
		}
		else
			return SOCKET_ERROR;
	}

	*ptr = 0;
	return n;
}

void SendMessageToClient(SOCKET client_socket, string& message)
{
	send(client_socket, message.c_str(), (int)message.size(), 0);
}

void SCommand::Chatting_By_OorJ(User& objUser, Room& objRoom, unsigned int client_sock, char* addr, struct sockaddr_in clientaddr, char* buf, int retval)
{
	while (true)
	{
		// 데이터 받기
		retval = recvline(client_sock, buf, BUFSIZE);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		string tempFlagStr = buf;

		if (tempFlagStr.size() == eCommand::ONLYCOMMAND_ONE && tempFlagStr[0] == 'X')
		{
			// 접속 종료시 접속자 명단에서 삭제 
			// 접속 종료시 방 명단에서 삭제
			objServerManager.UserQuitServer(objUser);
			break;
		}

		auto iter_objRoom = objServerManager.GetRoomInfoMap().end();
		--iter_objRoom;
		objRoom = iter_objRoom->second;
		string tempMessage = objUser.GetID() + "> " + buf;

		// 방에 있는 모든 유저에게 메시지 전송
		for (auto iter = objRoom.GetParticipantMap().begin(); iter != objRoom.GetParticipantMap().end(); ++iter)
		{
			unsigned int tempTargetUserSock = iter->second.GetSocket();
			SendMessageToClient(tempTargetUserSock, tempMessage);
		}

		printf("%s:%d [%s] %s", addr, ntohs(clientaddr.sin_port), objUser.GetID().c_str(), buf);
	}
}

void SCommand::Command_H_Func(unsigned int client_sock)
{
	for (int i = eSendMapKey::COMMANDINFO_1; i <= eSendMapKey::COMMANDINFO_11; i++)
	{
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[i]);
	}
}

void SCommand::Command_US_Func(unsigned int client_sock)
{
	string tempStr = objServerManager.LoginUsersList();
	SendMessageToClient(client_sock, tempStr);
}

void SCommand::Command_LT_Func(unsigned int client_sock)
{
	// 대화방 목록 출력 클라이언트에게 전송
	string tempStr = objServerManager.RoomList();
	SendMessageToClient(client_sock, tempStr);
}

void SCommand::Command_O_Func(vector<string>& tempBufSplit_Vec, User& objUser, unsigned int client_sock, char* addr, struct sockaddr_in clientaddr, char* buf, int retval)
{
	// 방 최대 인원수  
	int tempMaxNum = stoi(tempBufSplit_Vec[1]);

	// 방 제목
	string tempRoomTitle = "";

	for (int i = 2; i < tempBufSplit_Vec.size(); i++)
	{
		tempRoomTitle += tempBufSplit_Vec[i] + "";
	}

	tempRoomTitle = tempRoomTitle.substr(0, tempRoomTitle.size() - 1);

	// 방 최대 인원수 제한 만족 && 방 제목이 공백이 아닐 때
	if (tempMaxNum >= eCommand::MINNUMFORROOM && tempMaxNum <= eCommand::MAXNUMFORROOM && tempRoomTitle != "\r")
	{
		Room objRoom;

		//  방 정보 세팅 및 유저 정보 세팅
		objRoom.SetRoomInfo((int)objServerManager.GetRoomInfoMap().size() + 1, 1, tempMaxNum, tempRoomTitle);
		objUser.SetbInRoom(true);
		objUser.SetRoomNumber(objRoom.GetRoomNumber());
		objUser.SetRoomTitle(objRoom.GetRoomTitle());
		objRoom.GetParticipantMap().emplace(objUser.GetID(), objUser);

		objServerManager.GetRoomInfoMap().emplace(objRoom.GetRoomTitle(), objRoom);

		SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::CREATEROOM]);

		string tempStr = "** " + objUser.GetID() + "님이 들어오셨습니다. " + "(현재인원 1/" + to_string(objRoom.GetParticipantMaxNum()) + ")\n\r";
		SendMessageToClient(client_sock, tempStr);
		printf("%s:%d [%s] %s", addr, ntohs(clientaddr.sin_port), objUser.GetID().c_str(), buf);

		Chatting_By_OorJ(objUser, objRoom, (unsigned int)client_sock, addr, clientaddr, buf, retval); // 채팅방 내에서의 채팅 함수
	}
	else // 방 생성 조건에 부합하지 않은 경우
	{
		if (!(tempMaxNum >= eCommand::MINNUMFORROOM && tempMaxNum <= eCommand::MAXNUMFORROOM)) // 방 최대 인원수가 잘못 입력된 경우
		{
			SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::CANTCREATEROOMBYNUM]);
		}
		else // 방 제목이 잘못 입력된 경우
		{
			SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::CANTCREATEROOMBYTITLE]);
		}
	}
}

void SCommand::Command_ST_Func(unsigned int client_sock, vector<string>& tempBufSplit_Vec)
{
	string flagStr = tempBufSplit_Vec[1].substr(0, tempBufSplit_Vec[1].size() - 1);

	if (Util::bCanUsestoi(flagStr))
	{
		// 입력받은 방 번호
		int tempRoomNum = stoi(tempBufSplit_Vec[1]);

		// 입력받은 방 번호의 방이 존재할 경우
		if (objServerManager.GetRoomInfoMap().size() >= tempRoomNum)
		{
			auto iter_objRoom = objServerManager.GetRoomInfoMap().begin();

			for (int i = 0; i < tempRoomNum - 1; i++)
			{
				++iter_objRoom;
			}

			map<string, User>& temp_ParticipantList = iter_objRoom->second.GetParticipantMap();
			auto iter_tempList = temp_ParticipantList.begin();

			string tempStr = "";

			// 방 정보(방 번호, 인원수, 최대 인원수, 방 이름)
			tempStr = "[ " + to_string(iter_objRoom->second.GetRoomNumber()) + "]  "
				+ "( " + to_string(iter_objRoom->second.GetParticipantNum()) + "/"
				+ to_string(iter_objRoom->second.GetParticipantMaxNum()) + " )  "
				+ iter_objRoom->second.GetRoomTitle() + "\n\r"
				+ "개설시간:  \n\r";

			// 참여자 정보(아이디, 참여시간)
			for (int i = 0; i < temp_ParticipantList.size(); i++)
			{
				tempStr += "참여자: " + iter_tempList->second.GetID() + "            참여시간 :\n\r";
			}

			SendMessageToClient((unsigned int)client_sock, tempStr);
		}
	}
}

void SCommand::Command_J_Func(vector<string>& tempBufSplit_Vec, User& objUser, unsigned int client_sock, char* addr, struct sockaddr_in clientaddr, char* buf, int retval)
{
	string flagStr = tempBufSplit_Vec[1].substr(0, tempBufSplit_Vec[1].size() - 1);

	if (Util::bCanUsestoi(flagStr))
	{
		// 입력받은 방 번호
		int tempRoomNum = stoi(tempBufSplit_Vec[1]);

		if (objServerManager.GetRoomInfoMap().size() >= tempRoomNum)
		{
			auto iter_objRoomInfo = objServerManager.GetRoomInfoMap().begin();

			for (int i = 0; i < tempRoomNum - 1; i++)
			{
				++iter_objRoomInfo;
			}

			// 유저가 입력한 번호에 해당하는 방 객체
			Room& temp_objRoom = iter_objRoomInfo->second;

			// 방이 꽉 차지 않았다면
			if (temp_objRoom.GetbFull())
			{
				temp_objRoom.SetParticipantNum(temp_objRoom.GetParticipantNum() + 1);
				temp_objRoom.GetParticipantMap().emplace(objUser.GetID(), objUser);
				objUser.SetbInRoom(true);
				objUser.SetRoomNumber(temp_objRoom.GetRoomNumber());
				objUser.SetRoomTitle(temp_objRoom.GetRoomTitle());

				string tempStr = "** " + objUser.GetID() + "님이 들어오셨습니다. " + "(현재인원" + to_string(temp_objRoom.GetParticipantNum()) + "/ " + to_string(temp_objRoom.GetParticipantMaxNum()) + ")\n\r";
				printf("%s:%d [%s] %s", addr, ntohs(clientaddr.sin_port), objUser.GetID().c_str(), buf);

				// 방에 있는 모든 인원에게 유저 대화방 참여 메시지 전송
				for (auto iter_userInRoom = temp_objRoom.GetParticipantMap().begin(); iter_userInRoom != temp_objRoom.GetParticipantMap().end(); ++iter_userInRoom)
				{
					unsigned int tempUserSock = iter_userInRoom->second.GetSocket();
					SendMessageToClient(tempUserSock, tempStr);
				}

				Chatting_By_OorJ(objUser, temp_objRoom, (unsigned int)client_sock, addr, clientaddr, buf, retval); // 채팅방 내에서의 채팅 함수
			}
			else
			{
				// 대화방이 꽉 찼다는 메시지 전송
				SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::ROOMFULL]);
			}
		}
	}
}

void SCommand::Command_PF_Func(vector<string>& tempBufSplit_Vec, unsigned int client_sock, User& objUser)
{
	// 유저에게 입력받은 ID
	string tempUserID = tempBufSplit_Vec[1].substr(0, tempBufSplit_Vec[1].size() - 1);

	// 접속자중 유저가 찾는 ID가 없을 경우
	if (objServerManager.GetUserInfoMap().find(tempUserID) == objServerManager.GetUserInfoMap().end())
	{
		string tempStr = "** " + tempUserID + "님을 찾을 수 없습니다.\n\r";
		SendMessageToClient(client_sock, tempStr);
	}
	else // 접속자중 유저가 찾는 ID가 있을 경우
	{
		if (objUser.GetbInRoom()) // 해당 접속자가 채팅방에 입장해있는 경우
		{
			string tempStr = "** " + tempUserID + "님은 현재 채팅방에 있습니다.\n\r";

			tempStr += "** 접속지: "
				+ objServerManager.GetUserInfoMap()[tempUserID].GetIP() + ":"
				+ objServerManager.GetUserInfoMap()[tempUserID].GetPort() + "\n\r";

			SendMessageToClient(client_sock, tempStr);
		}
		else // 해당 접속자가 대기실에 있는 경우
		{
			string tempStr = "** " + tempUserID + "님은 현재 대기실에 있습니다.\n\r";

			tempStr += "** 접속지: "
				+ objServerManager.GetUserInfoMap()[tempUserID].GetIP() + ":"
				+ objServerManager.GetUserInfoMap()[tempUserID].GetPort() + "\n\r";

			SendMessageToClient(client_sock, tempStr);
		}
	}
}

void SCommand::Command_TO_Func(vector<string>& tempBufSplit_Vec, unsigned int client_sock, User& objUser)
{
	// 유저에게 입력받은 ID
	string tempRecvUserID = tempBufSplit_Vec[1];

	if (tempRecvUserID == objUser.GetID()) // 자기 자신에게 쪽지를 보내려고 하는 경우
	{
		// 해당 유저에게 자기 자신에게는 쪽지를 보낼 수 없다는 문구 전송
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::CANNOTSENDME]);
	}
	else if (objServerManager.GetUserInfoMap().find(tempRecvUserID) == objServerManager.GetUserInfoMap().end()) // 접속해있지 않은 유저에게 쪽지를 보내려고 하는 경우
	{
		// 해당 유저에게 타켓 유저가 접속해있지 않다는 문구 전송
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::CANNOTFINDUSER]);
	}
	else // 해당 유저가 접속해 있을 때
	{
		string tempMessage = "\n\r# " + objUser.GetID() + "님의 쪽지 ==> ";

		for (int i = 2; i < tempBufSplit_Vec.size(); i++)
		{
			if (i == tempBufSplit_Vec.size() - 1)
			{
				tempMessage += tempBufSplit_Vec[i].substr(0, tempBufSplit_Vec[i].size() - 1) + "\n\r";
			}
			else
			{
				tempMessage += tempBufSplit_Vec[i] + " ";
			}
		}

		// 쪽지 전송 정보 수신자와 송신자에게 모두 전송
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::SENDLETTER]);
		SendMessageToClient(objServerManager.GetUserInfoMap()[tempRecvUserID].GetSocket(), tempMessage);

		// 명렁어 H or X 정보 클라이언트에게 전송
		SendMessageToClient(objServerManager.GetUserInfoMap()[tempRecvUserID].GetSocket(), objPrintOrSend.sendMap[eSendMapKey::FIRSTINFO]);
	}
}
