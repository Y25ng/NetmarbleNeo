#pragma once

#include "User.h"

#include <string>
#include <string.h>
#include <list>

using namespace std;

class Room
{
public:

	Room()
	{
		m_roomNumber = - 1;
		m_participantNum = 0;
		m_participantNum = 0;
		m_participantMaxNum = 0;
	}

	void SetParticipantMaxNum(int value);
	int GetParticipantMaxNum();

	int GetParticipantNum();
	void SetParticipantNum(int value);

	string GetRoomTitle();
	void SetRoomTitle(string value);

	int GetRoomNumber();
	void SetRoomNumber(int value);

	list<User>& GetParticipantList();

private:

	int m_roomNumber;
	int m_participantNum;
	int m_participantMaxNum;
	string m_roomTitle;
	list<User> m_participantList;
};
