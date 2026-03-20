#ifndef ENEMY_HPP
#define ENEMY_HPP

#include <cstdlib>
#include <cmath>   // fabsf

// ------------------------------
//         Enemy Car System
// ------------------------------
struct EnemyCar {
	int active;
	float x, y;

	int phase;          // 0=approach, 1=side, 2=bump, 3=pass
	int side;           // -1 left, +1 right

	int bumpsLeft;
	int bumpDir;        // 0 = going in, 1 = going out

	int waitTicks;      // small pauses

	// bump attempt locking (so player can dodge and enemy can miss)
	int attackLocked;   // 0/1
	float attackX;      // fixed "bump-in" target during this attempt

	int hitCooldown;    // ticks before enemy can score another hit
};

struct EnemySystem {

	static const int ENEMY_W = 116;
	static const int ENEMY_H = 169;

	// tweakables
	int enabled;
	int maxEnemy;
	int spawnMinTicks;
	int spawnRangeTicks;

	float approachGain;
	float followXSpeed;

	int bumpCount;
	float farGap;
	float nearGap;
	float bumpPush;

	// dodge/miss tuning
	float dodgeToMiss;      // MUST move at least this far (px) to make them miss
	float contactWindow;    // how close enemy must be to attackX to count as "touch moment"

	// side-by-side feel
	int sideHoldTicks;
	float sideXSpeedMul;
	float sideYLockSpeed;

	// pass feel (linger)
	float passFastGain;
	float passSlowGain;
	float passSlowAfter;

	int clampEnabled;

	// runtime
	int imgEnemy;
	EnemyCar e[3];
	int spawnCooldown;

	EnemySystem() {
		enabled = 1;

		maxEnemy = 1;
		spawnMinTicks = 480;
		spawnRangeTicks = 360;

		approachGain = 7.0f;
		followXSpeed = 6.0f;

		bumpCount = 2;
		farGap = 30.0f;
		nearGap = 2.0f;     // close enough to look like a hit attempt
		bumpPush = 2.5f;

		// You want dodging to matter:
		// Increase this if it still feels like "hits every time".
		dodgeToMiss = 20.0f;

		// How close to the locked line before we decide HIT/MISS.
		contactWindow = 2.0f;

		sideHoldTicks = 75;
		sideXSpeedMul = 0.65f;
		sideYLockSpeed = 6.0f;

		passFastGain = 5.5f;
		passSlowGain = 2.4f;
		passSlowAfter = 90.0f;

		clampEnabled = 0;

		imgEnemy = -1;

		for (int i = 0; i < 3; i++) e[i].active = 0;
		spawnCooldown = spawnMinTicks;
	}

	void loadImages() {
		imgEnemy = iLoadImage("Assets\\game\\enemy_car.png");
	}

	void reset() {
		for (int i = 0; i < 3; i++) e[i].active = 0;
		spawnCooldown = spawnMinTicks + (spawnRangeTicks ? (rand() % (spawnRangeTicks + 1)) : 0);
	}

	void setEnabled(int on) {
		enabled = on ? 1 : 0;
		if (!enabled) reset();
	}

	int activeCount() const {
		int c = 0;
		for (int i = 0; i < 3; i++) if (e[i].active) c++;
		return c;
	}

	void spawnOne(float carX, float carY, float carW) {
		if (!enabled) return;
		if (activeCount() >= maxEnemy) return;

		int idx = -1;
		for (int i = 0; i < 3; i++) {
			if (!e[i].active) { idx = i; break; }
		}
		if (idx < 0) return;

		e[idx].active = 1;
		e[idx].phase = 0;
		e[idx].side = (rand() % 2) ? 1 : -1;

		e[idx].bumpsLeft = bumpCount;
		e[idx].bumpDir = 0;
		e[idx].waitTicks = 0;

		e[idx].attackLocked = 0;
		e[idx].attackX = 0.0f;

		e[idx].hitCooldown = 0;

		e[idx].y = carY - 260.0f;
		e[idx].x = (e[idx].side == -1)
			? (carX - ENEMY_W - farGap)
			: (carX + carW + farGap);
	}

	static float moveToward(float v, float target, float step) {
		if (v < target) { v += step; if (v > target) v = target; }
		else if (v > target) { v -= step; if (v < target) v = target; }
		return v;
	}

	float update(
		float roadSpeed,
		float carX, float carY, float carW, float carH,
		int nitroOn,
		float roadLeft, float roadRight,
		int* outPlayerHit = 0
		) {
		if (outPlayerHit) *outPlayerHit = 0;
		if (!enabled) return carX;

		// spawn control
		if (spawnCooldown > 0) spawnCooldown--;
		if (spawnCooldown <= 0) {
			if (!nitroOn) spawnOne(carX, carY, carW);
			spawnCooldown = spawnMinTicks + (spawnRangeTicks ? (rand() % (spawnRangeTicks + 1)) : 0);
		}

		const float EPS = 0.01f;

		for (int i = 0; i < 3; i++) {
			if (!e[i].active) continue;

			if (e[i].hitCooldown > 0) e[i].hitCooldown--;

			float relY = nitroOn ? -8.0f : 0.0f;

			if (e[i].phase == 0) {
				relY += approachGain;
				e[i].y += relY;

				float targetX = (e[i].side == -1)
					? (carX - ENEMY_W - farGap)
					: (carX + carW + farGap);
				e[i].x = moveToward(e[i].x, targetX, followXSpeed);

				if (e[i].y >= carY) {
					e[i].y = carY;
					e[i].phase = 1;
					e[i].waitTicks = sideHoldTicks;
				}
			}
			else if (e[i].phase == 1) {
				// lock Y to your car so it truly stays side-by-side
				if (!nitroOn) {
					e[i].y = moveToward(e[i].y, carY, sideYLockSpeed);
				}

				if (e[i].waitTicks > 0) e[i].waitTicks--;

				float targetX = (e[i].side == -1)
					? (carX - ENEMY_W - farGap)
					: (carX + carW + farGap);
				e[i].x = moveToward(e[i].x, targetX, followXSpeed * sideXSpeedMul);

				if (e[i].waitTicks <= 0) {
					e[i].phase = 2;
					e[i].bumpDir = 0;

					// lock attack line ONCE so dodging can cause a miss
					e[i].attackLocked = 1;
					e[i].attackX = (e[i].side == -1)
						? (carX - ENEMY_W - nearGap)
						: (carX + carW + nearGap);
				}
			}
			else if (e[i].phase == 2) {
				// keep Y aligned while attempting bump
				if (!nitroOn) {
					e[i].y = moveToward(e[i].y, carY, sideYLockSpeed);
				}

				float inX = e[i].attackX;

				float outX = (e[i].side == -1)
					? (carX - ENEMY_W - farGap)
					: (carX + carW + farGap);

				float targetX = (e[i].bumpDir == 0) ? inX : outX;
				e[i].x = moveToward(e[i].x, targetX, followXSpeed + 2.0f);

				// decide hit/miss ONLY when enemy actually reaches the locked attack line
				if (e[i].bumpDir == 0 && fabsf(e[i].x - inX) <= contactWindow) {

					// Where would contact be *right now* if player did not dodge?
					float nowInX = (e[i].side == -1)
						? (carX - ENEMY_W - nearGap)
						: (carX + carW + nearGap);

					// If the player moved away enough from the locked line -> MISS
					float dodgeDelta = fabsf(nowInX - e[i].attackX);
					int dodged = (dodgeDelta >= dodgeToMiss);

					if (dodged) {
						// MISS: immediately give up and go forward (PASS)
						e[i].phase = 3;
						continue;
					}

					// Not dodged enough: allow a hit but not every time (cooldown)
					if (e[i].hitCooldown <= 0) {
						carX += e[i].side * bumpPush;

						if (carX < roadLeft) carX = roadLeft;
						if (carX > roadRight - carW) carX = roadRight - carW;

						e[i].hitCooldown = 45; // ~0.75s
						if (outPlayerHit) *outPlayerHit = 1;
					}

					// After contact moment, go back out (or end)
					e[i].bumpDir = 1;
				}
				else if (e[i].bumpDir == 1 && fabsf(e[i].x - outX) <= EPS) {
					e[i].bumpsLeft--;
					e[i].bumpDir = 0;

					if (e[i].bumpsLeft <= 0) {
						e[i].phase = 3;
					}
					else {
						// For fairness, lock a NEW attack line (next attempt)
						e[i].attackX = (e[i].side == -1)
							? (carX - ENEMY_W - nearGap)
							: (carX + carW + nearGap);

						e[i].waitTicks = 10;
					}
				}

				if (e[i].waitTicks > 0) e[i].waitTicks--;

				if (clampEnabled) {
					float minX = (e[i].side == -1)
						? (carX - ENEMY_W - nearGap)
						: (carX + carW + nearGap);

					if (e[i].side == -1 && e[i].x > minX) e[i].x = minX;
					if (e[i].side == 1 && e[i].x < minX) e[i].x = minX;
				}
			}
			else {
				// PASS: slow pass + linger, X fixed for aiming later
				float gain = (e[i].y < carY + passSlowAfter) ? passFastGain : passSlowGain;
				e[i].y += (nitroOn ? -6.0f : gain);

				// IMPORTANT: keep X unchanged

				if (e[i].y > 1600.0f || e[i].y < -500.0f) {
					e[i].active = 0;
				}
			}
		}

		return carX;
	}

	void draw() const {
		if (!enabled) return;
		for (int i = 0; i < 3; i++) {
			if (!e[i].active) continue;
			iShowImage((int)e[i].x, (int)e[i].y, 116, 169, imgEnemy);
		}
	}
};

#endif