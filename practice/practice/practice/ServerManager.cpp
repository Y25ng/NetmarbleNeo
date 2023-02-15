#include "ServerManager.h"

#include <string>
#include <string.h>

using namespace std;

string ServerManager::LoginUsersList()
{
	string tempStr = "------------------------- �̿��� ��� -------------------------\n\r";

	for (auto iter = m_userInfoMap.begin(); iter != m_userInfoMap.end(); ++iter)
	{
		tempStr += "  �̿���:";
		tempStr += iter->first;
		tempStr += "            ";
		tempStr += "������:  ";
		tempStr += iter->second.GetIP() + iter->second.GetPort();
		tempStr += "\n\r";
	}

	tempStr += "---------------------------------------------------------------\n\r";

	return tempStr;
}

string ServerManager::RoomList()
{
	string tempStr = "------------------------- ��ȭ�� ��� -------------------------\n\r";

	for (int i = 0; i < m_roomInfoVec.size(); i++)
	{
		tempStr += "[  " + to_string(m_roomInfoVec[i].GetRoomNumber()) + "] ";
		tempStr += "( " + to_string(m_roomInfoVec[i].GetParticipantNum()) + " / " + to_string(m_roomInfoVec[i].GetParticipantMaxNum()) + " ) ";
		tempStr += m_roomInfoVec[i].GetRoomTitle();
		tempStr += "\n\r";
	}

	tempStr += "---------------------------------------------------------------\n\r";

	return tempStr;

}

string ServerManager::RoomInfo()
{
	string tempStr = "------------------------- ��ȭ�� ���� -------------------------\n\r";

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