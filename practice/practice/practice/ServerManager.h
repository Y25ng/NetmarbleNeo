#pragma once

#include "Room.h"
#include "User.h"

#include <string>
#include <string.h>

#include <unordered_map>
#include <map>
#include <vector>

using namespace std;

class ServerManager
{
public:

	string LoginUsersList();
	string RoomList();
	string RoomInfo();

	unordered_map<string, User>& GetUserInfoMap();
	map<string, Room>& GetRoomInfoMap();

private:

	unordered_map<string, User> m_userInfoMap; // key: ID, value: IP + port
	map<string, Room> m_roomInfoMap;

};