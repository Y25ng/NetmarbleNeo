#pragma once

#include <string>
#include <string.h>

#include <unordered_map>

using namespace std;

enum ePrintMapKey
{
	INITIALIZE = 0,
};

enum eSendMapKey
{
	WELCOME = 0,
	REQUESTLOGIN,

	COMPLETELOGIN_1,
	COMPLETELOGIN_2,
	COMPLETELOGIN_3,
	COMPLETELOGIN_4,
	COMPLETELOGIN_5,
	COMPLETELOGIN_6,

	FIRSTINFO,

	COMMANDINFO_1,
	COMMANDINFO_2,
	COMMANDINFO_3,
	COMMANDINFO_4,
	COMMANDINFO_5,
	COMMANDINFO_6,
	COMMANDINFO_7,
	COMMANDINFO_8,
	COMMANDINFO_9,
	COMMANDINFO_10,
	COMMANDINFO_11,

};

struct sPrintOrSend
{
	sPrintOrSend()
	{
		printMap[ePrintMapKey::INITIALIZE]
			= "server initialized successfully.\nall client message would be printed in console window.\nwaiting for client..\n";

		////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////

		sendMap[eSendMapKey::WELCOME]
			= "** �ȳ��ϼ���.�ؽ�Ʈ ä�� ���� ver 0.1�Դϴ�.\n\r";

		sendMap[eSendMapKey::REQUESTLOGIN]
			= "** �α��� ���ɾ�(LOGIN)�� ������ּ���.\n\r";

		sendMap[eSendMapKey::COMPLETELOGIN_1]
			= "---------------------------------------------------------------\n\r";
		sendMap[eSendMapKey::COMPLETELOGIN_2]
			= "�ݰ����ϴ�. �ؽ�Ʈ ä�� ���� ver 0.1 �Դϴ�.\n\n\r";
		sendMap[eSendMapKey::COMPLETELOGIN_3]
			= "�̿��� �����Ͻ� ���� ������ �Ʒ� �̸��Ϸ� ���� �ٶ��ϴ�.\n\r�����մϴ�.\n\n\r";
		sendMap[eSendMapKey::COMPLETELOGIN_4]
			= "                                programmed & arranged by oranze\n\r";
		sendMap[eSendMapKey::COMPLETELOGIN_5]
			= "                                    email: oranze@softweb.co.kr\n\r";
		sendMap[eSendMapKey::COMPLETELOGIN_6]
			= "---------------------------------------------------------------\n\r";

		sendMap[eSendMapKey::FIRSTINFO]
			= "���ɾ�ȳ�(H) ����(X)\n\r����> ";

		sendMap[eSendMapKey::COMMANDINFO_1]
			= "---------------------------------------------------------------\n\r";
		sendMap[eSendMapKey::COMMANDINFO_2]
			= "H                         ���ɾ� �ȳ�\n\r";
		sendMap[eSendMapKey::COMMANDINFO_3]
			= "US                        �̿��� ��� ����\n\r";
		sendMap[eSendMapKey::COMMANDINFO_4]
			= "LT                        ��ȭ�� ��� ����\n\r";
		sendMap[eSendMapKey::COMMANDINFO_5]
			= "ST [���ȣ]               ��ȭ�� ���� ����\n\r";
		sendMap[eSendMapKey::COMMANDINFO_6]
			= "PF [����ID]             �̿��� ���� ����\n\r";
		sendMap[eSendMapKey::COMMANDINFO_7]
			= "TO [����ID] [�޽���]    ���� ������\n\r";
		sendMap[eSendMapKey::COMMANDINFO_8]
			= "O  [�ִ��ο�] [������]    ��ȭ�� �����\n\r";
		sendMap[eSendMapKey::COMMANDINFO_9]
			= "J  [���ȣ]               ��ȭ�� �����ϱ�\n\r";
		sendMap[eSendMapKey::COMMANDINFO_10]
			= "X                         ������\n\r";
		sendMap[eSendMapKey::COMMANDINFO_11]
			= "---------------------------------------------------------------\n\r";

	}

	unordered_map<int, string> printMap;
	unordered_map<int, string> sendMap;
};
sPrintOrSend objPrintOrSend;


struct USER
{
	string id;
};



