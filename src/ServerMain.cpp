#include "Engine.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include "Player.h"
#include "Server.h"
#include "Network.h"
#include "MessageType.h"
#include "Projectile.h"
#include "Pickup.h"
#include "Missile.h"
#include <cmath>
#if SERVER


bool gameStarted = false;
float pickupTimer = 0;
void handleMessage(int userId, NetMessage msg)
{
	MessageType type = msg.read<MessageType>();
	switch(type)
	{
		case MessageType::PlayerName:
		{
			int playerId = msg.read<int>();
			if (playerId != userId)
			{
				serverKickUser(userId);
				break;
			}

			int nameLen = msg.read<unsigned char>();
			if(nameLen > PLAYER_NAME_MAX)
			{
				serverKickUser(userId);
				break;
			}

			Player* player = &players[userId];
			msg.read(player->name, nameLen);

			player->name[nameLen] = 0;

			serverBroadcast(msg);
			break;
		}

		case MessageType::PlayerPosition:
		{
			int playerId = msg.read<int>();
			if(playerId != userId)
			{
				serverKickUser(userId);
				break;
			}

			Player* player = &players[userId];
			float newX = msg.read<float>();
			float newY = msg.read<float>();
			player->netReceivePosition(newX, newY);

			serverBroadcast(msg);
			break;
		}

		case MessageType::PlayerInput:
		{
			int playerId = msg.read<int>();
			if (playerId != userId)
			{
				serverKickUser(userId);
				break;
			}

			Player* player = &players[userId];
			float newX = msg.read<float>();
			float newY = msg.read<float>();
			player->netReceivePosition(newX, newY);

			player->inputX = msg.read<char>();
			player->inputY = msg.read<char>();
			serverBroadcast(msg);
			break;
		}
		case MessageType::PlayerRequestFire:
		{
			if (gameStarted)
				break;

			int ProjectiileIndex = -1;
			for (int i = 0; i < PROJECTILE_MAX; i++)
			{
				if (projectiles[i].alive)
					continue;

				ProjectiileIndex = i;
				break;
				
			}

			if (ProjectiileIndex == -1) {
				engError("ran out of projectile");
				break;
			}

			Player* player = &players[userId];
			projectiles[ProjectiileIndex].Spawn(userId, player->x, player->y, player->inputX, player->inputY);

			if (player->inputX == 0 && player->inputY == 0)
				break;

			if (engElapsedTime() - player->lastFireTime < PLAYER_COOLDOWN_TIME)
				break;
			
			player->lastFireTime = engElapsedTime();

			NetMessage response;
			response.write<MessageType>(MessageType::ProjectileSpawn);
			response.write<int>(ProjectiileIndex);
			response.write<int>(userId);
			response.write<float>(player->x);
			response.write<float>(player->y);
			response.write<char>(player->inputX);
			response.write<char>(player->inputY);

			serverBroadcast(response);
			response.free();
			break;
		}
		case MessageType::PlayerRequestMissileFire:
		{
			Player* userPlayer = &players[userId];
			engPrint("Missile Request Received from player id %d", userId);
			if (!userPlayer->hasMissile)
				break;

			int missileId = -1;
			for (int i = 0; i < MAX_NUMBER_OF_MISSILES; i++)
			{
				if (!missiles[i].alive) {
					missileId = i;
					break;
				}
			}
			if (missileId == -1)
			{
				break;
				engError("ran out of Missiles");
			}

			Player* closestPlayer = nullptr;
			float closestPlayerDist = 100000000;

			for (auto& player : players)
			{
				float diffX = userPlayer->x - player.x;
				float diffY = userPlayer->y - player.y;
				float distSqrd = (diffX * diffX + diffY * diffY);
				if (closestPlayerDist < distSqrd || player.id == userPlayer->id)
					continue;
				if (!player.alive)
					continue;
				
				closestPlayerDist = distSqrd;
				closestPlayer = &player;
			}

			if (closestPlayer == nullptr)
				break;

			float xDiff = closestPlayer->x - userPlayer->x;
			float yDiff = closestPlayer->y - userPlayer->y;
			float diffMagn = sqrtf(xDiff*xDiff + yDiff*yDiff);

			engPrint("Firing Misile Server");

			NetMessage response;

			response.write<MessageType>(MessageType::MissileSpawn);
			response.write<int>(missileId);
			response.write<int>(userId);
			response.write<float>(userPlayer->x);
			response.write<float>(userPlayer->y);
			response.write<float>(xDiff/diffMagn);
			response.write<float>(yDiff/diffMagn);

			serverBroadcast(response);

			response.free();

			missiles[missileId].Spawn(userId, userPlayer->x, userPlayer->y, xDiff / diffMagn, yDiff / diffMagn);
		
			userPlayer->hasMissile = false;
		}
	}
}

int WinMain(HINSTANCE, HINSTANCE, char*, int)
{
	engInit();
	netInit();

	if (!serverStartup(666))
		return 1;

	while(engBeginFrame())
	{
		NetEvent event;
		while(netPollEvent(&event))
		{
			switch(event.type)
			{
				case NetEventType::UserConnected:
				{
					if (gameStarted) 
					{
						serverKickUser(event.userId);
						break;
					}

					engPrint("User %d connected", event.userId);
					serverAcceptUser(event.userId);

					for (int i = 0; i < PLAYER_MAX; i++)
					{
						if (!players[i].alive)
							continue;

						{
							NetMessage spawnMsg;
							spawnMsg.write<MessageType>(MessageType::PlayerSpawn);
							spawnMsg.write<int>(i);
							spawnMsg.write<float>(players[i].x);
							spawnMsg.write<float>(players[i].y);

							serverSendTo(spawnMsg, event.userId);
							spawnMsg.free();
						}
						{
							NetMessage nameMsg;
							nameMsg.write<MessageType>(MessageType::PlayerName);
							nameMsg.write<int>(i);
							nameMsg.write<unsigned char>(strlen(players[i].name));
							nameMsg.write(players[i].name, strlen(players[i].name));

							serverSendTo(nameMsg, event.userId);
							nameMsg.free();
						}
					}

					{
						Player* player = &players[event.userId];
						player->spawn(event.userId, rand() % 800, rand() % 600);

						NetMessage msg;
						msg.write<MessageType>(MessageType::PlayerSpawn);
						msg.write<int>(event.userId);
						msg.write<float>(player->x);
						msg.write<float>(player->y);

						serverBroadcast(msg);
						msg.free();
					}

					{
						NetMessage msg;
						msg.write<MessageType>(MessageType::PlayerPossess);
						msg.write<int>(event.userId);

						serverSendTo(msg, event.userId);
						msg.free();
					}
					break;
				}

				case NetEventType::UserDisconnected:
				{
					engPrint("User %d disconnected", event.userId);
					players[event.userId].destroy();

					NetMessage destroyMsg;
					destroyMsg.write<MessageType>(MessageType::PlayerDestroy);
					destroyMsg.write<int>(event.userId);

					serverBroadcast(destroyMsg);
					destroyMsg.free();
					break;
				}

				case NetEventType::Message:
				{
					handleMessage(event.userId, event.message);
					break;
				}
			}

			event.free();
		}

		if(engKeyPressed(Key::Space))
			gameStarted = !gameStarted;

		engSetColor(0xCC4444FF);
		engText(400, 230, gameStarted ? "Started" : "Waitng");

		//do we have a winner
		int numAlivePlayers = 0;
		int lastAlivePlayerIndex = -1;

		for (auto& player : players)
		{
			if (player.alive)
			{
				numAlivePlayers++;
				lastAlivePlayerIndex = player.id;
			}
		}

		pickupTimer += engDeltaTime();

		if (pickupTimer > PICKUP_SPAWN_TIME)
		{
			int pickUpId = -1;
			for (int i = 0; i < NUMBER_OF_PICKUPS; i++)
			{
				if (!pickUps[i].alive) 
				{
					pickUpId = i;
					break;
				}
			}

			pickupTimer = 0;

			if (pickUpId != -1)
			{
				engPrint("Spawning Pickup %f", pickupTimer);

				float x = rand() % 800;
				float y = rand() % 600;
				PickupType randomPickup;

				if (rand() % 10 > 5) randomPickup = PickupType::ShieldPickup;
				else randomPickup = PickupType::MissilePickup;

				pickUps[pickUpId].Spawn(x, y, randomPickup);

				NetMessage response;
				response.write<MessageType>(MessageType::PickupSpawn);
				response.write<int>(pickUpId);
				response.write<float>(x);
				response.write<float>(y);
				response.write<PickupType>(randomPickup);

				serverBroadcast(response);
				response.free();
			}
		}

		if (numAlivePlayers == 1)
		{
			engTextf(400, 300, "'%s' WINS", players[lastAlivePlayerIndex].name);
		}
		else if (numAlivePlayers == 0)
			engText(400, 300, "Draw");


		engSetColor(0xCC4444FF);
		engClear();

		for (auto& player : players)
		{
			if (player.alive)
			{
				player.update();
				player.draw();
			}
		}
		
		for (auto& projectile : projectiles)
		{
			if (projectile.alive)
			{
				projectile.Update();
				projectile.Draw();
			}
		}

		for (auto& pickUp : pickUps)
		{
			if (pickUp.alive)
			{
				pickUp.Update();
				pickUp.Draw();
			}
		}

		for (auto& missile : missiles)
		{
			if (missile.alive)
			{
				missile.Update();
				missile.Draw();
			}
		}		
	}

	return 0;
}

#endif
