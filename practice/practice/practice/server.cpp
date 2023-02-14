#include "C:\Users\kyeongnyeong.lee\Desktop\practice\practice\practice\Common.h"
#include "server.h"

#include <string>
#include <string.h>

#define SERVERPORT 9000
#define BUFSIZE    512

using namespace std;


// 내부 구현용 함수
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

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE + 1];
	USER objUser;

	bool bCreateID = false; // 아이디 생성 여부
	bool bExit = false; // 연결 종료 여부 플래그 변수


	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	// 채팅서버 인사말 클라이언트에게 전송
	send(client_sock, objPrintOrSend.sendMap[eSendMapKey::WELCOME].c_str(),
		(objPrintOrSend.sendMap[eSendMapKey::WELCOME]).size(), 0);

	while (!bCreateID)
	{
		// 로그인 요청 문구 클라이언트에게 전송
		send(client_sock, objPrintOrSend.sendMap[eSendMapKey::REQUESTLOGIN].c_str(),
			(objPrintOrSend.sendMap[eSendMapKey::REQUESTLOGIN]).size(), 0);

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
			objUser.id = tempBufStr.substr(6, tempBufStr.size() - 8);
			printf("%s:%d [] %s", addr, ntohs(clientaddr.sin_port), buf);
			break;
		}

		printf("%s:%d [] %s", addr, ntohs(clientaddr.sin_port), buf);
	}


	// 채팅 서버 입장 인사말 클라이언트에게 전송
	for (int i = eSendMapKey::COMPLETELOGIN_1; i <= eSendMapKey::COMPLETELOGIN_6; i++)
	{
		send(client_sock, objPrintOrSend.sendMap[i].c_str(),
			(objPrintOrSend.sendMap[i]).size(), 0);
	}

	while(1)
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

		// 받은 데이터 출력
		printf("%s:%d [%s] %s", addr, ntohs(clientaddr.sin_port), objUser.id.c_str(), buf);

		if (bExit)
		{
			printf("%s:%d closed\n", addr, ntohs(clientaddr.sin_port));
			break;
		}
	}

	// 소켓 닫기
	closesocket(client_sock);

	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	/////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	// 서버 초기화 성공 메시지 출력
	printf("%s", objPrintOrSend.printMap[ePrintMapKey::INITIALIZE].c_str());


	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

		// 특정 클라이언트와 연결 성공 메시지 출력
		printf("%s:%d connected\n", addr, ntohs(clientaddr.sin_port));

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);
		if (hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

