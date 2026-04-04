#ifndef ENEMY_HPP
#define ENEMY_HPP

#include <cstdlib>
#include <cmath>   // fabsf
#include <mmsystem.h>  // For mciSendString

// Forward declare constants from main
extern const int ROAD_LEFT_X;
extern const int ROAD_RIGHT_X;

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

	// Health system for enemy cars (3 health levels)
	int health;         // current health (3 = full, 2 = damaged, 1 = critical, 0 = dead)
	static const int MAX_HEALTH = 3;  // Enemy takes 3 hits to destroy

	// Track hits by player car (for combo scoring)
	int hitsByCar;      // Number of times player car hit this enemy

	// Boss flag
	bool isBoss;        // True if this is the boss
};

struct EnemySystem {

	static const int ENEMY_W = 116;
	static const int ENEMY_H = 169;

	// Boss size (slightly larger for dramatic effect)
	static const int BOSS_W = 140;
	static const int BOSS_H = 190;

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
	int imgBoss;           // Boss image
	int imgEnemyHP[4];     // Array for health images (0 unused, 1-3 for health levels)
	int imgBossHP[10];     // Boss health images (boss_hp_1.png to boss_hp_9.png)
	EnemyCar e[4];         // Increased to 4 to accommodate boss (3 normal + 1 boss)
	int spawnCooldown;

	// Boss system variables
	bool bossSpawned;      // Has boss been spawned yet?
	int bossSpawnTimer;    // Timer for boss spawn (10 seconds after level start)
	int bossWarningTimer;  // Timer for warning text
	bool showBossWarning;  // Whether to show warning text
	int bossDefeated;      // Has boss been defeated?
	int normalEnemySpawnPaused; // Whether normal enemy spawning is paused due to boss
	int bossCurrentHealth; // Store boss health across respawns
	int currentDifficulty;

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
		imgBoss = -1;

		// Initialize health images to -1
		for (int i = 0; i <= EnemyCar::MAX_HEALTH; i++) {
			imgEnemyHP[i] = -1;
		}

		// Initialize boss health images
		for (int i = 0; i < 10; i++) {
			imgBossHP[i] = -1;
		}

		// Initialize explosion image to -1
		explosionImg = -1;

		// Initialize explosions
		for (int i = 0; i < 5; i++) {
			explosions[i].active = false;
		}

		for (int i = 0; i < 4; i++) {
			e[i].active = 0;
			e[i].health = EnemyCar::MAX_HEALTH;
			e[i].isBoss = false;
		}
		spawnCooldown = spawnMinTicks;

		// Boss system initialization
		bossSpawned = false;
		bossSpawnTimer = 0;
		bossWarningTimer = 0;
		showBossWarning = false;
		bossDefeated = false;
		normalEnemySpawnPaused = false;
		bossCurrentHealth = 9;
		currentDifficulty = 0;
	}

	void loadImages() {
		imgEnemy = iLoadImage("Assets\\game\\enemy_car.png");
		imgBoss = iLoadImage("Assets\\game\\boss.png");

		// Load health bar images (e_hp_1.png, e_hp_2.png, e_hp_3.png)
		for (int i = 1; i <= EnemyCar::MAX_HEALTH; i++) {
			char path[128];
			sprintf_s(path, "Assets\\hp\\e_hp_%d.png", i);
			imgEnemyHP[i] = iLoadImage(path);
		}

		// Load boss health bar images (boss_hp_1.png to boss_hp_9.png)
		for (int i = 1; i <= 9; i++) {
			char path[128];
			sprintf_s(path, "Assets\\hp\\boss_hp_%d.png", i);
			imgBossHP[i] = iLoadImage(path);
		}

		// Load explosion image
		explosionImg = iLoadImage("Assets\\game\\explosion.png");
	}

	// Sound effect methods
	void playEnemyDestroyedSound() {
		mciSendString("stop enemy_destroyed", NULL, 0, NULL);
		mciSendString("seek enemy_destroyed to start", NULL, 0, NULL);
		mciSendString("play enemy_destroyed", NULL, 0, NULL);
	}

	void playAnasLaughSound() {
		int randomChance = rand() % 4; // 0, 1, 2, or 3 (25% chance)
		if (randomChance == 0) {
			mciSendString("stop anas_laugh", NULL, 0, NULL);
			mciSendString("seek anas_laugh to start", NULL, 0, NULL);
			mciSendString("play anas_laugh", NULL, 0, NULL);
		}
	}

	// Boss system functions
	void initBossSystem(int difficulty) {
		currentDifficulty = difficulty;

		bossSpawned = false;
		bossSpawnTimer = 0;
		bossWarningTimer = 0;
		showBossWarning = false;
		bossDefeated = false;
		normalEnemySpawnPaused = false;
		bossCurrentHealth = 9;

		// Only initialize boss system for HARD difficulty
		if (difficulty == 2) { // DIFF_HARD
			bossSpawnTimer = 833;
			bossWarningTimer = 416;
		}
	}

	void updateBossSystem(int gameTick) {
		// Only proceed if boss hasn't been spawned and not defeated
		if (bossSpawned || bossDefeated) return;

		// Handle warning text
		if (bossWarningTimer > 0) {
			bossWarningTimer--;
			if (bossWarningTimer <= 0) {
				showBossWarning = false;
			}
			else if (bossWarningTimer <= 416 && bossWarningTimer > 400) {
				// Show warning for 3 seconds (from 5 seconds to 2 seconds before spawn)
				showBossWarning = true;
			}
		}

		// Handle boss spawn timer
		if (bossSpawnTimer > 0) {
			bossSpawnTimer--;
			if (bossSpawnTimer <= 0) {
				spawnBoss();
			}
		}
	}

	void spawnBoss() {
		if (bossSpawned || bossDefeated) return;

		// Find a slot for boss
		int idx = -1;
		for (int i = 0; i < 4; i++) {
			if (!e[i].active) {
				idx = i;
				break;
			}
		}
		if (idx < 0) return;

		bossSpawned = true;
		normalEnemySpawnPaused = true; // Pause normal enemy spawning

		e[idx].active = 1;
		e[idx].phase = 0;  // Start in approach phase (comes from top)
		e[idx].side = (rand() % 2) ? 1 : -1;
		e[idx].bumpsLeft = bumpCount;
		e[idx].bumpDir = 0;
		e[idx].waitTicks = 0;
		e[idx].attackLocked = 0;
		e[idx].attackX = 0.0f;
		e[idx].hitCooldown = 0;

		// Use stored boss health
		e[idx].health = bossCurrentHealth;

		e[idx].hitsByCar = 0;
		e[idx].isBoss = true;

		// Spawn from TOP of screen (Y = 100) like normal enemies
		e[idx].y = 100.0f;

		// Calculate spawn position - start from the side like normal enemies
		float carX = 354; // Approximate car X (center) - will be updated in update loop
		float carW = 157;
		float spawnX = (e[idx].side == -1)
			? (carX - BOSS_W - farGap)
			: (carX + carW + farGap);

		// Ensure boss stays within road boundaries
		if (spawnX < ROAD_LEFT_X) spawnX = ROAD_LEFT_X;
		if (spawnX > ROAD_RIGHT_X - BOSS_W) spawnX = ROAD_RIGHT_X - BOSS_W;

		e[idx].x = spawnX;
	}

	void bossDefeatedAction() {
		bossDefeated = true;
		normalEnemySpawnPaused = false; // Resume normal enemy spawning
		bossSpawned = false;
	}

	bool isBossActive() {
		for (int i = 0; i < 4; i++) {
			if (e[i].active && e[i].isBoss) return true;
		}
		return false;
	}

	bool shouldPauseNormalSpawning() {
		// Only pause spawning on Hard difficulty when boss hasn't been defeated
		if (currentDifficulty == 2) { // DIFF_HARD
			return !bossDefeated && (bossSpawned || bossSpawnTimer <= 0);
		}
		// For Easy and Medium, always allow spawning
		return false;
	}

	void addExplosion(float x, float y, bool isBoss = false) {
		// Find an inactive explosion slot
		for (int i = 0; i < 5; i++) {
			if (!explosions[i].active) {
				explosions[i].active = true;
				explosions[i].x = x;
				explosions[i].y = y;
				explosions[i].duration = 15;  // Show explosion for ~0.25 seconds
				// Boss gets bigger explosion
				if (isBoss) {
					explosions[i].width = 180;
					explosions[i].height = 180;
				}
				else {
					explosions[i].width = 120;
					explosions[i].height = 120;
				}
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
				int drawX = (int)(explosions[i].x - (explosions[i].width - ENEMY_W) / 2);
				int drawY = (int)(explosions[i].y - (explosions[i].height - ENEMY_H) / 2);
				iShowImage(drawX, drawY, explosions[i].width, explosions[i].height, explosionImg);
			}
		}
	}

	// Function to increment car hit count and return combo points
	int incrementCarHit(int enemyIndex) {
		if (enemyIndex < 0 || enemyIndex >= 4) return 0;
		if (!e[enemyIndex].active) return 0;

		e[enemyIndex].hitsByCar++;
		int hits = e[enemyIndex].hitsByCar;

		// Return bonus points for each hit (50 points per hit, with combo bonus on 3rd hit)
		if (hits == 3) {
			return 100;  // Bonus 100 points for 3rd hit (total will be 50 + 100 = 150)
		}
		return 50;  // Regular 50 points for each hit
	}

	// Reset car hits for an enemy (when enemy is destroyed)
	void resetCarHits(int enemyIndex) {
		if (enemyIndex < 0 || enemyIndex >= 4) return;
		e[enemyIndex].hitsByCar = 0;
	}

	void reset() {
		for (int i = 0; i < 4; i++) {
			e[i].active = 0;
			e[i].health = EnemyCar::MAX_HEALTH;
			e[i].hitsByCar = 0;
			e[i].isBoss = false;
		}
		// Clear all explosions
		for (int i = 0; i < 5; i++) {
			explosions[i].active = false;
		}
		spawnCooldown = spawnMinTicks + (spawnRangeTicks ? (rand() % (spawnRangeTicks + 1)) : 0);

		// Reset boss system
		bossSpawned = false;
		bossSpawnTimer = 0;
		bossWarningTimer = 0;
		showBossWarning = false;
		bossDefeated = false;
		normalEnemySpawnPaused = false;
		bossCurrentHealth = 9;
	}

	void setEnabled(int on) {
		enabled = on ? 1 : 0;
		if (!enabled) reset();
	}

	int activeCount() const {
		int c = 0;
		for (int i = 0; i < 4; i++) if (e[i].active) c++;
		return c;
	}

	// Function to reduce enemy health when hit by bat
	void reduceHealth(int enemyIndex) {
		if (enemyIndex < 0 || enemyIndex >= 4) return;
		if (!e[enemyIndex].active) return;

		bool wasAlive = (e[enemyIndex].health > 0);
		e[enemyIndex].health--;

		// If this is the boss, update stored health
		if (e[enemyIndex].isBoss) {
			bossCurrentHealth = e[enemyIndex].health;
		}

		if (e[enemyIndex].health <= 0 && wasAlive) {
			// Play enemy destroyed sound
			playEnemyDestroyedSound();

			// Play anas laugh with 25% chance (only on hard difficulty)
			if (currentDifficulty == 2) { // DIFF_HARD
				playAnasLaughSound();
			}

			// Add explosion at enemy position before destroying
			addExplosion(e[enemyIndex].x, e[enemyIndex].y, e[enemyIndex].isBoss);
			// Reset car hits for this enemy when destroyed
			resetCarHits(enemyIndex);

			// If this was the boss, trigger boss defeated actions
			if (e[enemyIndex].isBoss) {
				bossDefeatedAction();
				bossCurrentHealth = 9; // Reset for next game session
			}

			// Enemy destroyed
			e[enemyIndex].active = 0;
		}
	}

	// Get enemy health for damage calculation
	int getEnemyHealth(int enemyIndex) {
		if (enemyIndex < 0 || enemyIndex >= 4) return 0;
		if (!e[enemyIndex].active) return 0;
		return e[enemyIndex].health;
	}

	// Check if enemy is boss
	bool isBossEnemy(int enemyIndex) {
		if (enemyIndex < 0 || enemyIndex >= 4) return false;
		if (!e[enemyIndex].active) return false;
		return e[enemyIndex].isBoss;
	}

	// Get damage amount when colliding with enemy (boss does 2 damage)
	int getCollisionDamage(int enemyIndex) {
		if (enemyIndex < 0 || enemyIndex >= 4) return 1;
		if (!e[enemyIndex].active) return 1;
		if (e[enemyIndex].isBoss) return 2;  // Boss does 2 damage
		return 1;  // Normal enemy does 1 damage
	}

	// Get points for defeating enemy (boss gives 500 points)
	int getDefeatPoints(int enemyIndex) {
		if (enemyIndex < 0 || enemyIndex >= 4) return 100;
		if (!e[enemyIndex].active) return 100;
		if (e[enemyIndex].isBoss) return 500;  // Boss gives 500 points
		return 100;  // Normal enemy gives 100 points
	}

	// Get rage reduction for defeating enemy (boss reduces 500 rage)
	int getRageReduction(int enemyIndex) {
		if (enemyIndex < 0 || enemyIndex >= 4) return 100;
		if (!e[enemyIndex].active) return 100;
		if (e[enemyIndex].isBoss) return 500;  // Boss reduces 500 rage
		return 100;  // Normal enemy reduces 100 rage
	}

	// Check if bat hit any enemy (returns index of hit enemy, -1 if none)
	int checkBatHit(float batX, float batY, float batW, float batH) {
		for (int i = 0; i < 4; i++) {
			if (!e[i].active) continue;

			int enemyW = e[i].isBoss ? BOSS_W : ENEMY_W;
			int enemyH = e[i].isBoss ? BOSS_H : ENEMY_H;

			// Check collision between bat and enemy
			if (batX < e[i].x + enemyW &&
				batX + batW > e[i].x &&
				batY < e[i].y + enemyH &&
				batY + batH > e[i].y) {
				return i;  // Return index of hit enemy
			}
		}
		return -1;  // No hit
	}

	void spawnOne(float carX, float carY, float carW, float roadLeft, float roadRight) {
		if (!enabled) return;

		// Don't spawn normal enemies if boss is active or if boss hasn't been defeated yet
		if (shouldPauseNormalSpawning()) return;

		if (activeCount() >= maxEnemy) return;

		int idx = -1;
		for (int i = 0; i < 4; i++) {
			if (!e[i].active && !e[i].isBoss) { idx = i; break; }
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

		// Reset car hits for new enemy
		e[idx].hitsByCar = 0;
		e[idx].isBoss = false;

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

		// spawn control - only spawn normal enemies if boss not active
		if (spawnCooldown > 0) spawnCooldown--;
		if (spawnCooldown <= 0) {
			if (!nitroOn && !shouldPauseNormalSpawning()) {
				spawnOne(carX, carY, carW, roadLeft, roadRight);
			}
			spawnCooldown = spawnMinTicks + (spawnRangeTicks ? (rand() % (spawnRangeTicks + 1)) : 0);
		}

		const float EPS = 0.01f;

		for (int i = 0; i < 4; i++) {
			if (!e[i].active) continue;

			int enemyW = e[i].isBoss ? BOSS_W : ENEMY_W;
			int enemyH = e[i].isBoss ? BOSS_H : ENEMY_H;
			float speedMultiplier = e[i].isBoss ? 0.8f : 1.0f; // Boss moves slightly slower

			if (e[i].hitCooldown > 0) e[i].hitCooldown--;

			float relY = nitroOn ? -8.0f : 0.0f;

			if (e[i].phase == 0) {
				relY += approachGain * speedMultiplier;
				e[i].y += relY;

				float targetX = (e[i].side == -1)
					? (carX - enemyW - farGap)
					: (carX + carW + farGap);
				e[i].x = moveToward(e[i].x, targetX, followXSpeed * speedMultiplier);

				if (e[i].y >= carY) {
					e[i].y = carY;
					e[i].phase = 1;
					e[i].waitTicks = sideHoldTicks;
				}
			}
			else if (e[i].phase == 1) {
				// lock Y to your car so it truly stays side-by-side
				if (!nitroOn) {
					e[i].y = moveToward(e[i].y, carY, sideYLockSpeed * speedMultiplier);
				}

				if (e[i].waitTicks > 0) e[i].waitTicks--;

				float targetX = (e[i].side == -1)
					? (carX - enemyW - farGap)
					: (carX + carW + farGap);
				e[i].x = moveToward(e[i].x, targetX, followXSpeed * sideXSpeedMul * speedMultiplier);

				if (e[i].waitTicks <= 0) {
					e[i].phase = 2;
					e[i].bumpDir = 0;

					// lock attack line ONCE so dodging can cause a miss
					e[i].attackLocked = 1;
					e[i].attackX = (e[i].side == -1)
						? (carX - enemyW - nearGap)
						: (carX + carW + nearGap);
				}
			}
			else if (e[i].phase == 2) {
				// keep Y aligned while attempting bump
				if (!nitroOn) {
					e[i].y = moveToward(e[i].y, carY, sideYLockSpeed * speedMultiplier);
				}

				float inX = e[i].attackX;

				float outX = (e[i].side == -1)
					? (carX - enemyW - farGap)
					: (carX + carW + farGap);

				float targetX = (e[i].bumpDir == 0) ? inX : outX;
				e[i].x = moveToward(e[i].x, targetX, (followXSpeed + 2.0f) * speedMultiplier);

				// decide hit/miss ONLY when enemy actually reaches the locked attack line
				if (e[i].bumpDir == 0 && fabsf(e[i].x - inX) <= contactWindow) {

					// Where would contact be *right now* if player did not dodge?
					float nowInX = (e[i].side == -1)
						? (carX - enemyW - nearGap)
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
						int damage = getCollisionDamage(i);

						if (carX < roadLeft) carX = roadLeft;
						if (carX > roadRight - carW) carX = roadRight - carW;

						// Also push the enemy car in opposite direction and clamp to road
						e[i].x -= e[i].side * bumpPush;
						if (e[i].x < roadLeft) e[i].x = roadLeft;
						if (e[i].x > roadRight - enemyW) e[i].x = roadRight - enemyW;

						e[i].hitCooldown = 45; // ~0.75s
						if (outPlayerHit) *outPlayerHit = damage; // Return damage amount
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
							? (carX - enemyW - nearGap)
							: (carX + carW + nearGap);

						e[i].waitTicks = 10;
					}
				}

				if (e[i].waitTicks > 0) e[i].waitTicks--;

				if (clampEnabled) {
					float minX = (e[i].side == -1)
						? (carX - enemyW - nearGap)
						: (carX + carW + nearGap);

					if (e[i].side == -1 && e[i].x > minX) e[i].x = minX;
					if (e[i].side == 1 && e[i].x < minX) e[i].x = minX;
				}
			}
			else {
				// PASS: slow pass + linger, X fixed for aiming later
				float gain = (e[i].y < carY + passSlowAfter) ? passFastGain : passSlowGain;
				e[i].y += (nitroOn ? -6.0f : gain) * speedMultiplier;

				// IMPORTANT: keep X unchanged

				if (e[i].y > 1600.0f || e[i].y < -500.0f) {
					e[i].active = 0;
					if (e[i].isBoss) {
						// Boss not defeated, will respawn later - keep health stored
						bossSpawned = false;
						bossSpawnTimer = 416; // Respawn after 5 seconds
						// bossCurrentHealth already has the current health, so it persists
					}
				}
			}
		}

		return carX;
	}

	void draw() const {
		if (!enabled) return;

		// First pass: draw all normal enemies (non-boss)
		for (int i = 0; i < 4; i++) {
			if (!e[i].active) continue;
			if (e[i].isBoss) continue; // Skip boss in first pass

			int enemyW = ENEMY_W;
			int enemyH = ENEMY_H;

			// Draw enemy car
			iShowImage((int)e[i].x, (int)e[i].y, enemyW, enemyH, imgEnemy);

			// Draw health bar for normal enemies
			if (e[i].health > 0 && e[i].health <= EnemyCar::MAX_HEALTH && imgEnemyHP[e[i].health] >= 0) {
				int hpX = (int)e[i].x;
				int hpY = (int)e[i].y + enemyH - 20;
				int hpW = 80;
				int hpH = 12;
				iShowImage(hpX + (enemyW - hpW) / 2, hpY, hpW, hpH, imgEnemyHP[e[i].health]);
			}
		}

		// Second pass: draw boss (so it appears on top of normal enemies)
		for (int i = 0; i < 4; i++) {
			if (!e[i].active) continue;
			if (!e[i].isBoss) continue; // Only draw boss in second pass

			int enemyW = BOSS_W;
			int enemyH = BOSS_H;

			// Draw boss car
			iShowImage((int)e[i].x, (int)e[i].y, enemyW, enemyH, imgBoss);

			// Draw boss health bar
			if (e[i].health > 0 && e[i].health <= 9 && imgBossHP[e[i].health] >= 0) {
				int hpX = (int)e[i].x;
				int hpY = (int)e[i].y + enemyH + 5;
				int hpW = 100;
				int hpH = 15;
				iShowImage(hpX + (enemyW - hpW) / 2, hpY, hpW, hpH, imgBossHP[e[i].health]);
			}
		}

		// Draw explosions after all cars (so they're visible on top)
		drawExplosions();
	}

	// Draw boss warning text
	void drawBossWarning(int screenW, int screenH) const {
		if (showBossWarning) {
			iSetColor(255, 0, 0);
			iText(screenW / 2 - 100, screenH / 2, "Prepare for boss fight!", GLUT_BITMAP_TIMES_ROMAN_24);
			iSetColor(255, 255, 255);
		}
	}
};

#endif
