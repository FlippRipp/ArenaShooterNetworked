#include "Projectile.h"
#include "Engine.h"
#include "Player.h"
#include "Server.h"

Projectile projectiles[PROJECTILE_MAX];

Projectile::Projectile()
{
}

void Projectile::Update()
{
	x += VelocityX * engDeltaTime();
	y += VelocityY * engDeltaTime();

	if (engElapsedTime() - spawnTime > PROJECTILE_SPAWN_TIME)
	{
		Destroy();
	}

	// check for collision with player

	for (auto& player : players)
	{
		if (!player.alive)
			continue;

		if (player.id == ownerPlayer)
			continue;

			float diffX = x - player.x;
			float diffY = y - player.y;
			float radiusSqrd = PROJECTILE_RADIUS + PLAYER_RADIUS;
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

void Projectile::Draw()
{
	engSetColor(0x000000FF);
	engFillRect(x - PROJECTILE_RADIUS, y - PROJECTILE_RADIUS, PROJECTILE_RADIUS, PROJECTILE_RADIUS);

}

void Projectile::Spawn(int player, float spawnX, float spawnY, float dirX, float dirY)
{
	ownerPlayer = player;
	x = spawnX;
	y = spawnY;
	alive = true;
	VelocityX = dirX * SPEED;
	VelocityY = dirY * SPEED;
	spawnTime = engElapsedTime();
}

void Projectile::Destroy()
{
	alive = false;
}
