#include "User.h"

using namespace std;

string User::GetID()
{
	return m_id;
}
void User::SetID(string strValue)
{
	m_id = strValue;
}

string User::GetIP()
{
	return m_ip;
}
void User::SetIP(string strValue)
{
	m_ip = strValue;
}

string User::GetPort()
{
	return m_port;
}

void User::SetPortStr(string strValue)
{
	m_port = strValue;
}

void User::SetUserInfo(string idValue, string ipValue, string portValue, int sockValue)
{
	SetID(idValue);
	SetIP(ipValue);
	SetPortStr(portValue);
	SetSocket(sockValue);
}

bool User::GetbInRoom()
{
	return m_bInRoom;
}

void User::SetbInRoom(bool value)
{
	m_bInRoom = value;
}

unsigned int User::GetSocket()
{
	return m_sock;
}

void User::SetSocket(unsigned int value)
{
	m_sock = value;
}

string User::GetJoinTime()
{
	return m_joinTime;
}

void User::SetJoinTime(string value)
{
	m_joinTime = value;
}

int User::GetRoomNumber()
{
	return m_roomNumber;
}

void User::SetRoomNumber(int value)
{
	m_roomNumber = value;
}




