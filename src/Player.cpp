#include "Player.h"

#include "Client.h"
#include "Engine.h"
#include "MessageType.h"
#include "Network.h"
#include "Projectile.h"
#include "Missile.h"
#include <cmath>

#define clamp(a, min, max) (a < min ? min : (a > max ? max : a))

Player players[PLAYER_MAX];
#if CLIENT
int possessedPlayerId = -1;
#endif

void Player::netReceivePosition(float newX, float newY)
{
	newX = clamp(x, 0 + PLAYER_RADIUS, 800 - PLAYER_RADIUS);
	newY = clamp(y, 0 + PLAYER_RADIUS, 600 - PLAYER_RADIUS);
	
	errorX = newX - x;
	errorY = newY - y;
}

void Player::spawn(int id, int spawnX, int spawnY)
{
	this->id = id;
	alive = true;
	x = (float) spawnX;
	y = (float) spawnY;
}

void Player::destroy()
{
	alive = false;
}

bool Player::hasControl()
{
#if SERVER
	return false;
#else
	return id == possessedPlayerId;
#endif
}

void Player::update()
{
#if CLIENT
	if(hasControl())
	{
		int frameInputX = 0;
		int frameInputY = 0;

		if (engKeyDown(Key::A)) frameInputX -= 1;
		if (engKeyDown(Key::D)) frameInputX += 1;
		if (engKeyDown(Key::W)) frameInputY -= 1;
		if (engKeyDown(Key::S)) frameInputY += 1;

		if (frameInputX != inputX || frameInputY != inputY)
		{
			NetMessage msg;
			msg.write<MessageType>(MessageType::PlayerInput);
			msg.write<int>(id);
			msg.write<float>(x);
			msg.write<float>(y);

			msg.write<char>(frameInputX);
			msg.write<char>(frameInputY);

			clientSend(msg);
			msg.free();
		}

		inputX = frameInputX;
		inputY = frameInputY;

		if (engKeyPressed(Key::K))
		{
			NetMessage msg;
			msg.write<MessageType>(MessageType::PlayerRequestFire);
			clientSend(msg);
			msg.free();
		}

		if (engKeyDown(Key::J))
		{
			DrawTrajectoryLine();

			if (engKeyPressed(Key::H))
			{
				engPrint("Requesting To Fire Missile");
				NetMessage msg;
				msg.write<MessageType>(MessageType::PlayerRequestMissileFire);
				clientSend(msg);
				msg.free();

			}
		}
	}
#endif

	if(!hasControl())
	{
		float errorDeltaX = errorX * playerErrorCorrectionStrength * engDeltaTime();
		float errorDeltaY = errorY * playerErrorCorrectionStrength * engDeltaTime();

		x += errorDeltaX;
		y += errorDeltaY;
		errorX -= errorDeltaX;
		errorY -= errorDeltaY;
	}

	x += inputX * playerSpeed * engDeltaTime();
	y += inputY * playerSpeed * engDeltaTime();

	x = clamp(x, 0 + PLAYER_RADIUS, 800 - PLAYER_RADIUS);
	y = clamp(y, 0 + PLAYER_RADIUS, 600 - PLAYER_RADIUS);
}

void Player::DrawTrajectoryLine()
{
	if (!hasMissile)
		return;

	Player* closestPlayer = nullptr;
	float closestPlayerDist = 100000000;

	for (auto& player : players)
	{
		float diffX = x - player.x;
		float diffY = y - player.y;
		float distSqrd = (diffX * diffX + diffY * diffY);
		if (closestPlayerDist < distSqrd || player.id == id)
			continue;
		if (!player.alive)
			continue;


		closestPlayerDist = distSqrd;
		closestPlayer = &player;
	}


	if (closestPlayer == nullptr)
		return;

	
	float t = 0;

	while (t <= 1)
	{
		float xPos = closestPlayer->x + (x - closestPlayer->x) * t;
		float yPos = closestPlayer->y + (y - closestPlayer->y) * t;

		//engPrint("XPos %f, YPos %f", xPos, yPos);

		engSetColor(0xFFFFFFFF);
		engFillRect((int)xPos - TRAJECTORY_LINE_POINT_SIZE / 2, (int)yPos - TRAJECTORY_LINE_POINT_SIZE / 2,
			TRAJECTORY_LINE_POINT_SIZE, TRAJECTORY_LINE_POINT_SIZE);

		t += 1.f / TRAJECTORY_LINE_POINTS;
	}
}

void Player::draw()
{
	engSetColor(0xDEADBEEF);
#if CLIENT
	if (hasControl())
		engSetColor(0xADDEBEEF);
#endif
	engFillRect((int)x - PLAYER_RADIUS, (int)y - PLAYER_RADIUS, 32, 32);
	engText((int)x - PLAYER_RADIUS, (int)y - 16 - PLAYER_RADIUS, name);

	if (isShielded)
	{
		engSetColor(0xFFFFFFFF);
		engRect((int)x - PLAYER_RADIUS * 1.5, (int)y - PLAYER_RADIUS * 1.5, 48, 48);
	}
	if (hasMissile)
	{
		engSetColor(0xFF0000FF);
		engFillRect((int)x - 8, (int)y - 8, 16, 16);
	}
}
