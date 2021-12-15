#include "Network.h"
#include "Engine.h"
#include <WinSock2.h>
#include <vector>

std::vector<NetEvent> eventQueue;
HANDLE eventMutex;

void NetMessage::resize(int newSize)
{
	char* newBuffer = (char*)malloc(newSize);
	if (buffer)
	{
		memcpy(newBuffer, buffer, size);
		::free(buffer);
	}

	buffer = newBuffer;
	size = newSize;
}

void NetMessage::free()
{
	if (buffer)
		::free(buffer);

	buffer = nullptr;
	size = 0;
}

void NetMessage::write(const void* ptr, int writeSize)
{
	resize(offset + writeSize);
	memcpy(buffer + offset, ptr, writeSize);

	offset += writeSize;
}

void NetMessage::read(void* ptr, int readSize)
{
	// Boundary check
	if (offset + readSize > size)
	{
		engError("NetMessage::read caused an out-of-bound error\nMessage size: %d\nTried to read %d bytes at offset %d",
			size, readSize, offset);
		return;
	}

	memcpy(ptr, buffer + offset, readSize);
	offset += readSize;
}

void netInit()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	eventMutex = CreateMutexA(nullptr, false, nullptr);
}

void netPushEvent(const NetEvent& event)
{
	WaitForSingleObject(eventMutex, INFINITE);
	eventQueue.push_back(event);
	ReleaseMutex(eventMutex);
}

bool netPollEvent(NetEvent* outEvent)
{
	WaitForSingleObject(eventMutex, INFINITE);

	if (eventQueue.size() == 0)
	{
		ReleaseMutex(eventMutex);
		return false;
	}

	*outEvent = eventQueue[0];
	eventQueue.erase(eventQueue.begin());

	ReleaseMutex(eventMutex);
	return true;
}