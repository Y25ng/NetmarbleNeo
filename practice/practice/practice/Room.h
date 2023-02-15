#pragma once

#include "User.h"

#include <string>
#include <string.h>
#include <list>

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

	string& GetRoomTitle();
	void SetRoomTitle(string value);

	list<User>& GetParticipantList();

	void SetRoomInfo(int roomNumber, int participantNum, int participantMaxNum, string& roomTitle);

	bool GetbFull();

private:

	int m_roomNumber;
	int m_participantNum;
	int m_participantMaxNum;
	string m_roomTitle;
	list<User> m_participantList;
};
