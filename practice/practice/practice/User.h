#pragma once

#include <string>
#include <string.h>

using namespace std;

class User
{
public:
	User(): 
		m_id(""), 
		m_ip(""), 
		m_port(""), 
		m_bInRoom(false), 
		m_sock(0), 
		m_roomNumber(0)
	{
	}

	string GetID();
	void SetID(string strValue);

	string GetIP();
	void SetIP(string strValue);

	string GetPort();
	void SetPortStr(string strValue);

	void SetUserInfo(string idValue, string ipValue, string portValue, unsigned int sockValue);

	bool GetbInRoom();
	void SetbInRoom(bool value);

	unsigned int GetSocket();
	void SetSocket(unsigned int value);

	string GetJoinTime();
	void SetJoinTime(string value);

	int GetRoomNumber();
	void SetRoomNumber(int value);

	const string& GetRoomTitle();
	void SetRoomTitle(const string& value);

	bool operator==(const User& user);

private:

	string m_id;
	string m_ip;
	string m_port;
	string m_joinTime;

	bool m_bInRoom;

	int m_roomNumber;
	string m_roomTitle;

	unsigned int m_sock;
};