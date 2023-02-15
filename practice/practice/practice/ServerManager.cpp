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

map<string, Room>& ServerManager::GetRoomInfoMap()
{
	return m_roomInfoMap;
}

void ServerManager::UserQuitServer(User& objUser)
{
	m_userInfoMap.erase(objUser.GetID()); // 접속 종료시 접속자 명단에서 삭제

	auto iter = m_roomInfoMap.find(objUser.GetRoomTitle());

	if (iter != m_roomInfoMap.end())
	{
		User delUser = objUser;
		m_roomInfoMap[iter->first].SetParticipantNum(m_roomInfoMap[iter->first].GetParticipantNum() - 1);
		m_roomInfoMap[iter->first].GetParticipantMap().erase(objUser.GetID());

		if ( m_roomInfoMap[iter->first].GetParticipantMap().size() == 0 )
		{
			m_roomInfoMap.erase(iter->first);
		}
	}
}