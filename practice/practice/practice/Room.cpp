#include "Room.h"


int Room::GetParticipantMaxNum()
{
	return m_participantMaxNum;
}

void Room::SetParticipantMaxNum(int value)
{
	m_participantMaxNum = value;
}


void Room::SetParticipantNum(int value)
{
	m_participantNum = value;
}

int Room::GetParticipantNum()
{
	return m_participantNum;
}

int Room::GetRoomNumber()
{
	return m_roomNumber;
}

void Room::SetRoomNumber(int value)
{
	m_roomNumber = value;
}

string Room::GetRoomTitle()
{
	return m_roomTitle;
}

void Room::SetRoomTitle(string value)
{
	m_roomTitle = value;
}