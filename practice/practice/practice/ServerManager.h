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
	string RoomInfo(int roomNumber);
	string UserInfo(string userID);

	unordered_map<string, User>& GetUserInfoMap();
	map<string, Room>& GetRoomInfoMap();

	void UserQuitServer(User& objUser);
	void DeleteUserInRoom(User& objUser);

private:

	unordered_map<string, User> m_userInfoMap; // key: ID, value: User Object
	map<string, Room> m_roomInfoMap; // key: Room Title, value: Room Object

};