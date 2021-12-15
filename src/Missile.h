#pragma once
#define MISSILE_SPEED 200.f
#define MISSILE_RADIUS 8
#define MAX_NUMBER_OF_MISSILES 30

class Missile
{
public:

	float positionX = 0;
	float positionY = 0;
	float velocityX = 0;
	float velocityY = 0;

	int ownerPlayer;

	bool alive = false;
	void Update();
	void Draw();

	void Spawn(int player, float x, float y, float xDir, float yDir);
	void Destroy();

};

extern Missile missiles[MAX_NUMBER_OF_MISSILES];

