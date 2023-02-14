#include "Server.h"
#include "User.h"
#include "Room.h"

#include <string>
#include <string.h>
#include <vector>

#define SERVERPORT 9000
#define BUFSIZE    512

using namespace std;


// ���� ������ �Լ�
int _recv_ahead(SOCKET s, char* p);

// ����� ���� ������ ���� �Լ�
int recvline(SOCKET s, char* buf, int maxlen);

vector<string> split(string str, char delimiter);

// Ŭ���̾�Ʈ�� ������ ���
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
	// ���� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		return 1;

	// ���� ����
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

	// ���� �ʱ�ȭ ���� �޽��� ���
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

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &m_clientaddr.sin_addr, addr, sizeof(addr));

		// Ư�� Ŭ���̾�Ʈ�� ���� ���� �޽��� ���
		printf("%s:%d connected\n", addr, ntohs(m_clientaddr.sin_port));

		// ������ ����
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)m_client_sock, 0, NULL);
		if (hThread == NULL) { closesocket(m_client_sock); }
		else { CloseHandle(hThread); }
	}

	// ���� �ݱ�
	closesocket(m_listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}


string Login(SOCKET client_sock, int& retval, char* buf, char* addr, struct sockaddr_in clientaddr, User* objUser)
{
	// �α��� ��û ���� Ŭ���̾�Ʈ���� ����
	send(client_sock, objPrintOrSend.sendMap[eSendMapKey::REQUESTLOGIN].c_str(),
		(objPrintOrSend.sendMap[eSendMapKey::REQUESTLOGIN]).size(), 0);

	while (1)
	{
		bool bAlreadyExistID = false;

		// ������ �ޱ�
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

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
	User objUser;
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE + 1];

	bool bExit = false; // ���� ���� ���� �÷��� ����


	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	// ä�ü��� �λ縻 Ŭ���̾�Ʈ���� ����
	send(client_sock, objPrintOrSend.sendMap[eSendMapKey::WELCOME].c_str(),
		(objPrintOrSend.sendMap[eSendMapKey::WELCOME]).size(), 0);


	string tempID = Login(client_sock, retval, buf, addr, clientaddr, &objUser);

	// ä�� ���� ���� �λ縻 Ŭ���̾�Ʈ���� ����
	for (int i = eSendMapKey::COMPLETELOGIN_1; i <= eSendMapKey::COMPLETELOGIN_6; i++)
	{
		send(client_sock, objPrintOrSend.sendMap[i].c_str(),
			(objPrintOrSend.sendMap[i]).size(), 0);
	}

	while (1)
	{
		// ���� H or X ���� Ŭ���̾�Ʈ���� ����
		send(client_sock, objPrintOrSend.sendMap[eSendMapKey::FIRSTINFO].c_str(),
			(objPrintOrSend.sendMap[FIRSTINFO]).size(), 0);

		// ������ �ޱ�
		retval = recvline(client_sock, buf, BUFSIZE);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		string tempBuf = buf;

		if (tempBuf.size() == 3) // Ŭ���̾�Ʈ���Լ� �ѱ��� �Է��� ������ ���
		{
			switch (tempBuf[0])
			{
			case 'H': // H �Է��� ������ ���

				// ��ɾ� �ȳ� ����
				for (int i = COMMANDINFO_1; i <= COMMANDINFO_11; i++)
				{
					send(client_sock, objPrintOrSend.sendMap[i].c_str(),
						(objPrintOrSend.sendMap[i]).size(), 0);
				}
				break;

			case 'X': // X �Է��� ������ ���
				bExit = true;
				break;

			default:
				break;
			}
		}
		else if (tempBuf.size() == 4) // Ŭ���̾�Ʈ���Լ� �α��� �Է��� ������ ���
		{
			if (tempBuf.substr(0, 2) == "US") // US �Է��� ������ ���
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

		// ���� ������ ���
		printf("%s:%d [%s] %s", addr, ntohs(clientaddr.sin_port), objUser.GetID().c_str(), buf);

		if (bExit)
		{
			objServerManager.GetUserInfoMap().erase(objUser.GetID()); // ���� ����� ������ ��ܿ��� ����
			printf("%s:%d closed\n", addr, ntohs(clientaddr.sin_port));
			break;
		}
	}

	// ���� �ݱ�
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

// ����� ���� ������ ���� �Լ�
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
		if (str[i] == delimiter) /* �����ڸ� ������ ��� push �� clear */
		{
			vs.push_back(temp);
			temp.clear();

			continue;
		}

		/* temp�� ���ڸ� �ϳ��� ��´�. */
		temp.push_back(str[i]);
	}

	/* ������ string�� push���� �ʾ����Ƿ� for�� �ۿ��� push */
	vs.push_back(temp);

	return vs;
}



