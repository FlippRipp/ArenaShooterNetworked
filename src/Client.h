#pragma once
struct NetMessage;

bool clientConnect(const char* address, unsigned short port);
bool clientIsConnected();

void clientSend(const NetMessage& message);