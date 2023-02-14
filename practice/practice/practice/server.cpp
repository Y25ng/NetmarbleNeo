#include "Server.h"
#include "User.h"
#include "Room.h"

#include <string>
#include <string.h>
#include <vector>

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

sPrintOrSend objPrintOrSend;
ServerManager objServerManager;

int main(int argc, char* argv[])
{
	Server objServer;

	objServer.InitServer();
	objServer.Run();
}

int Server::InitServer()
{
	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		return 1;

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

	return 0;
}

int Server::Run()
{
	HANDLE hThread;

	while (1) 
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
	return 0;
}


string Login(SOCKET client_sock, int& retval, char* buf, char* addr, struct sockaddr_in clientaddr, User* objUser)
{
	// 로그인 요청 문구 클라이언트에게 전송
	send(client_sock, objPrintOrSend.sendMap[eSendMapKey::REQUESTLOGIN].c_str(),
		(objPrintOrSend.sendMap[eSendMapKey::REQUESTLOGIN]).size(), 0);

	while (1)
	{
		bool bAlreadyExistID = false;

		// 데이터 받기
		retval = recvline(client_sock, buf, BUFSIZE + 1);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		string tempBufStr = buf;

		if (tempBufStr.substr(0, 5) == "LOGIN" && tempBufStr[5] == ' ' && tempBufStr.size() >= 9)
		{
			bAlreadyExistID = objServerManager.GetUserInfoMap().find(tempBufStr.substr(6, tempBufStr.size() - 8)) != objServerManager.GetUserInfoMap().end();

			if (bAlreadyExistID)
			{
				send(client_sock, objPrintOrSend.sendMap[eSendMapKey::ALREADYEXISTID].c_str(),
					(objPrintOrSend.sendMap[eSendMapKey::ALREADYEXISTID]).size(), 0);
			}
			else
			{
				objUser->SetUserInfo(tempBufStr.substr(6, tempBufStr.size() - 8),
					to_string(ntohs(clientaddr.sin_port)), addr);

				printf("%s:%d [] %s", addr, ntohs(clientaddr.sin_port), buf);

				objServerManager.GetUserInfoMap()[objUser->GetID()] = objUser->GetIP() + ":" + objUser->GetPort();
				break;
			}
		}

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

	bool bExit = false; // 연결 종료 여부 플래그 변수


	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	// 채팅서버 인사말 클라이언트에게 전송
	send(client_sock, objPrintOrSend.sendMap[eSendMapKey::WELCOME].c_str(),
		(objPrintOrSend.sendMap[eSendMapKey::WELCOME]).size(), 0);


	string tempID = Login(client_sock, retval, buf, addr, clientaddr, &objUser);

	// 채팅 서버 입장 인사말 클라이언트에게 전송
	for (int i = eSendMapKey::COMPLETELOGIN_1; i <= eSendMapKey::COMPLETELOGIN_6; i++)
	{
		send(client_sock, objPrintOrSend.sendMap[i].c_str(),
			(objPrintOrSend.sendMap[i]).size(), 0);
	}

	while (1)
	{
		// 명렁어 H or X 정보 클라이언트에게 전송
		send(client_sock, objPrintOrSend.sendMap[eSendMapKey::FIRSTINFO].c_str(),
			(objPrintOrSend.sendMap[FIRSTINFO]).size(), 0);

		// 데이터 받기
		retval = recvline(client_sock, buf, BUFSIZE);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		string tempBuf = buf;

		if (tempBuf.size() == 3) // 클라이언트에게서 한글자 입력이 들어왔을 경우
		{
			switch (tempBuf[0])
			{
			case 'H': // H 입력이 들어왔을 경우

				// 명령어 안내 전송
				for (int i = COMMANDINFO_1; i <= COMMANDINFO_11; i++)
				{
					send(client_sock, objPrintOrSend.sendMap[i].c_str(),
						(objPrintOrSend.sendMap[i]).size(), 0);
				}
				break;

			case 'X': // X 입력이 들어왔을 경우
				bExit = true;
				break;

			default:
				break;
			}
		}
		else if (tempBuf.size() == 4) // 클라이언트에게서 두글자 입력이 들어왔을 경우
		{
			if (tempBuf.substr(0, 2) == "US") // US 입력이 들어왔을 경우
			{
				string tempStr = objServerManager.LoginUsersInfo();
				send(client_sock, tempStr.c_str(), tempStr.size(), 0);
			}
		}
		else
		{
			vector<string> tempVec;
			tempVec = split(tempBuf, ' ');

			if (tempVec.size() >= 2)
			{
				if (tempVec[0] == "O" && tempVec.size() >=3)
				{
					int tempMaxNum = stoi(tempVec[1]);
					string tempRoomTitle = "";

					for (int i = 2; i < tempVec.size(); i++)
					{
						tempRoomTitle += tempVec[i];
					}

					if (tempMaxNum >= 2 && tempMaxNum <= 20 && tempRoomTitle != "")
					{
						Room objRoom;
						objRoom.SetParticipantMaxNum(tempMaxNum);
						objRoom.SetParticipantNum(1);
						objRoom.SetRoomTitle(tempRoomTitle);
						objRoom.SetRoomNumber(objServerManager.GetRoomInfoVec().size() + 1);

						objServerManager.GetRoomInfoVec().push_back(objRoom);
					}
				}
			}
		}

		// 받은 데이터 출력
		printf("%s:%d [%s] %s", addr, ntohs(clientaddr.sin_port), objUser.GetID().c_str(), buf);

		if (bExit)
		{
			objServerManager.GetUserInfoMap().erase(objUser.GetID()); // 접속 종료시 접속자 명단에서 삭제
			printf("%s:%d closed\n", addr, ntohs(clientaddr.sin_port));
			break;
		}
	}

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


vector<string> split(string str, char delimiter)
{
	std::vector<string> vs;
	string temp = "";

	for (int i = 0; !(str[i] == '\n'); i++)
	{
		if (str[i] == delimiter) /* 구분자를 만나는 경우 push 후 clear */
		{
			vs.push_back(temp);
			temp.clear();

			continue;
		}

		/* temp에 문자를 하나씩 담는다. */
		temp.push_back(str[i]);
	}

	/* 마지막 string은 push되지 않았으므로 for문 밖에서 push */
	vs.push_back(temp);

	return vs;
}



