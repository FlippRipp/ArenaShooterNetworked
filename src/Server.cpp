#include "Server.h"
#include "Network.h"
#include "Engine.h"
#include <vector>

SOCKET acceptSocket = INVALID_SOCKET;

enum class UserState
{
	// User slot not used
	Inactive,
	// User connected, but haven't been accepted on the game thread yet
	Pending,
	// Accepted, ready for work
	Active
};

struct User
{
	UserState state = UserState::Inactive;
	int index;
	SOCKET socket;
};
User users[SERVER_CAPACITY];

static DWORD recvWorker(void* ptr)
{
	User* user = (User*)ptr;
	int id = (int)(user - users);

	while(true)
	{
		int msgSize = 0;
		recv(user->socket, (char*)&msgSize, 4, MSG_WAITALL);

		char* msgBuffer = (char*)malloc(msgSize);
		int result = recv(user->socket, msgBuffer, msgSize, MSG_WAITALL);
		if (result == 0 || result == SOCKET_ERROR)
		{
			users[id].state = UserState::Inactive;
			closesocket(users[id].socket);
			free(msgBuffer);

			// Add user event
			netPushEvent(NetEvent::makeUserDisconnected(id));
			return 0;
		}

		// Create a message for this
		NetMessage message;
		message.buffer = msgBuffer;
		message.size = msgSize;

		netPushEvent(NetEvent::makeMessage(id, message));
	}
}

static DWORD acceptWorker(void*)
{
	while(true)
	{
		SOCKET newClient = accept(acceptSocket, nullptr, 0);

		// Find available id for this client
		int id = -1;
		for(int i = 0; i < SERVER_CAPACITY; ++i)
		{
			if (users[i].state != UserState::Inactive)
				continue;

			id = i;
			break;
		}

		// Server is full
		if (id == -1)
		{
			closesocket(newClient);
			continue;
		}

		users[id].state = UserState::Pending;
		users[id].socket = newClient;

		// Add user event
		netPushEvent(NetEvent::makeUserConnected(id));
	}
}

bool serverStartup(unsigned short port)
{
	// On server, setup accept socket and worker
	acceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = INADDR_ANY;
	bindAddr.sin_port = htons(port);

	if (bind(acceptSocket, (sockaddr*)&bindAddr, sizeof(bindAddr)))
	{
		engError("Binding socket to port %hd failed", port);
		closesocket(acceptSocket);
		acceptSocket = INVALID_SOCKET;

		return false;
	}

	listen(acceptSocket, 10);

	// Spin up accept thread
	CreateThread(
		0, 0,
		acceptWorker,
		&acceptSocket,
		0, 0
	);

	return true;
}

bool serverIsStarted()
{
	return acceptSocket != INVALID_SOCKET;
}

void serverAcceptUser(int userId)
{
	if (!serverIsStarted())
	{
		engError("serverAcceptUser called on non-server");
		return;
	}

	// The reason for this function is that we want to give the game thread a chance
	//	to respond to a connection before messages start being sent/received
	// Otherwise game messages might become out-of-order
	if (users[userId].state != UserState::Pending)
	{
		engError("Tried to accept user %d, but they don't have a pending connection", userId);
		return;
	}

	// Activate user and spin up receive thread
	users[userId].state = UserState::Active;
	CreateThread(
		0, 0,
		recvWorker,
		&users[userId],
		0, 0
	);
}

void serverKickUser(int userId)
{
	if (!serverIsStarted())
	{
		engError("serverKickUser called on non-server");
		return;
	}

	if (users[userId].state == UserState::Inactive)
	{
		engError("Tried to kick user %d, but they aren't connected", userId);
		return;
	}

	users[userId].state = UserState::Inactive;
	closesocket(users[userId].socket);
}

void serverSendTo(const NetMessage& message, int userId)
{
	// Wait... we're not a server
	if (acceptSocket == INVALID_SOCKET)
	{
		engError("netSendToUser was called, but no server is running\nPerhaps you wanted to user the 'sendToServer' function?");
		return;
	}

	if (userId < 0 || userId >= SERVER_CAPACITY)
	{
		engError("Invalid user ID %d (Server capacity: %d)", userId, SERVER_CAPACITY);
		return;
	}

	User* user = &users[userId];
	if (user->state != UserState::Active)
	{
		engError("Tried to send message to user %d, but they aren't connected", userId);
		return;
	}

	// Send size first
	send(user->socket, (char*)&message.size, 4, 0);
	send(user->socket, message.buffer, message.size, 0);
}

void serverBroadcast(const NetMessage& message)
{
	// Wait... we're not a server
	if (acceptSocket == INVALID_SOCKET)
	{
		engError("netBroadcast was called, but no server is running\nPerhaps you wanted to user the 'sendToServer' function?");
		return;
	}

	for(auto& user : users)
	{
		if (user.state != UserState::Active)
			continue;

		send(user.socket, (char*)&message.size, 4, 0);
		send(user.socket, message.buffer, message.size, 0);
	}
}
