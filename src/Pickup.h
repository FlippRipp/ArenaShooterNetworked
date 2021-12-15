#pragma once
#define NUMBER_OF_PICKUPS 10
#define PICKUP_RADIUS 16
#define PICKUP_SPAWN_TIME 10

enum class PickupType
{
	ShieldPickup,
	MissilePickup,
};

class Pickup
{
	public:
		float posX;
		float posY;

		bool alive = false;

		PickupType pickupType;

		void Update();
		void Draw();

		void Spawn(float x, float y, PickupType pT);
		void Destroy();
};

extern Pickup pickUps[NUMBER_OF_PICKUPS];

