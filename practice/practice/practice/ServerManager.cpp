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
		tempStr += iter->second.GetIP() + ":" + iter->second.GetPort();
		tempStr += "\n\r";
	}

	tempStr += "---------------------------------------------------------------\n\r";

	return tempStr;
}

string ServerManager::RoomList()
{
	string tempStr = "------------------------- ��ȭ�� ��� -------------------------\n\r";

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

string ServerManager::RoomInfo(int roomNumber)
{
	auto iter_objRoom = m_roomInfoMap.begin();

	for (int i = 0; i < roomNumber - 1; i++)
	{
		++iter_objRoom;
	}

	map<string, User>& temp_ParticipantList = iter_objRoom->second.GetParticipantMap();
	auto iter_tempList = temp_ParticipantList.begin();

	string tempStr = "------------------------- ��ȭ�� ���� -------------------------\n\r";

	// �� ����(�� ��ȣ, �ο���, �ִ� �ο���, �� �̸�)
	tempStr += "[ " + to_string(iter_objRoom->second.GetRoomNumber()) + "]  "
		+ "( " + to_string(iter_objRoom->second.GetParticipantNum()) + "/"
		+ to_string(iter_objRoom->second.GetParticipantMaxNum()) + " )  "
		+ iter_objRoom->second.GetRoomTitle() + "\n\r"
		+ "�����ð�:  " + iter_objRoom->second.GetCreateTime() + "\n\r";
		
	// ������ ����(���̵�, �����ð�)
	for (iter_tempList; iter_tempList != temp_ParticipantList.end(); ++iter_tempList)
	{
		tempStr += "������: " + iter_tempList->second.GetID() + "            �����ð�: " + iter_tempList->second.GetJoinTime() + "\n\r";
	}

	tempStr += +"---------------------------------------------------------------\n\r";

	return tempStr;
}

string ServerManager::UserInfo(string userID)
{
	string tempStr = "";

	tempStr += "** ������: "
		+ m_userInfoMap[userID].GetIP() + ":"
		+ m_userInfoMap[userID].GetPort() + "\n\r";

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
	m_userInfoMap.erase(objUser.GetID()); // ���� ����� ������ ��ܿ��� ����
	DeleteUserInRoom(objUser); // ���� ����� �� ������ ��ܿ��� ����
}

void ServerManager::DeleteUserInRoom(User& objUser)
{
	auto iter = m_roomInfoMap.find(objUser.GetRoomTitle());

	if (iter != m_roomInfoMap.end())
	{
		User delUser = objUser;
		m_roomInfoMap[iter->first].SetParticipantNum(m_roomInfoMap[iter->first].GetParticipantNum() - 1);
		m_roomInfoMap[iter->first].GetParticipantMap().erase(objUser.GetID());

		if (m_roomInfoMap[iter->first].GetParticipantMap().size() == 0)
		{
			m_roomInfoMap.erase(iter->first);
		}
	}
}