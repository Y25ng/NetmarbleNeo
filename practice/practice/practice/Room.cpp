#include "Room.h"


int Room::GetRoomNumber()
{
	return m_roomNumber;
}

void Room::SetRoomNumber(int value)
{
	m_roomNumber = value;
}

int Room::GetParticipantNum()
{
	return m_participantNum;
}

void Room::SetParticipantNum(int value)
{
	m_participantNum = value;
}

int Room::GetParticipantMaxNum()
{
	return m_participantMaxNum;
}

void Room::SetParticipantMaxNum(int value)
{
	m_participantMaxNum = value;
}

const string& Room::GetRoomTitle()
{
	return m_roomTitle;
}

void Room::SetRoomTitle(string value)
{
	m_roomTitle = value;
}

map<string, User>& Room::GetParticipantMap()
{
	return m_participantMap;
}

const string& Room::GetCreateTime()
{
	return m_createTime;
}

void Room::SetCreateTime(string& value)
{
	m_createTime = value;
}

void Room::SetRoomInfo(int roomNumber, int participantNum, int participantMaxNum, const string& roomTitle, const string& createTime)
{
	m_roomNumber = roomNumber;
	m_participantNum = participantNum;
	m_participantMaxNum = participantMaxNum;
	m_roomTitle  = roomTitle;
	m_createTime = createTime;
}

bool Room::GetbFull()
{
	return m_participantNum < m_participantMaxNum;
}