#include "ServerManager.h"

#include <string>
#include <string.h>

using namespace std;

string ServerManager::LoginUsersList()
{
	string tempStr = "------------------------- 이용자 목록 -------------------------\n\r";

	for (auto iter = m_userInfoMap.begin(); iter != m_userInfoMap.end(); ++iter)
	{
		tempStr += "  이용자:";
		tempStr += iter->first;
		tempStr += "            ";
		tempStr += "접속지:  ";
		tempStr += iter->second.GetIP() + ":" + iter->second.GetPort();
		tempStr += "\n\r";
	}

	tempStr += "---------------------------------------------------------------\n\r";

	return tempStr;
}

string ServerManager::RoomList()
{
	string tempStr = "------------------------- 대화방 목록 -------------------------\n\r";

	int cnt = 1;

	for (auto iter = m_roomInfoMap.begin(); iter != m_roomInfoMap.end(); ++iter)
	{
		tempStr += "[  " + to_string(cnt) + "] ";
		tempStr += "( " + to_string(iter->second.GetParticipantNum()) + " / " + to_string(iter->second.GetParticipantMaxNum()) + " ) ";
		tempStr += iter->second.GetRoomTitle();
		tempStr += "\n\r";

		++cnt;
	}

	tempStr += "---------------------------------------------------------------\n\r";

	return tempStr;

}

string ServerManager::RoomInfo()
{
	string tempStr = "------------------------- 대화방 정보 -------------------------\n\r";

	return tempStr;
}

unordered_map<string, User>& ServerManager::GetUserInfoMap()
{
	return m_userInfoMap;
}

vector<Room>& ServerManager::GetRoomInfoVec()
{
	return m_roomInfoVec;
}

map<string, Room>& ServerManager::GetRoomInfoMap()
{
	return m_roomInfoMap;
}