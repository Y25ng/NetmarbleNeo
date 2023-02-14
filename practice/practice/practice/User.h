#pragma once

#include <string>
#include <string.h>


using namespace std;

class User
{
public:
	string GetID();
	void SetID(string strValue);

	string GetIP();
	void SetIP(string strValue);

	string GetPort();
	void SetPortStr(string strValue);

	void SetUserInfo(string idValue, string ipValue, string portValue);

private:
	string m_id;
	string m_ip;
	string m_port;
};