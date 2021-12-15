#include "Client.h"
#include "Network.h"
#include "Engine.h"
#include <stdlib.h>
#include <stdio.h>
#include <WinSock2.h>

SOCKET serverSocket;

static DWORD recvWorker(void*)
{
	while(true)
	{
		int msgSize = 0;
		recv(serverSocket, (char*)&msgSize, 4, MSG_WAITALL);

		char* msgBuffer = (char*)malloc(msgSize);
		int result = recv(serverSocket, msgBuffer, msgSize, MSG_WAITALL);
		if (result == 0 || result == SOCKET_ERROR)
		{
			serverSocket = INVALID_SOCKET;

			free(msgBuffer);
			return 0;
		}

		// Create a message for this
		NetMessage message;
		message.buffer = msgBuffer;
		message.size = msgSize;

		netPushEvent(NetEvent::makeMessage(0, message));
	}
}

bool clientConnect(const char* address, unsigned short port)
{
	// On client, connect to our server
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in connectAddr;

	// Parse address
	connectAddr.sin_family = AF_INET;
	int addrResult = sscanf_s(address, "%hhu.%hhu.%hhu.%hhu",
		&connectAddr.sin_addr.s_net,
		&connectAddr.sin_addr.s_host,
		&connectAddr.sin_addr.s_lh,
		&connectAddr.sin_addr.s_impno
	);
	if (addrResult != 4)
	{
		engError("Ip addr '%s' is invalid, expected address in form 'XXX.XXX.XXX.XXX'", address);
		return false;
	}

	connectAddr.sin_port = htons(port);

	if (connect(serverSocket, (sockaddr*)&connectAddr, sizeof(connectAddr)))
	{
		engError("Failed to connect to (%s:%d)", address, port);
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;

		return false;
	}

	// Spin up a recv worker for the server socket
	CreateThread(
		0, 0,
		recvWorker,
		nullptr,
		0, 0
	);

	return true;
}

bool clientIsConnected()
{
	return serverSocket != INVALID_SOCKET;
}

void clientSend(const NetMessage& message)
{
	// Wait... we're not connected
	if (!clientIsConnected())
	{
		engError("clientSend was called, but we're not connected to a server");
		return;
	}

	// Send size first
	send(serverSocket, (char*)&message.size, 4, 0);
	send(serverSocket, message.buffer, message.size, 0);
}