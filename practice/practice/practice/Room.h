#pragma once

#include <string>
#include <string.h>

using namespace std;

class Room
{
public:
	Room()
	{
		m_participantNum = 0;
		m_participantNum = 0;
		m_participantMaxNum = 0;
	}

	void SetParticipantMaxNum(int value);

	int GetParticipantNum();
	void SetParticipantNum(int value);

	void SetRoomTitle(string value);

	void SetRoomNumber(int value);

private:
	int m_roomNumber;
	int m_participantNum;
	int m_participantMaxNum;
	string m_roomTitle;
};
