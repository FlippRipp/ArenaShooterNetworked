#pragma once
#define PROJECTILE_MAX 256
#define SPEED 400.f
#define PROJECTILE_SPAWN_TIME 5.f
#define PROJECTILE_RADIUS 4.f
class Projectile {

public:
	Projectile();

	void Update();
	void Draw();
	void Spawn(int player, float spawnX, float spawnY, float Dirx, float dirY);
	void Destroy();

	bool alive = false;
	float x;
	float y;

	float spawnTime;

	float VelocityX;
	float VelocityY;

	int ownerPlayer;
};

extern Projectile projectiles[PROJECTILE_MAX];