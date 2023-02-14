#include "ServerManager.h"

#include <string>
#include <string.h>

using namespace std;

string ServerManager::LoginUsersInfo()
{
	string tempStr = "------------------------- 이용자 목록 -------------------------\n\r";

	for (auto iter = m_userInfoMap.begin(); iter != m_userInfoMap.end(); ++iter)
	{	
		tempStr += "  이용자:";
		tempStr += iter->first;
		tempStr += "            ";
		tempStr += "접속지:  ";
		tempStr += iter->second;
		tempStr += "\n\r";
	}

	tempStr += "---------------------------------------------------------------\n\r";
	
	return tempStr;
}

unordered_map<string, string>& ServerManager::GetUserInfoMap() 
{ 
	return m_userInfoMap;
}

vector<Room>& ServerManager::GetRoomInfoVec()
{
	return m_roomInfoVec;
}