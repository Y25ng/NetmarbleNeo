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
	int GetParticipantMaxNum();

	int GetParticipantNum();
	void SetParticipantNum(int value);

	string GetRoomTitle();
	void SetRoomTitle(string value);

	int GetRoomNumber();
	void SetRoomNumber(int value);


private:
	int m_roomNumber;
	int m_participantNum;
	int m_participantMaxNum;
	string m_roomTitle;
};
