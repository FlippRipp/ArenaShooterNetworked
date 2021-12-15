#include "Missile.h"
#include "Engine.h"
#include "Player.h"
#include "Server.h"

Missile missiles[MAX_NUMBER_OF_MISSILES];

void Missile::Update()
{
	positionX += velocityX * engDeltaTime();
	positionY += velocityY * engDeltaTime();

	for (auto& player : players)
	{
		if (!player.alive)
			continue;

		if (player.id == ownerPlayer)
			continue;

		float diffX = positionX - player.x;
		float diffY = positionY - player.y;
		float radiusSqrd = MISSILE_RADIUS + PLAYER_RADIUS;
		radiusSqrd *= radiusSqrd;
		float distSqrd = (diffX * diffX + diffY * diffY);

		if (distSqrd < radiusSqrd) {
			Destroy();

			if (player.isShielded)
			{
				player.isShielded = false;
				return;
			}

#if SERVER
			serverKickUser(player.id);
			engPrint("'%s' --> '%s'", players[ownerPlayer].name, player.name);
#endif
			return;
		}
	}
}

void Missile::Draw()
{
	engSetColor(0x990000FF);
	engFillRect(positionX - MISSILE_RADIUS, positionY - MISSILE_RADIUS,
		MISSILE_RADIUS * 2, MISSILE_RADIUS * 2);

}

void Missile::Spawn(int player, float x, float y, float xDir, float yDir)
{
	alive = true;
	ownerPlayer = player;

	positionX = x;
	positionY = y;

	velocityX = xDir * MISSILE_SPEED;
	velocityY = yDir * MISSILE_SPEED;
}

void Missile::Destroy()
{
	alive = false;
}
