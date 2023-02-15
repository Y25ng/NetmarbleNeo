#pragma once

#include <string>
#include <string.h>

using namespace std;

class User
{
public:
	User() : m_id(""), m_ip(""), m_port(""), m_bInRoom(false), m_sock(0)
	{
	}

	string GetID();
	void SetID(string strValue);

	string GetIP();
	void SetIP(string strValue);

	string GetPort();
	void SetPortStr(string strValue);

	void SetUserInfo(string idValue, string ipValue, string portValue, int sockValue);

	bool GetbInRoom();
	void SetbInRoom(bool value);

	unsigned int GetSocket();
	void SetSocket(unsigned int value);

private:
	string m_id;
	string m_ip;
	string m_port;

	bool m_bInRoom;

	unsigned int m_sock;
};