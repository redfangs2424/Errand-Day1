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

	// NEW: Health system for enemy cars (3 health levels)
	int health;         // current health (3 = full, 2 = damaged, 1 = critical, 0 = dead)
	static const int MAX_HEALTH = 3;  // Enemy takes 3 hits to destroy
};

struct EnemySystem {

	static const int ENEMY_W = 116;
	static const int ENEMY_H = 169;

	// Explosion system (single image)
	int explosionImg;

	struct Explosion {
		bool active;
		float x, y;
		int duration;      // How long to show explosion (in ticks)
		int width, height;
	};

	Explosion explosions[5];  // Max 5 simultaneous explosions

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
	int imgEnemyHP[4];  // Array for health images (0 unused, 1-3 for health levels)
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

		// Initialize health images to -1
		for (int i = 0; i <= EnemyCar::MAX_HEALTH; i++) {
			imgEnemyHP[i] = -1;
		}

		// Initialize explosion image to -1
		explosionImg = -1;

		// Initialize explosions
		for (int i = 0; i < 5; i++) {
			explosions[i].active = false;
		}

		for (int i = 0; i < 3; i++) {
			e[i].active = 0;
			e[i].health = EnemyCar::MAX_HEALTH;
		}
		spawnCooldown = spawnMinTicks;
	}

	void loadImages() {
		imgEnemy = iLoadImage("Assets\\game\\enemy_car.png");

		// Load health bar images (e_hp_1.png, e_hp_2.png, e_hp_3.png)
		for (int i = 1; i <= EnemyCar::MAX_HEALTH; i++) {
			char path[128];
			sprintf_s(path, "Assets\\hp\\e_hp_%d.png", i);
			imgEnemyHP[i] = iLoadImage(path);
		}

		// Load explosion image
		explosionImg = iLoadImage("Assets\\game\\explosion.png");
	}

	void addExplosion(float x, float y) {
		// Find an inactive explosion slot
		for (int i = 0; i < 5; i++) {
			if (!explosions[i].active) {
				explosions[i].active = true;
				explosions[i].x = x;
				explosions[i].y = y;
				explosions[i].duration = 15;  // Show explosion for ~0.25 seconds (15 ticks at 60fps)
				explosions[i].width = 120;    // Adjust based on your explosion image size
				explosions[i].height = 120;
				break;
			}
		}
	}

	void updateExplosions() {
		for (int i = 0; i < 5; i++) {
			if (explosions[i].active) {
				explosions[i].duration--;
				if (explosions[i].duration <= 0) {
					explosions[i].active = false;
				}
			}
		}
	}

	void drawExplosions() const {
		for (int i = 0; i < 5; i++) {
			if (explosions[i].active && explosionImg >= 0) {
				// Center the explosion on the enemy position
				int drawX = (int)(explosions[i].x - (explosions[i].width - ENEMY_W) / 2);
				int drawY = (int)(explosions[i].y - (explosions[i].height - ENEMY_H) / 2);
				iShowImage(drawX, drawY, explosions[i].width, explosions[i].height, explosionImg);
			}
		}
	}

	void reset() {
		for (int i = 0; i < 3; i++) {
			e[i].active = 0;
			e[i].health = EnemyCar::MAX_HEALTH;
		}
		// Clear all explosions
		for (int i = 0; i < 5; i++) {
			explosions[i].active = false;
		}
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

	// Function to reduce enemy health when hit by bat
	void reduceHealth(int enemyIndex) {
		if (enemyIndex < 0 || enemyIndex >= 3) return;
		if (!e[enemyIndex].active) return;

		e[enemyIndex].health--;
		if (e[enemyIndex].health <= 0) {
			// Add explosion at enemy position before destroying
			addExplosion(e[enemyIndex].x, e[enemyIndex].y);
			// Enemy destroyed
			e[enemyIndex].active = 0;
		}
	}

	// Check if bat hit any enemy (returns index of hit enemy, -1 if none)
	int checkBatHit(float batX, float batY, float batW, float batH) {
		for (int i = 0; i < 3; i++) {
			if (!e[i].active) continue;

			// Check collision between bat and enemy
			if (batX < e[i].x + ENEMY_W &&
				batX + batW > e[i].x &&
				batY < e[i].y + ENEMY_H &&
				batY + batH > e[i].y) {
				return i;  // Return index of hit enemy
			}
		}
		return -1;  // No hit
	}

	void spawnOne(float carX, float carY, float carW, float roadLeft, float roadRight) {
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

		// Reset health when spawned (full health = 3)
		e[idx].health = EnemyCar::MAX_HEALTH;

		e[idx].y = carY - 260.0f;

		// Calculate spawn position
		float spawnX = (e[idx].side == -1)
			? (carX - ENEMY_W - farGap)
			: (carX + carW + farGap);

		// Clamp to road boundaries
		if (spawnX < roadLeft) spawnX = roadLeft;
		if (spawnX > roadRight - ENEMY_W) spawnX = roadRight - ENEMY_W;

		e[idx].x = spawnX;
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

		// Update explosions animation
		updateExplosions();

		// spawn control
		if (spawnCooldown > 0) spawnCooldown--;
		if (spawnCooldown <= 0) {
			if (!nitroOn) spawnOne(carX, carY, carW, roadLeft, roadRight);
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

						// Also push the enemy car in opposite direction and clamp to road
						e[i].x -= e[i].side * bumpPush;
						if (e[i].x < roadLeft) e[i].x = roadLeft;
						if (e[i].x > roadRight - ENEMY_W) e[i].x = roadRight - ENEMY_W;

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

			// Draw enemy car
			iShowImage((int)e[i].x, (int)e[i].y, ENEMY_W, ENEMY_H, imgEnemy);

			// Draw health bar on top of enemy car (only if health > 0)
			if (e[i].health > 0 && e[i].health <= EnemyCar::MAX_HEALTH && imgEnemyHP[e[i].health] >= 0) {
				// Position health bar slightly above the enemy car
				int hpX = (int)e[i].x;
				int hpY = (int)e[i].y + ENEMY_H - 20;  // 20 pixels above the car
				int hpW = 80;   // Width of health bar image (adjust based on your image)
				int hpH = 12;   // Height of health bar image (adjust based on your image)

				iShowImage(hpX + (ENEMY_W - hpW) / 2, hpY, hpW, hpH, imgEnemyHP[e[i].health]);
			}
		}

		// Draw explosions after cars so they're visible on top
		drawExplosions();
	}
};

#endif
