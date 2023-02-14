#pragma once

#include "Room.h"

#include <string>
#include <string.h>

#include <unordered_map>
#include <vector>

using namespace std;

class ServerManager
{
public:
	string LoginUsersInfo();

	unordered_map<string, string>& GetUserInfoMap();
	vector<Room>& GetRoomInfoVec();

private:
	unordered_map<string, string> m_userInfoMap; // key: ID, value: IP + port
	vector<Room> m_roomInfoVec;
};