#pragma once

struct NetMessage
{
	char* buffer = nullptr;
	int size = 0;
	int offset = 0;

	void resize(int newSize);
	void free();

	// Writes data to the net message and increments the offset
	void write(const void* ptr, int writeSize);
	template<typename T>
	void write(T value)
	{
		write(&value, sizeof(T));
	}

	// Reads data from the net message and increments the offset
	void read(void* ptr, int readSize);
	template<typename T>
	T read()
	{
		T value;
		read(&value, sizeof(T));

		return value;
	}
};

enum class NetEventType : unsigned char
{
	Message,
	UserConnected,
	UserDisconnected,
};

struct NetEvent
{
	static NetEvent makeMessage(int userId, const NetMessage& message) { return { NetEventType::Message, userId, message }; }
	static NetEvent makeUserConnected(int userId) { return { NetEventType::UserConnected, userId, NetMessage() }; }
	static NetEvent makeUserDisconnected(int userId) { return { NetEventType::UserDisconnected, userId, NetMessage() }; }

	NetEventType type;

	// User whom this event concerns. For clients this will always be 0.
	int userId;
	NetMessage message;

	void free() { message.free(); }
};

void netInit();

void netPushEvent(const NetEvent& event);
bool netPollEvent(NetEvent* outEvent);