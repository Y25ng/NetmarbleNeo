#pragma once

#include "User.h"

#include <string>
#include <string.h>
#include <map>

using namespace std;

class Room
{
public:

	Room():
		m_roomNumber(-1),
		m_participantNum(0),
		m_participantMaxNum(0),
		m_roomTitle("")
	{
	}

	int GetRoomNumber();
	void SetRoomNumber(int value);

	int GetParticipantNum();
	void SetParticipantNum(int value);

	void SetParticipantMaxNum(int value);
	int GetParticipantMaxNum();

	const string& GetRoomTitle();
	void SetRoomTitle(string value);

	map<string, User>& GetParticipantMap();

	const string& GetCreateTime();
	void SetCreateTime(string& value);

	void SetRoomInfo(int roomNumber, int participantNum, int participantMaxNum, const string& roomTitle, const string& createTime);

	bool GetbFull();

private:

	int m_roomNumber;
	int m_participantNum;
	int m_participantMaxNum;
	string m_roomTitle;
	string m_createTime;
	map<string, User> m_participantMap;
};
