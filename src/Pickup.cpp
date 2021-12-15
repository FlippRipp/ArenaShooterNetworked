#include "Pickup.h"
#include "Engine.h"
#include "Player.h"

Pickup pickUps[NUMBER_OF_PICKUPS];

void Pickup::Update()
{
	for (auto& player : players)
	{
		if (!player.alive)
			continue;


		float diffX = posX - player.x;
		float diffY = posY - player.y;
		float radiusSqrd = PICKUP_RADIUS + PLAYER_RADIUS; 
		radiusSqrd *= radiusSqrd;
		float distSqrd = (diffX * diffX + diffY * diffY);


		if (distSqrd < radiusSqrd) {
			Destroy();
			if (pickupType == PickupType::ShieldPickup)
				player.isShielded = true;
			else if (pickupType == PickupType::MissilePickup)
				player.hasMissile = true;
			return;
		}
	}
}

void Pickup::Draw()
{
	if(pickupType == PickupType::ShieldPickup) engSetColor(0x00FF00FF);
	else if(pickupType == PickupType::MissilePickup) engSetColor(0xFF0000FF);

	engFillRect(posX - PICKUP_RADIUS, posY - PICKUP_RADIUS, PICKUP_RADIUS, PICKUP_RADIUS);

}

void Pickup::Spawn(float x, float y, PickupType pT)
{
	posX = x;
	posY = y;
	pickupType = pT;
	alive = true;
}

void Pickup::Destroy()
{
	alive = false;
}
