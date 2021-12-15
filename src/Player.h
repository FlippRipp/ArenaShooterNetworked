#pragma once
#define PLAYER_MAX 20
#define PLAYER_NAME_MAX 15
#define PLAYER_RADIUS 16.f
#define PLAYER_COOLDOWN_TIME 1
#define TRAJECTORY_LINE_POINTS 16.f
#define TRAJECTORY_LINE_POINT_SIZE 8

const float playerSpeed = 180.f;
const float playerErrorCorrectionStrength = 1.5f;

class Player
{
public:
	int id = -1;
	bool alive = false;
	float x;
	float y;

	bool isShielded;

	bool hasMissile = false;

	int inputX = 0;
	int inputY = 0;

	float errorX = 0.f;
	float errorY = 0.f;

	float lastFireTime = 0;

	char name[PLAYER_NAME_MAX + 1];

	void netReceivePosition(float newX, float newY);

	void spawn(int id, int spawnX, int spawnY);
	void destroy();

	bool hasControl();
	void update();

	void DrawTrajectoryLine();

	void draw();
};

extern Player players[PLAYER_MAX];

#if CLIENT
extern int possessedPlayerId;
#endif