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

// ���� ������ �Լ�
int _recv_ahead(SOCKET s, char* p);

// ����� ���� ������ ���� �Լ�
int recvline(SOCKET s, char* buf, int maxlen);

vector<string> split(string str, char delimiter);

// Ŭ���̾�Ʈ�� ������ ���
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
	// ���� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		return true;

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
	return false;
}


string Login(SOCKET client_sock, int& retval, char* buf, char* addr, struct sockaddr_in clientaddr, User* objUser)
{
	// �α��� ��û ���� Ŭ���̾�Ʈ���� ����
	SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::REQUESTLOGIN]);

	while (true)
	{
		bool bAlreadyExistID = false;
		string tempBufStr = "";

		// ������ �ޱ�
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

		// Ŭ���̾�Ʈ �Է� ������ �α��� ������ �������� Ȯ�� => �����ϸ� �α��� ���� ����
		if (tempBufStr.substr(0, 5) == "LOGIN" && tempBufStr[5] == ' ' && tempBufStr.size() >= eCommand::MINLENGTHLOGINPOSSIBLE)
		{
			// ���� ������ ���ϴ� ������ �Է� ID�� ���� ������ ID�� ��ġ���� Ȯ�� 
			bAlreadyExistID = objServerManager.GetUserInfoMap().find(tempBufStr.substr(6, tempBufStr.size() - eCommand::MINLENGTHLOGINPOSSIBLE-1)) != objServerManager.GetUserInfoMap().end();

			if (bAlreadyExistID) // ���� ������ ID�� ��ĥ ��
			{
				// ���� ������ ���ϴ� �������� �̹� �����ϴ� ID���� �˷��ִ� �ȳ� ���� ����
				SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::ALREADYEXISTID]);
			}
			else // ���� ������ ID�� ��ġ�� ���� ��
			{

				// ���� ��ü�� ���� ���� ���� (id, ip, port, socket)
				objUser->SetUserInfo(tempBufStr.substr(6, tempBufStr.size() - eCommand::MINLENGTHLOGINPOSSIBLE - 1), addr, to_string(ntohs(clientaddr.sin_port)), (unsigned int)client_sock);

				// �������� ���� �Է� ���� ���
				printf("%s:%d [] %s", addr, ntohs(clientaddr.sin_port), buf);

				// ServerManager���� �����ϴ� �������� �ʿ� ���� ��ü ����
				// key: ���� ID
				// value: ���� Ŭ���� ��ü
				objServerManager.GetUserInfoMap()[objUser->GetID()] = *objUser;

				break;
			}
		}

		// �������� ���� �Է� ���� ���
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

	// ���� ���� ���� �÷��� ����
	bool bExit = false; 


	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	// ä�ü��� �λ縻 Ŭ���̾�Ʈ���� ����
	SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::WELCOME]);

	// ���� �α��� ID
	string tempID = Login(client_sock, retval, buf, addr, clientaddr, &objUser);

	// ä�� ���� ���� �λ縻 Ŭ���̾�Ʈ���� ����
	for (int i = eSendMapKey::COMPLETELOGIN_1; i <= eSendMapKey::COMPLETELOGIN_6; ++i)
	{
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[i]);
	}

	while (true)
	{
		// ���� H and X ���� Ŭ���̾�Ʈ���� ����
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::FIRSTINFO]);

		// ������ �ޱ�
		retval = recvline(client_sock, buf, BUFSIZE);
		if (retval == SOCKET_ERROR) 
		{
			// ���� ����� ������ ��ܿ��� ���� 
			// ���� ����� �� ��ܿ��� ����
			objServerManager.UserQuitServer(objUser); 

			err_display("recv()");
			break;
		}
		else if (retval == eRet::EMPTYMESSAGE)
		{
			// ���� ����� ������ ��ܿ��� ���� 
			// ���� ����� �� ��ܿ��� ����
			objServerManager.UserQuitServer(objUser);

			break;
		}

		string tempBuf = buf;

		if (tempBuf.size() == eCommand::ONLYCOMMAND_ONE) // Ŭ���̾�Ʈ���Լ� �ѱ��� ����� ������ ���
		{
			switch (tempBuf[0])
			{
			case 'H': // H �Է��� ������ ���

				// ��ɾ� �ȳ� ����
				SCommand::Command_H_Func((unsigned int)client_sock);

				break;

			case 'X': // X �Է��� ������ ���
				bExit = true;
				break;

			default:
				break;
			}
		}
		else if (tempBuf.size() == eCommand::ONLYCOMMAND_TWO) // Ŭ���̾�Ʈ���Լ� �α��� ����� ������ ���
		{
			if (tempBuf.substr(0, 2) == "US") // US �Է��� ������ ���
			{
				// �̿��� ��� Ŭ���̾�Ʈ���� ����
				SCommand::Command_US_Func((unsigned int)client_sock);
			}
			else if (tempBuf.substr(0, 2) == "LT") // LT �Է��� ������ ���
			{
				// ��ȭ�� ��� ��� Ŭ���̾�Ʈ���� ����
				SCommand::Command_LT_Func((unsigned int)client_sock);
			}
		}
		else // Ŭ���̾�Ʈ���Լ� ������ �̻��� ����� ������ ���
		{
			// Ŭ���̾�Ʈ �Է� ���� ���� �������� �и��Ͽ� ������ �����̳�
			vector<string> tempBufSplit_Vec;

			// Ŭ���̾�Ʈ �Է� �� �и� �� ����
			tempBufSplit_Vec = Util::split(tempBuf, ' ');

			// 1�� ��ɰ� 1�� �̻��� ���ڰ��� ����
			if (tempBufSplit_Vec.size() >= eCommand::ONECOMMANDONEPARAMETER)
			{
				// ���: O && 2�� �̻��� ���ڰ��� ���� && �ִ� �ο����� ���ڰ� �ƴ� ���ڰ� ������ �ʾ��� ��
				// => �� ���� ���
				if (tempBufSplit_Vec[0] == "O" && tempBufSplit_Vec.size() >= eCommand::ONECOMMANDTWOPARAMETER && Util::bCanUsestoi(tempBufSplit_Vec[1]))
				{
					SCommand::Command_O_Func(tempBufSplit_Vec, objUser, (unsigned int)client_sock, addr, clientaddr, buf, retval);
				}
				else if (tempBufSplit_Vec[0] == "ST" && tempBufSplit_Vec.size() == eCommand::ONECOMMANDONEPARAMETER) // ��ȭ�� ����
				{
					SCommand::Command_ST_Func((unsigned int)client_sock, tempBufSplit_Vec);
				}
				else if (tempBufSplit_Vec[0] == "J" && tempBufSplit_Vec.size() == eCommand::ONECOMMANDONEPARAMETER) // ��ȭ�� ����
				{
					SCommand::Command_J_Func(tempBufSplit_Vec, objUser, (unsigned int)client_sock, addr, clientaddr, buf, retval);
				}
				else if (tempBufSplit_Vec[0] == "PF" && tempBufSplit_Vec.size() == eCommand::ONECOMMANDONEPARAMETER) // �̿��� ���� Ȯ��
				{
					SCommand::Command_PF_Func(tempBufSplit_Vec, (unsigned int)client_sock, objUser);
				}
				else if (tempBufSplit_Vec[0] == "TO" && tempBufSplit_Vec.size() >= eCommand::ONECOMMANDTWOPARAMETER) // �������� ����������
				{
					SCommand::Command_TO_Func(tempBufSplit_Vec, (unsigned int)client_sock, objUser);
				}
			}
		}

		// ���� ������ ���
		printf("%s:%d [%s] %s", addr, ntohs(clientaddr.sin_port), objUser.GetID().c_str(), buf);

		if (bExit)
		{
			break;
		}
	}

	// ���� ����� ������ ��ܿ��� ���� 
	// ���� ����� �� ��ܿ��� ����
	objServerManager.UserQuitServer(objUser);

	printf("%s:%d closed\n", addr, ntohs(clientaddr.sin_port));

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

void SendMessageToClient(SOCKET client_socket, string& message)
{
	send(client_socket, message.c_str(), (int)message.size(), 0);
}

void SCommand::Chatting_By_OorJ(User& objUser, Room& objRoom, unsigned int client_sock, char* addr, struct sockaddr_in clientaddr, char* buf, int retval)
{
	while (true)
	{
		// ������ �ޱ�
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
			// ���� ����� ������ ��ܿ��� ���� 
			// ���� ����� �� ��ܿ��� ����
			objServerManager.UserQuitServer(objUser);
			break;
		}

		auto iter_objRoom = objServerManager.GetRoomInfoMap().end();
		--iter_objRoom;
		objRoom = iter_objRoom->second;
		string tempMessage = objUser.GetID() + "> " + buf;

		// �濡 �ִ� ��� �������� �޽��� ����
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
	// ��ȭ�� ��� ��� Ŭ���̾�Ʈ���� ����
	string tempStr = objServerManager.RoomList();
	SendMessageToClient(client_sock, tempStr);
}

void SCommand::Command_O_Func(vector<string>& tempBufSplit_Vec, User& objUser, unsigned int client_sock, char* addr, struct sockaddr_in clientaddr, char* buf, int retval)
{
	// �� �ִ� �ο���  
	int tempMaxNum = stoi(tempBufSplit_Vec[1]);

	// �� ����
	string tempRoomTitle = "";

	for (int i = 2; i < tempBufSplit_Vec.size(); i++)
	{
		tempRoomTitle += tempBufSplit_Vec[i] + "";
	}

	tempRoomTitle = tempRoomTitle.substr(0, tempRoomTitle.size() - 1);

	// �� �ִ� �ο��� ���� ���� && �� ������ ������ �ƴ� ��
	if (tempMaxNum >= eCommand::MINNUMFORROOM && tempMaxNum <= eCommand::MAXNUMFORROOM && tempRoomTitle != "\r")
	{
		Room objRoom;

		//  �� ���� ���� �� ���� ���� ����
		objRoom.SetRoomInfo((int)objServerManager.GetRoomInfoMap().size() + 1, 1, tempMaxNum, tempRoomTitle);
		objUser.SetbInRoom(true);
		objUser.SetRoomNumber(objRoom.GetRoomNumber());
		objUser.SetRoomTitle(objRoom.GetRoomTitle());
		objRoom.GetParticipantMap().emplace(objUser.GetID(), objUser);

		objServerManager.GetRoomInfoMap().emplace(objRoom.GetRoomTitle(), objRoom);

		SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::CREATEROOM]);

		string tempStr = "** " + objUser.GetID() + "���� �����̽��ϴ�. " + "(�����ο� 1/" + to_string(objRoom.GetParticipantMaxNum()) + ")\n\r";
		SendMessageToClient(client_sock, tempStr);
		printf("%s:%d [%s] %s", addr, ntohs(clientaddr.sin_port), objUser.GetID().c_str(), buf);

		Chatting_By_OorJ(objUser, objRoom, (unsigned int)client_sock, addr, clientaddr, buf, retval); // ä�ù� �������� ä�� �Լ�
	}
	else // �� ���� ���ǿ� �������� ���� ���
	{
		if (!(tempMaxNum >= eCommand::MINNUMFORROOM && tempMaxNum <= eCommand::MAXNUMFORROOM)) // �� �ִ� �ο����� �߸� �Էµ� ���
		{
			SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::CANTCREATEROOMBYNUM]);
		}
		else // �� ������ �߸� �Էµ� ���
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
		// �Է¹��� �� ��ȣ
		int tempRoomNum = stoi(tempBufSplit_Vec[1]);

		// �Է¹��� �� ��ȣ�� ���� ������ ���
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

			// �� ����(�� ��ȣ, �ο���, �ִ� �ο���, �� �̸�)
			tempStr = "[ " + to_string(iter_objRoom->second.GetRoomNumber()) + "]  "
				+ "( " + to_string(iter_objRoom->second.GetParticipantNum()) + "/"
				+ to_string(iter_objRoom->second.GetParticipantMaxNum()) + " )  "
				+ iter_objRoom->second.GetRoomTitle() + "\n\r"
				+ "�����ð�:  \n\r";

			// ������ ����(���̵�, �����ð�)
			for (int i = 0; i < temp_ParticipantList.size(); i++)
			{
				tempStr += "������: " + iter_tempList->second.GetID() + "            �����ð� :\n\r";
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
		// �Է¹��� �� ��ȣ
		int tempRoomNum = stoi(tempBufSplit_Vec[1]);

		if (objServerManager.GetRoomInfoMap().size() >= tempRoomNum)
		{
			auto iter_objRoomInfo = objServerManager.GetRoomInfoMap().begin();

			for (int i = 0; i < tempRoomNum - 1; i++)
			{
				++iter_objRoomInfo;
			}

			// ������ �Է��� ��ȣ�� �ش��ϴ� �� ��ü
			Room& temp_objRoom = iter_objRoomInfo->second;

			// ���� �� ���� �ʾҴٸ�
			if (temp_objRoom.GetbFull())
			{
				temp_objRoom.SetParticipantNum(temp_objRoom.GetParticipantNum() + 1);
				temp_objRoom.GetParticipantMap().emplace(objUser.GetID(), objUser);
				objUser.SetbInRoom(true);
				objUser.SetRoomNumber(temp_objRoom.GetRoomNumber());
				objUser.SetRoomTitle(temp_objRoom.GetRoomTitle());

				string tempStr = "** " + objUser.GetID() + "���� �����̽��ϴ�. " + "(�����ο�" + to_string(temp_objRoom.GetParticipantNum()) + "/ " + to_string(temp_objRoom.GetParticipantMaxNum()) + ")\n\r";
				printf("%s:%d [%s] %s", addr, ntohs(clientaddr.sin_port), objUser.GetID().c_str(), buf);

				// �濡 �ִ� ��� �ο����� ���� ��ȭ�� ���� �޽��� ����
				for (auto iter_userInRoom = temp_objRoom.GetParticipantMap().begin(); iter_userInRoom != temp_objRoom.GetParticipantMap().end(); ++iter_userInRoom)
				{
					unsigned int tempUserSock = iter_userInRoom->second.GetSocket();
					SendMessageToClient(tempUserSock, tempStr);
				}

				Chatting_By_OorJ(objUser, temp_objRoom, (unsigned int)client_sock, addr, clientaddr, buf, retval); // ä�ù� �������� ä�� �Լ�
			}
			else
			{
				// ��ȭ���� �� á�ٴ� �޽��� ����
				SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::ROOMFULL]);
			}
		}
	}
}

void SCommand::Command_PF_Func(vector<string>& tempBufSplit_Vec, unsigned int client_sock, User& objUser)
{
	// �������� �Է¹��� ID
	string tempUserID = tempBufSplit_Vec[1].substr(0, tempBufSplit_Vec[1].size() - 1);

	// �������� ������ ã�� ID�� ���� ���
	if (objServerManager.GetUserInfoMap().find(tempUserID) == objServerManager.GetUserInfoMap().end())
	{
		string tempStr = "** " + tempUserID + "���� ã�� �� �����ϴ�.\n\r";
		SendMessageToClient(client_sock, tempStr);
	}
	else // �������� ������ ã�� ID�� ���� ���
	{
		if (objUser.GetbInRoom()) // �ش� �����ڰ� ä�ù濡 �������ִ� ���
		{
			string tempStr = "** " + tempUserID + "���� ���� ä�ù濡 �ֽ��ϴ�.\n\r";

			tempStr += "** ������: "
				+ objServerManager.GetUserInfoMap()[tempUserID].GetIP() + ":"
				+ objServerManager.GetUserInfoMap()[tempUserID].GetPort() + "\n\r";

			SendMessageToClient(client_sock, tempStr);
		}
		else // �ش� �����ڰ� ���ǿ� �ִ� ���
		{
			string tempStr = "** " + tempUserID + "���� ���� ���ǿ� �ֽ��ϴ�.\n\r";

			tempStr += "** ������: "
				+ objServerManager.GetUserInfoMap()[tempUserID].GetIP() + ":"
				+ objServerManager.GetUserInfoMap()[tempUserID].GetPort() + "\n\r";

			SendMessageToClient(client_sock, tempStr);
		}
	}
}

void SCommand::Command_TO_Func(vector<string>& tempBufSplit_Vec, unsigned int client_sock, User& objUser)
{
	// �������� �Է¹��� ID
	string tempRecvUserID = tempBufSplit_Vec[1];

	if (tempRecvUserID == objUser.GetID()) // �ڱ� �ڽſ��� ������ �������� �ϴ� ���
	{
		// �ش� �������� �ڱ� �ڽſ��Դ� ������ ���� �� ���ٴ� ���� ����
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::CANNOTSENDME]);
	}
	else if (objServerManager.GetUserInfoMap().find(tempRecvUserID) == objServerManager.GetUserInfoMap().end()) // ���������� ���� �������� ������ �������� �ϴ� ���
	{
		// �ش� �������� Ÿ�� ������ ���������� �ʴٴ� ���� ����
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::CANNOTFINDUSER]);
	}
	else // �ش� ������ ������ ���� ��
	{
		string tempMessage = "\n\r# " + objUser.GetID() + "���� ���� ==> ";

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

		// ���� ���� ���� �����ڿ� �۽��ڿ��� ��� ����
		SendMessageToClient(client_sock, objPrintOrSend.sendMap[eSendMapKey::SENDLETTER]);
		SendMessageToClient(objServerManager.GetUserInfoMap()[tempRecvUserID].GetSocket(), tempMessage);

		// ���� H or X ���� Ŭ���̾�Ʈ���� ����
		SendMessageToClient(objServerManager.GetUserInfoMap()[tempRecvUserID].GetSocket(), objPrintOrSend.sendMap[eSendMapKey::FIRSTINFO]);
	}
}
