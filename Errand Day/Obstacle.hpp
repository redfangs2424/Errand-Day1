#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

#include <cstdlib>

// ---- constants ----
#define OB_COUNT 10
#define OB_CAPACITY 6

static int OB_WIDTH[OB_COUNT]  = { 102, 95, 56, 108, 98, 120, 95, 120, 59, 117 };
static int OB_HEIGHT[OB_COUNT] = {  74, 63,101,  74,100,  76, 63,  76, 76, 108 };

static int gRoadLeft  = 72;
static int gRoadRight = 636;
static int gScreenH   = 1080;

// Random "bag" to avoid repeats
static int gBag[OB_COUNT];
static int gBagIndex = OB_COUNT;

// ✅ Spawn enable flag (1 = spawn allowed, 0 = stop spawning)
static int gObstacleSpawnEnabled = 1;

// obstacle state
struct Obstacle {
	int active;
	int imgIndex;
	int lane;
	float x;
	float y;
	int width;
	int height;
};

static Obstacle gObs[OB_CAPACITY];

// ---- runtime difficulty knobs ----
struct ObstacleSpawnConfig {
	int maxActive;             // <= OB_CAPACITY
	int spawnChancePercent;    // 0..100

	// If active == 0:  baseNoObs + rand()%rangeNoObs
	// Else:            baseWithObs + rand()%rangeWithObs
	int baseDelayNoObs;
	int rangeDelayNoObs;
	int baseDelayWithObs;
	int rangeDelayWithObs;
};

static ObstacleSpawnConfig gSpawnCfg = {
	2,   // maxActive
	60,  // spawnChancePercent
	60, 180,   // no obstacles delay
	80, 240    // with obstacles delay
};

static int gSpawnTicks = 60;

// ✅ Call this from iMain to enable/disable spawning
static void setObstacleSpawnEnabled(int enabled)
{
	gObstacleSpawnEnabled = enabled ? 1 : 0;
}

// Lane helpers
static int laneWidth() {
	return (gRoadRight - gRoadLeft) / 4;
}

static float laneX(int lane, int obstacleWidth) {
	int lw = laneWidth();
	return (float)(gRoadLeft + lane * lw + (lw - obstacleWidth) / 2);
}

// Collision
static int collision(float ax, float ay, float aw, float ah,
	                 float bx, float by, float bw, float bh)
{
	return (ax < bx + bw) && (ax + aw > bx) &&
		   (ay < by + bh) && (ay + ah > by);
}

static void setObstacleSpawnConfig(const ObstacleSpawnConfig& cfg)
{
	gSpawnCfg = cfg;

	if (gSpawnCfg.maxActive < 0) gSpawnCfg.maxActive = 0;
	if (gSpawnCfg.maxActive > OB_CAPACITY) gSpawnCfg.maxActive = OB_CAPACITY;

	if (gSpawnCfg.spawnChancePercent < 0) gSpawnCfg.spawnChancePercent = 0;
	if (gSpawnCfg.spawnChancePercent > 100) gSpawnCfg.spawnChancePercent = 100;

	if (gSpawnCfg.baseDelayNoObs < 1) gSpawnCfg.baseDelayNoObs = 1;
	if (gSpawnCfg.rangeDelayNoObs < 0) gSpawnCfg.rangeDelayNoObs = 0;

	if (gSpawnCfg.baseDelayWithObs < 1) gSpawnCfg.baseDelayWithObs = 1;
	if (gSpawnCfg.rangeDelayWithObs < 0) gSpawnCfg.rangeDelayWithObs = 0;
}

static void initObstacleSystem(int roadLeft, int roadRight, int screenH)
{
	gRoadLeft = roadLeft;
	gRoadRight = roadRight;
	gScreenH = screenH;

	for (int i = 0; i < OB_CAPACITY; i++)
		gObs[i].active = 0;

	gSpawnTicks = 60;
}

static void resetObstacles()
{
	for (int i = 0; i < OB_CAPACITY; i++)
		gObs[i].active = 0;

	gSpawnTicks = 60;
}

static int activeObstacleCount()
{
	int c = 0;
	for (int i = 0; i < OB_CAPACITY; i++)
		if (gObs[i].active) c++;
	return c;
}

static void refillBag()
{
	for (int i = 0; i < OB_COUNT; i++)
		gBag[i] = i;

	for (int i = 0; i < OB_COUNT; i++) {
		int r = rand() % OB_COUNT;
		int temp = gBag[i];
		gBag[i] = gBag[r];
		gBag[r] = temp;
	}

	gBagIndex = 0;
}

static void spawnObstacle()
{
	if (activeObstacleCount() >= gSpawnCfg.maxActive) return;

	int idx = -1;
	for (int i = 0; i < OB_CAPACITY; i++) {
		if (!gObs[i].active) { idx = i; break; }
	}
	if (idx < 0) return;

	gObs[idx].active = 1;

	if (gBagIndex >= OB_COUNT)
		refillBag();

	gObs[idx].imgIndex = gBag[gBagIndex++];
	gObs[idx].lane = rand() % 4;

	int k = gObs[idx].imgIndex;
	gObs[idx].width = OB_WIDTH[k];
	gObs[idx].height = OB_HEIGHT[k];

	gObs[idx].x = laneX(gObs[idx].lane, gObs[idx].width);
	gObs[idx].y = (float)(gScreenH + (rand() % 200));
}

// ✅ ONE-ARG update (easy to call) + uses gObstacleSpawnEnabled internally
static void updateObstacles(float speed)
{
	gSpawnTicks--;
	if (gSpawnTicks <= 0) {

		// only spawn when enabled
		if (gObstacleSpawnEnabled &&
			(rand() % 100) < gSpawnCfg.spawnChancePercent)
		{
			spawnObstacle();
		}

		int active = activeObstacleCount();

		if (active == 0) {
			gSpawnTicks = gSpawnCfg.baseDelayNoObs
				+ (gSpawnCfg.rangeDelayNoObs == 0 ? 0 : (rand() % gSpawnCfg.rangeDelayNoObs));
		}
		else {
			gSpawnTicks = gSpawnCfg.baseDelayWithObs
				+ (gSpawnCfg.rangeDelayWithObs == 0 ? 0 : (rand() % gSpawnCfg.rangeDelayWithObs));
		}
	}

	// Move existing obstacles (always)
	for (int i = 0; i < OB_CAPACITY; i++) {
		if (!gObs[i].active) continue;

		gObs[i].y -= speed;

		if (gObs[i].y < -gObs[i].height) {
			gObs[i].active = 0;
		}
	}
}

static int checkObstacleCollision(float carX, float carY, float carW, float carH)
{
	for (int i = 0; i < OB_CAPACITY; i++) {
		if (!gObs[i].active) continue;

		if (collision(carX, carY, carW, carH,
			gObs[i].x, gObs[i].y,
			(float)gObs[i].width, (float)gObs[i].height))
		{
			gObs[i].active = 0;
			return 1;
		}
	}
	return 0;
}

#endif
