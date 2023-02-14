#include "Room.h"

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

void Room::SetRoomNumber(int value)
{
	m_roomNumber = value;
}

void Room::SetRoomTitle(string value)
{
	m_roomTitle = value;
}