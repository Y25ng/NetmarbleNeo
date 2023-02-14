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
			= "** 안녕하세요.텍스트 채팅 서버 ver 0.1입니다.\n\r";

		sendMap[eSendMapKey::REQUESTLOGIN]
			= "** 로그인 명령어(LOGIN)를 사용해주세요.\n\r";

		sendMap[eSendMapKey::COMPLETELOGIN_1]
			= "---------------------------------------------------------------\n\r";
		sendMap[eSendMapKey::COMPLETELOGIN_2]
			= "반갑습니다. 텍스트 채팅 서버 ver 0.1 입니다.\n\n\r";
		sendMap[eSendMapKey::COMPLETELOGIN_3]
			= "이용중 불편하신 점이 있으면 아래 이메일로 문의 바랍니다.\n\r감사합니다.\n\n\r";
		sendMap[eSendMapKey::COMPLETELOGIN_4]
			= "                                programmed & arranged by oranze\n\r";
		sendMap[eSendMapKey::COMPLETELOGIN_5]
			= "                                    email: oranze@softweb.co.kr\n\r";
		sendMap[eSendMapKey::COMPLETELOGIN_6]
			= "---------------------------------------------------------------\n\r";

		sendMap[eSendMapKey::FIRSTINFO]
			= "명령어안내(H) 종료(X)\n\r선택> ";

		sendMap[eSendMapKey::COMMANDINFO_1]
			= "---------------------------------------------------------------\n\r";
		sendMap[eSendMapKey::COMMANDINFO_2]
			= "H                         명령어 안내\n\r";
		sendMap[eSendMapKey::COMMANDINFO_3]
			= "US                        이용자 목록 보기\n\r";
		sendMap[eSendMapKey::COMMANDINFO_4]
			= "LT                        대화방 목록 보기\n\r";
		sendMap[eSendMapKey::COMMANDINFO_5]
			= "ST [방번호]               대화방 정보 보기\n\r";
		sendMap[eSendMapKey::COMMANDINFO_6]
			= "PF [상대방ID]             이용자 정보 보기\n\r";
		sendMap[eSendMapKey::COMMANDINFO_7]
			= "TO [상대방ID] [메시지]    쪽지 보내기\n\r";
		sendMap[eSendMapKey::COMMANDINFO_8]
			= "O  [최대인원] [방제목]    대화방 만들기\n\r";
		sendMap[eSendMapKey::COMMANDINFO_9]
			= "J  [방번호]               대화방 참여하기\n\r";
		sendMap[eSendMapKey::COMMANDINFO_10]
			= "X                         끝내기\n\r";
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




