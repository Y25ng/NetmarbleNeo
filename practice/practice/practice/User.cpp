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

void User::SetUserInfo(string idValue, string ipValue, string portValue)
{
	SetID(idValue);
	SetIP(ipValue);
	SetPortStr(portValue);
}



