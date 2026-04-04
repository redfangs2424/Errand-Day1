#include "iGraphics.h"
#include "Enemy.hpp"
#include "Button.hpp"
#include "Obstacle.hpp"
#include "Nitro.hpp"
#include "HealthSystem.hpp"
#include "Score.hpp"
#include "HighScore.hpp"
#include <cstdlib>
#include <cctype>

int x, y;

// ==============================
//            CONSTANTS
// ==============================
const int SCREEN_W = 708;
const int SCREEN_H = 1080;

// ROAD
const int ROAD_LEFT_X = 72;
const int ROAD_RIGHT_X = 636;

double roadOffsetY;                 // Vertical road scrolling
const double MIN_SPEED = 4.0;
double roadSpeed = MIN_SPEED;

// CAR
const int CAR_W = 157;
const int CAR_H = 169;
double carX;
const int CAR_Y = 200;              // uplift distance from bottom

// ==============================
//          BAT SYSTEM
// ==============================
const int BAT_FRAMES = 10;

// Left swing frames (original)
int batImgL[BAT_FRAMES];

// Right swing frames (mirrored sprites for passenger side)
int batImgR[BAT_FRAMES];

// Drawing adjustments (same as your original)
const int BAT_EXTRA_W = 24;
const int BAT_OFFSET_X = -24;
const int BAT_OFFSET_Y = 0;

// Right side draws at different X position
const int BAT_OFFSET_X_R = 0;
const int BAT_OFFSET_Y_R = 0;

// BAT HITBOX CONFIGURATION - UNCHANGED from your working version
const int BAT_HITBOX_EXTRA_W = 80;   // Additional width for hitbox
const int BAT_HITBOX_EXTRA_H = 30;   // Additional height for hitbox
const int BAT_HITBOX_OFFSET_X = -20; // Adjust X position offset for better coverage
const float BAT_KNOCKBACK_FORCE = 35.0f; // Knockback force when hitting enemies

enum BatSwingSide {
	BAT_SWING_LEFT,
	BAT_SWING_RIGHT
};

BatSwingSide currentBatSide = BAT_SWING_LEFT;
bool batPlaying = false;
int batFrame = 0;

// ==============================
//     HEALTH PICKUP SYSTEM
// ==============================
struct HealthPickup {
	float x, y;          // Position on road
	bool active;         // Whether it exists
	int lifetime;        // Timer for disappearance (4 seconds = 333 ticks at 12ms)
};

const int MAX_HEALTH_PICKUPS = 10;
HealthPickup healthPickups[MAX_HEALTH_PICKUPS];
int healthPickupImg = -1;  // Image for health pickup

// ==============================
//         MENU / RESOURCES
// ==============================
int mouseX, mouseY;
int menuBG;
int scoreBG;
static bool audioOpened = false;
static bool gameOverSoundPlayed = false;
static int enemyCollisionCooldown = 0;

// DISTANCE TRACKING SYSTEM
static double distanceTraveled = 0.0;  // Distance in km
static int distanceTimerTicks = 0;      // Counter for 10-second intervals
static const int TICKS_PER_10_SEC = 833; // 10 seconds at 12ms per tick

// LEVEL COMPLETION SYSTEM
static bool levelCompleted = false;
static int levelCompleteTimer = 0;
static const int LEVEL_COMPLETE_DURATION = 832; // 10 seconds
static bool isLevelCompletion = false; // Track if we're coming from level completion
static bool rageRequirementFailed = false; // Track if player failed rage requirement

// Boss system variables
static int gameTickCounter = 0;  // Counter for boss system timing

// Distance goals for each difficulty
const double DISTANCE_GOAL_EASY = 20.0;
const double DISTANCE_GOAL_MEDIUM = 25.0;
const double DISTANCE_GOAL_HARD = 30.0;

// Rage (Score) requirements for each difficulty
const int RAGE_REQUIREMENT_EASY = 1500;
const int RAGE_REQUIREMENT_MEDIUM = 2000;
const int RAGE_REQUIREMENT_HARD = 2500;

struct MenuButtons {
	Button start;
	Button highScores;
	Button options;
	Button gameExit;
	Button easy;
	Button medium;
	Button hard;
	Button credits;
	Button controls;
	Button back;
} btn;

// GAME RESOURCES
int roadImgA;
int roadImgB;
int carImg;
int carBrakedImg;
int obsImg[OB_COUNT];
NitroSystem nitro;
int carLaneX[4];

EnemySystem enemy;
// SCORE SYSTEM
ScoreSystem score;

//HIGH SCORE SYSTEM
HighScoreSystem highScores;
bool waitingForName = false;
char playerName[50] = "";
int nameInputIndex = 0;
int finalScore = 0;
int finalDifficulty = 0;

// HEALTH + PAUSE
HealthSystem health;
bool gamePaused = false;
bool prevP = false;


// DAMAGE BLINK
bool damageBlink = false;
int blinkTicks = 0;

// One-key "just pressed" tracking
bool prevSpace = false;   // for bat swing (Space key)
bool prevN = false;       // for nitro (N key)
bool prevB = false;       // For bat click edge-trigger tracking (keep for mouse click)

// ==============================
//            STATE
// ==============================
enum Page {
	PAGE_HOME,
	PAGE_START,
	PAGE_HIGHSCORE,
	PAGE_OPTIONS,
	PAGE_CONTROLS,
	PAGE_CREDITS,
	PAGE_GAME,
	PAGE_GAMEOVER,
	PAGE_LEVEL_COMPLETE
};

enum Difficulty {
	DIFF_EASY,
	DIFF_MEDIUM,
	DIFF_HARD,
};

Page page = PAGE_HOME;
Difficulty difficulty = DIFF_EASY;

struct DifficultyConfig {
	double maxSpeed;
	double accelRate;
	double brakeRate;
	double frictionRate;
	double steerMin;
	double steerMax;

	int damageBlinkTicks;
	double crashSpeedReset;

	ObstacleSpawnConfig obstacles;
};

DifficultyConfig cfg;

// ==============================
//          FORWARD DECLS
// ==============================
void drawHome();
void drawStart();
void drawOptions();
void drawControls();
void drawCredits();
void drawGame();
void drawGameOver();
void drawHighScore();
void drawLevelComplete();

void initBackground();
void initImages();

void setDifficulty(Difficulty d);
void initGame();
void updateGame();
void handleBatSwing();
void updateBatAnimation();
void goToGameOver();

BatSwingSide chooseBatSwingSide();

// Health pickup functions
void initHealthPickups();
void spawnHealthPickup(float x, float y);
void updateHealthPickups(float roadSpeed);
void drawHealthPickups();
void checkHealthPickupCollision();

// Button handlers
void startButtonClickHandler();
void backButtonClickHandler();
void optionsButtonClickHandler();
void controlsButtonClickHandler();
void creditsButtonClickHandler();
void easyButtonClickHandler();
void mediumButtonClickHandler();
void hardButtonClickHandler();
void highScoreButtonClickHandler();

//  AUDIO forward declarations
void initAudio();
void playBackgroundMusic();
void stopBackgroundMusic();
void playGameOverMusicOnce();
void playLevelCompleteSound();
void playCarDamageSound();
void playBossDamageSound();
void playPowerUpSound();

// Distance tracking functions
void resetDistance() {
	distanceTraveled = 0.0;
	distanceTimerTicks = 0;
}

void updateDistance(bool nitroActive) {
	// Add distance every frame based on current nitro status
	// Normal: 1 km per 10 seconds = 0.0012 km per tick (12ms per tick)
	// Nitro: 5 km per 10 seconds = 0.006 km per tick
	if (nitroActive) {
		distanceTraveled += 0.006;  // 5 km per 10 seconds when nitro active
	}
	else {
		distanceTraveled += 0.0012; // 1 km per 10 seconds when normal
	}

	// Optional: Keep timer for reference
	distanceTimerTicks++;
	if (distanceTimerTicks >= TICKS_PER_10_SEC) {
		distanceTimerTicks = 0;
	}
}

void drawDistance() {
	char distText[50];
	sprintf_s(distText, "Distance: %.1f km", distanceTraveled);
	iSetColor(255, 255, 255);
	iText(SCREEN_W - 640, SCREEN_H - 70, "Health", GLUT_BITMAP_HELVETICA_18);
	iText(SCREEN_W - 680, SCREEN_H - 125, distText, GLUT_BITMAP_HELVETICA_18);
}

// Level completion functions
double getDistanceGoal() {
	switch (difficulty) {
	case DIFF_EASY: return DISTANCE_GOAL_EASY;
	case DIFF_MEDIUM: return DISTANCE_GOAL_MEDIUM;
	case DIFF_HARD: return DISTANCE_GOAL_HARD;
	default: return DISTANCE_GOAL_EASY;
	}
}

int getRageRequirement() {
	switch (difficulty) {
	case DIFF_EASY: return RAGE_REQUIREMENT_EASY;
	case DIFF_MEDIUM: return RAGE_REQUIREMENT_MEDIUM;
	case DIFF_HARD: return RAGE_REQUIREMENT_HARD;
	default: return RAGE_REQUIREMENT_EASY;
	}
}

void checkLevelCompletion() {
	if (levelCompleted) return;

	double goal = getDistanceGoal();
	int rageReq = getRageRequirement();
	int currentScore = score.getScore();

	if (distanceTraveled >= goal) {
		// Check if score meets rage requirement
		if (currentScore >= rageReq) {
			// Level complete!
			levelCompleted = true;
			levelCompleteTimer = 0;
			isLevelCompletion = true;
			rageRequirementFailed = false;
			page = PAGE_LEVEL_COMPLETE;

			// Store final stats for level completion
			finalScore = currentScore;
			finalDifficulty = difficulty;

			// Stop background music and play level complete sound
			stopBackgroundMusic();
			playLevelCompleteSound();
		}
		else {
			// Failed rage requirement - game over with special message
			rageRequirementFailed = true;
			isLevelCompletion = false;
			goToGameOver();
		}
	}
}

void resetLevelCompletion() {
	levelCompleted = false;
	levelCompleteTimer = 0;
	isLevelCompletion = false;
	rageRequirementFailed = false;
}

// ==============================
//              UTILS
// ==============================
bool inside(int mx, int my, int x1, int x2, int y1, int y2) {
	return mx >= x1 && mx <= x2 && my >= y1 && my <= y2;
}

double clampDouble(double v, double lo, double hi) {
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

// ==============================
//      BAT SIDE SELECTION (FOR ANIMATION ONLY)
// ==============================
BatSwingSide chooseBatSwingSide()
{
	float carCenterX = (float)carX + CAR_W / 2.0f;

	for (int i = 0; i < 4; i++) {
		if (!enemy.e[i].active) continue;

		int enemyW = enemy.e[i].isBoss ? 140 : 116;
		float enemyCenterX = enemy.e[i].x + enemyW / 2.0f;

		// Just pick the side based on enemy position relative to car center
		if (enemyCenterX < carCenterX) {
			return BAT_SWING_LEFT;
		}
		else {
			return BAT_SWING_RIGHT;
		}
	}

	return BAT_SWING_LEFT; // Default
}

// ==============================
//              DRAW
// ==============================
void iDraw()
{
	iClear();

	switch (page) {
	case PAGE_HOME:     drawHome();     break;
	case PAGE_START:    drawStart();    break;
	case PAGE_HIGHSCORE:    drawHighScore();    break;
	case PAGE_OPTIONS:  drawOptions();  break;
	case PAGE_CONTROLS: drawControls(); break;
	case PAGE_CREDITS:  drawCredits();  break;
	case PAGE_GAME:     drawGame();     break;
	case PAGE_GAMEOVER: drawGameOver(); break;
	case PAGE_LEVEL_COMPLETE: drawLevelComplete(); break;
	}
}

void iMouseMove(int mx, int my) {}

void iPassiveMouseMove(int mx, int my)
{
	mouseX = mx;
	mouseY = my;
}

void iMouse(int button, int state, int mx, int my)
{
	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

	printf("x: %d\t y: %d\n", mx, my);

	if (page == PAGE_HOME) {
		if (mx >= 316 && mx <= 602 && my >= 240 && my <= 321) {
			startButtonClickHandler();
		}
		else if (mx >= 321 && mx <= 645 && my >= 165 && my <= 225) {
			highScoreButtonClickHandler();
		}
		else if (mx >= 316 && mx <= 549 && my >= 68 && my <= 149) {
			optionsButtonClickHandler();
		}
		else if (mx >= 553 && mx <= 634 && my >= 68 && my <= 149) {
			exit(0);
		}
	}
	else if (page == PAGE_HIGHSCORE) {
		if (mx >= 64 && mx <= 144 && my >= SCREEN_H - 80 && my <= SCREEN_H) {
			backButtonClickHandler();
		}
	}
	else if (page == PAGE_START) {
		if (mx >= 312 && mx <= 392 && my >= 240 && my <= 320) {
			backButtonClickHandler();
		}
		else if (mx >= 402 && mx <= 633 && my >= 240 && my <= 320) {
			easyButtonClickHandler();
		}
		else if (mx >= 402 && mx <= 633 && my >= 154 && my <= 234) {
			mediumButtonClickHandler();
		}
		else if (mx >= 402 && mx <= 633 && my >= 66 && my <= 146) {
			hardButtonClickHandler();
		}
	}
	else if (page == PAGE_OPTIONS) {
		if (mx >= 268 && mx <= 348 && my >= 240 && my <= 320) {
			backButtonClickHandler();
		}
		else if (mx >= 356 && mx <= 631 && my >= 240 && my <= 320) {
			controlsButtonClickHandler();
		}
		else if (mx >= 356 && mx <= 630 && my >= 152 && my <= 232) {
			creditsButtonClickHandler();
		}
	}
	else if (page == PAGE_CONTROLS) {
		if (mx >= 64 && mx <= 144 && my >= 652 && my <= 732) {
			optionsButtonClickHandler();
		}
	}
	else if (page == PAGE_CREDITS) {
		if (mx >= 74 && mx <= 154 && my >= 532 && my <= 612) {
			optionsButtonClickHandler();
		}
	}
	else if (page == PAGE_GAME) {
		if (!gamePaused) {
			currentBatSide = chooseBatSwingSide();
			batPlaying = true;
			batFrame = 0;
		}
	}
	else if (page == PAGE_GAMEOVER) {
		if (waitingForName) {
			if (strlen(playerName) > 0) {
				highScores.addScore(playerName, finalScore, finalDifficulty);
			}
			waitingForName = false;
			page = PAGE_HOME;
			gameOverSoundPlayed = false;
			playBackgroundMusic();
		}
		else {
			page = PAGE_HOME;
			gameOverSoundPlayed = false;
			playBackgroundMusic();
		}
	}
	else if (page == PAGE_LEVEL_COMPLETE) {
		waitingForName = true;
		memset(playerName, 0, sizeof(playerName));
		nameInputIndex = 0;
		isLevelCompletion = true;
		page = PAGE_GAMEOVER;
		playBackgroundMusic(); // Restart music when leaving level complete
	}
}

void fixedUpdate()
{
	updateGame();
}

// ==============================
//        INIT RESOURCES
// ==============================
void initBackground()
{
	menuBG = iLoadImage("Assets\\global\\menu_background.png");
	scoreBG = iLoadImage("Assets\\global\\menu_background.png");
}

void initImages()
{
	/*------------------------------------MENU------------------------------------*/
	// GLOBAL
	btn.back.normal = iLoadImage("Assets\\global\\buttons\\back.png");
	btn.back.hover = iLoadImage("Assets\\global\\buttons\\back_hover.png");

	// HOME PAGE
	btn.start.normal = iLoadImage("Assets\\home\\buttons\\start.png");
	btn.highScores.normal = iLoadImage("Assets\\home\\buttons\\high_scores.png");
	btn.options.normal = iLoadImage("Assets\\home\\buttons\\options.png");
	btn.gameExit.normal = iLoadImage("Assets\\home\\buttons\\exit.png");

	btn.start.hover = iLoadImage("Assets\\home\\buttons\\start_hover.png");
	btn.highScores.hover = iLoadImage("Assets\\home\\buttons\\high_scores_hover.png");
	btn.options.hover = iLoadImage("Assets\\home\\buttons\\options_hover.png");
	btn.gameExit.hover = iLoadImage("Assets\\home\\buttons\\exit_hover.png");

	// START PAGE
	btn.easy.normal = iLoadImage("Assets\\start\\buttons\\easy.png");
	btn.medium.normal = iLoadImage("Assets\\start\\buttons\\medium.png");
	btn.hard.normal = iLoadImage("Assets\\start\\buttons\\hard.png");

	btn.easy.hover = iLoadImage("Assets\\start\\buttons\\easy_hover.png");
	btn.medium.hover = iLoadImage("Assets\\start\\buttons\\medium_hover.png");
	btn.hard.hover = iLoadImage("Assets\\start\\buttons\\hard_hover.png");

	// OPTIONS PAGE
	btn.credits.normal = iLoadImage("Assets\\options\\buttons\\credits.png");
	btn.controls.normal = iLoadImage("Assets\\options\\buttons\\controls.png");

	btn.credits.hover = iLoadImage("Assets\\options\\buttons\\credits_hover.png");
	btn.controls.hover = iLoadImage("Assets\\options\\buttons\\controls_hover.png");

	/*------------------------------------GAME------------------------------------*/
	roadImgA = iLoadImage("Assets\\game\\road.png");
	roadImgB = iLoadImage("Assets\\game\\road.png");
	carImg = iLoadImage("Assets\\game\\car.png");
	carBrakedImg = iLoadImage("Assets\\game\\car_braked.png");

	// BAT - Load both left and right animations
	for (int i = 0; i < BAT_FRAMES; i++) {
		char pathL[128];
		char pathR[128];

		sprintf_s(pathL, "Assets\\game\\bat\\bat%d.png", i);
		sprintf_s(pathR, "Assets\\game\\bat\\bat_r%d.png", i);

		batImgL[i] = iLoadImage(pathL);
		batImgR[i] = iLoadImage(pathR);
	}

	// OBSTACLES
	for (int i = 0; i < OB_COUNT; i++) {
		char path[128];
		sprintf_s(path, "Assets//game//obstacles//obs%d.png", i);
		obsImg[i] = iLoadImage(path);
	}

	// NITRO
	nitro.loadImages();

	enemy.loadImages();

	health.loadImages();

	// Health pickup image
	healthPickupImg = iLoadImage("Assets\\hp\\addHealth.png");
}

// ==============================
//        HEALTH PICKUP FUNCTIONS
// ==============================
void initHealthPickups() {
	for (int i = 0; i < MAX_HEALTH_PICKUPS; i++) {
		healthPickups[i].active = false;
		healthPickups[i].lifetime = 0;
	}
}

void spawnHealthPickup(float x, float y) {
	// Find an inactive pickup slot
	for (int i = 0; i < MAX_HEALTH_PICKUPS; i++) {
		if (!healthPickups[i].active) {
			healthPickups[i].x = x;
			healthPickups[i].y = y;
			healthPickups[i].active = true;
			healthPickups[i].lifetime = 333; // 4 seconds at 12ms per tick (4000/12 ≈ 333)
			return;
		}
	}
}

void updateHealthPickups(float roadSpeed) {
	for (int i = 0; i < MAX_HEALTH_PICKUPS; i++) {
		if (healthPickups[i].active) {
			// Move pickup upward with road scrolling
			healthPickups[i].y -= roadSpeed;

			// Decrease lifetime
			healthPickups[i].lifetime--;

			// Remove if off screen or lifetime expired
			if (healthPickups[i].y + 50 < 0 || healthPickups[i].lifetime <= 0) {
				healthPickups[i].active = false;
			}
		}
	}
}

void drawHealthPickups() {
	for (int i = 0; i < MAX_HEALTH_PICKUPS; i++) {
		if (healthPickups[i].active && healthPickupImg != -1) {
			iShowImage((int)healthPickups[i].x, (int)healthPickups[i].y, 40, 40, healthPickupImg);
		}
	}
}

void checkHealthPickupCollision() {
	float carLeft = (float)carX;
	float carRight = (float)carX + CAR_W;
	float carTop = CAR_Y;
	float carBottom = CAR_Y + CAR_H;

	for (int i = 0; i < MAX_HEALTH_PICKUPS; i++) {
		if (healthPickups[i].active) {
			float pickupLeft = healthPickups[i].x;
			float pickupRight = healthPickups[i].x + 40;
			float pickupTop = healthPickups[i].y;
			float pickupBottom = healthPickups[i].y + 40;

			// Check collision
			if (carRight > pickupLeft && carLeft < pickupRight &&
				carBottom > pickupTop && carTop < pickupBottom) {

				// Collect health pickup
				healthPickups[i].active = false;

				// Restore 1 health (if not already at max)
				health.increaseHealth();

				// Play power-up sound
				playPowerUpSound();
			}
		}
	}
}

// ==============================
//             PAGES
// ==============================
void drawHome()
{
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);

	iShowImage(0, 0, SCREEN_W, SCREEN_H, menuBG);

	// buttons
	iShowImage(316, 240, 286, 81, btn.start.normal);
	iShowImage(316, 154, 346, 81, btn.highScores.normal);
	iShowImage(316, 68, 233, 81, btn.options.normal);
	iShowImage(553, 68, 81, 81, btn.gameExit.normal);

	// hovers
	if (inside(mouseX, mouseY, 316, 602, 240, 321)) {
		iShowImage(316, 240, 286, 81, btn.start.hover);
	}
	else if (inside(mouseX, mouseY, 316, 546, 154, 235)) {
		iShowImage(316, 154, 346, 81, btn.highScores.hover);
	}
	else if (inside(mouseX, mouseY, 316, 549, 68, 149)) {
		iShowImage(316, 68, 233, 81, btn.options.hover);
	}
	else if (inside(mouseX, mouseY, 553, 634, 68, 149)) {
		iShowImage(553, 68, 81, 81, btn.gameExit.hover);
	}
}

void drawStart()
{
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);
	iShowImage(0, 0, SCREEN_W, SCREEN_H, menuBG);

	// BUTTONS
	iShowImage(312, 240, 80, 80, btn.back.normal);
	iShowImage(402, 240, 231, 80, btn.easy.normal);
	iShowImage(402, 154, 231, 80, btn.medium.normal);
	iShowImage(402, 66, 231, 80, btn.hard.normal);

	// BUTTON_HOVERS
	if (inside(mouseX, mouseY, 312, 392, 240, 320)) {
		iShowImage(312, 240, 80, 80, btn.back.hover);
	}
	else if (inside(mouseX, mouseY, 402, 633, 240, 320)) {
		iShowImage(402, 240, 231, 80, btn.easy.hover);
	}
	else if (inside(mouseX, mouseY, 402, 633, 154, 234)) {
		iShowImage(402, 154, 231, 80, btn.medium.hover);
	}
	else if (inside(mouseX, mouseY, 402, 633, 66, 146)) {
		iShowImage(402, 66, 231, 80, btn.hard.hover);
	}
}

void drawHighScore()
{
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);
	iShowImage(0, 0, SCREEN_W, SCREEN_H, scoreBG);
	iShowImage(64, SCREEN_H - 80, 80, 80, btn.back.normal);

	if (inside(mouseX, mouseY, 64, 144, SCREEN_H - 80, SCREEN_H)) {
		iShowImage(64, SCREEN_H - 80, 80, 80, btn.back.hover);
	}

	highScores.draw(SCREEN_W, SCREEN_H);
}

void drawLevelComplete()
{
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);

	iSetColor(255, 215, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);

	iSetColor(0, 0, 0);
	iText(SCREEN_W / 2 - 120, SCREEN_H / 2 + 100, "LEVEL COMPLETE!", GLUT_BITMAP_TIMES_ROMAN_24);

	char diffText[50];
	const char* diffName = (difficulty == DIFF_EASY) ? "Easy" : (difficulty == DIFF_MEDIUM) ? "Medium" : "Hard";
	sprintf_s(diffText, "Difficulty: %s", diffName);
	iText(SCREEN_W / 2 - 80, SCREEN_H / 2 + 50, diffText, GLUT_BITMAP_HELVETICA_18);

	char distText[50];
	sprintf_s(distText, "Distance: %.1f km", distanceTraveled);
	iText(SCREEN_W / 2 - 80, SCREEN_H / 2, distText, GLUT_BITMAP_HELVETICA_18);

	char scoreText[50];
	sprintf_s(scoreText, "Score: %d", score.getScore());
	iText(SCREEN_W / 2 - 60, SCREEN_H / 2 - 50, scoreText, GLUT_BITMAP_HELVETICA_18);

	iSetColor(0, 0, 0);
	iText(SCREEN_W / 2 - 150, SCREEN_H / 2 - 120, "Click anywhere to continue...", GLUT_BITMAP_HELVETICA_18);

	int remainingSeconds = (LEVEL_COMPLETE_DURATION - levelCompleteTimer) / 83;
	char timerText[50];
	sprintf_s(timerText, "Auto-continue in: %d seconds", remainingSeconds);
	iText(SCREEN_W / 2 - 120, SCREEN_H / 2 - 180, timerText, GLUT_BITMAP_HELVETICA_18);
}

void initAudio()
{
	if (audioOpened) return;
	mciSendString("open \"Assets\\\\audio\\\\errand_day.mp3\" alias bgsong", NULL, 0, NULL);
	mciSendString("open \"Assets\\\\audio\\\\game_over.mp3\" alias gosong", NULL, 0, NULL);
	mciSendString("open \"Assets\\\\audio\\\\level_complete.wav\" alias lvlcomplete", NULL, 0, NULL);

	// Add new sound effects
	mciSendString("open \"Assets\\\\audio\\\\anas_laugh.mp3\" alias anas_laugh", NULL, 0, NULL);
	mciSendString("open \"Assets\\\\audio\\\\enemy_destroyed.mp3\" alias enemy_destroyed", NULL, 0, NULL);
	mciSendString("open \"Assets\\\\audio\\\\enemy_destroyed2.mp3\" alias enemy_destroyed2", NULL, 0, NULL);
	mciSendString("open \"Assets\\\\audio\\\\car_damage.mp3\" alias car_damage", NULL, 0, NULL);
	mciSendString("open \"Assets\\\\audio\\\\angry.mp3\" alias boss_damage", NULL, 0, NULL);
	mciSendString("open \"Assets\\\\audio\\\\powerup.mp3\" alias powerup", NULL, 0, NULL);

	audioOpened = true;
}

void playBackgroundMusic()
{
	if (!audioOpened) initAudio();
	mciSendString("stop bgsong", NULL, 0, NULL);
	mciSendString("seek bgsong to start", NULL, 0, NULL);
	mciSendString("play bgsong repeat", NULL, 0, NULL);
}

void stopBackgroundMusic()
{
	if (!audioOpened) return;
	mciSendString("stop bgsong", NULL, 0, NULL);
	mciSendString("seek bgsong to start", NULL, 0, NULL);
}

void playGameOverMusicOnce()
{
	if (!audioOpened) initAudio();
	stopBackgroundMusic();
	mciSendString("stop gosong", NULL, 0, NULL);
	mciSendString("seek gosong to start", NULL, 0, NULL);
	mciSendString("play gosong from 0", NULL, 0, NULL);
}

void playLevelCompleteSound()
{
	if (!audioOpened) initAudio();
	mciSendString("stop lvlcomplete", NULL, 0, NULL);
	mciSendString("seek lvlcomplete to start", NULL, 0, NULL);
	mciSendString("play lvlcomplete", NULL, 0, NULL);
}

void playCarDamageSound()
{
	if (!audioOpened) initAudio();
	int randomChance = rand() % 2; // 0 or 1 (50% chance)
	if (randomChance == 0) {
		mciSendString("stop car_damage", NULL, 0, NULL);
		mciSendString("seek car_damage to start", NULL, 0, NULL);
		// Set volume to maximum (1000 is typical max for mci)
		mciSendString("setaudio car_damage volume to 1000", NULL, 0, NULL);
		mciSendString("play car_damage", NULL, 0, NULL);
	}
}

void playBossDamageSound()
{
	if (!audioOpened) initAudio();
	int randomChance = rand() % 3; // 0, 1, or 2 (33% chance)
	if (randomChance == 0) {
		mciSendString("stop boss_damage", NULL, 0, NULL);
		mciSendString("seek boss_damage to start", NULL, 0, NULL);
		mciSendString("play boss_damage", NULL, 0, NULL);
	}
}

void playPowerUpSound()
{
	if (!audioOpened) initAudio();
	mciSendString("stop powerup", NULL, 0, NULL);
	mciSendString("seek powerup to start", NULL, 0, NULL);
	mciSendString("play powerup", NULL, 0, NULL);
}

void drawOptions()
{
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);
	iShowImage(0, 0, SCREEN_W, SCREEN_H, menuBG);

	iShowImage(268, 240, 80, 80, btn.back.normal);
	iShowImage(356, 240, 275, 80, btn.controls.normal);
	iShowImage(356, 152, 274, 80, btn.credits.normal);

	if (inside(mouseX, mouseY, 268, 348, 240, 320)) {
		iShowImage(268, 240, 80, 80, btn.back.hover);
	}
	else if (inside(mouseX, mouseY, 356, 631, 240, 320)) {
		iShowImage(356, 240, 275, 80, btn.controls.hover);
	}
	else if (inside(mouseX, mouseY, 356, 630, 152, 232)) {
		iShowImage(356, 152, 274, 80, btn.credits.hover);
	}
}

void drawControls()
{
	static int controlsBG = iLoadImage("Assets\\options\\controls\\controls_bg.png");
	iShowImage(0, 0, SCREEN_W, SCREEN_H, controlsBG);

	static int goBack = iLoadImage("Assets\\options\\controls\\go_back.png");
	static int controlsPreview = iLoadImage("Assets\\options\\controls\\controls_preview.png");

	iShowImage(64, 652, 80, 80, btn.back.normal);
	iShowImage(156, 652, 221, 80, goBack);
	iShowImage(64, 96, 471, 489, controlsPreview);

	if (inside(mouseX, mouseY, 64, 144, 652, 732)) {
		iShowImage(64, 652, 80, 80, btn.back.hover);
	}
}

void drawCredits()
{
	static int creditsBG = iLoadImage("Assets\\options\\credits\\credits_bg.png");
	iShowImage(0, 0, SCREEN_W, SCREEN_H, creditsBG);

	iShowImage(74, 532, 80, 80, btn.back.normal);
	if (inside(mouseX, mouseY, 74, 154, 532, 612)) {
		iShowImage(74, 532, 80, 80, btn.back.hover);
	}
}

// ==============================
//          BUTTON HANDLERS
// ==============================
void startButtonClickHandler()   { page = PAGE_START; }
void backButtonClickHandler()    { page = PAGE_HOME; }
void optionsButtonClickHandler() { page = PAGE_OPTIONS; }
void controlsButtonClickHandler(){ page = PAGE_CONTROLS; }
void creditsButtonClickHandler() { page = PAGE_CREDITS; }
void highScoreButtonClickHandler() { page = PAGE_HIGHSCORE; }

void easyButtonClickHandler()
{
	setDifficulty(DIFF_EASY);
	page = PAGE_GAME;
	initGame();
}

void mediumButtonClickHandler()
{
	setDifficulty(DIFF_MEDIUM);
	page = PAGE_GAME;
	initGame();
}

void hardButtonClickHandler()
{
	setDifficulty(DIFF_HARD);
	page = PAGE_GAME;
	initGame();
}

// ==============================
//           DIFFICULTY
// ==============================
void setDifficulty(Difficulty d)
{
	difficulty = d;

	if (difficulty == DIFF_EASY) {
		cfg.maxSpeed = 16.0;
		cfg.accelRate = 0.13;
		cfg.brakeRate = 0.42;
		cfg.frictionRate = 0.04;
		cfg.steerMin = 3.8;
		cfg.steerMax = 9.5;

		cfg.damageBlinkTicks = 45;
		cfg.crashSpeedReset = 8.0;

		cfg.obstacles.maxActive = 2;
		cfg.obstacles.spawnChancePercent = 10;
		cfg.obstacles.baseDelayNoObs = 185;  cfg.obstacles.rangeDelayNoObs = 300;
		cfg.obstacles.baseDelayWithObs = 260; cfg.obstacles.rangeDelayWithObs = 340;

		enemy.maxEnemy = 3;
		enemy.spawnMinTicks = 580;
		enemy.spawnRangeTicks = 280;
	}
	else if (difficulty == DIFF_MEDIUM) {
		cfg.maxSpeed = 18.0;
		cfg.accelRate = 0.15;
		cfg.brakeRate = 0.40;
		cfg.frictionRate = 0.04;
		cfg.steerMin = 3.5;
		cfg.steerMax = 10.0;

		cfg.damageBlinkTicks = 40;
		cfg.crashSpeedReset = 9.0;

		cfg.obstacles.maxActive = 3;
		cfg.obstacles.spawnChancePercent = 25;
		cfg.obstacles.baseDelayNoObs = 145;  cfg.obstacles.rangeDelayNoObs = 260;
		cfg.obstacles.baseDelayWithObs = 210; cfg.obstacles.rangeDelayWithObs = 290;

		enemy.maxEnemy = 4;
		enemy.spawnMinTicks = 520;
		enemy.spawnRangeTicks = 320;
	}
	else {
		cfg.maxSpeed = 20.0;
		cfg.accelRate = 0.17;
		cfg.brakeRate = 0.38;
		cfg.frictionRate = 0.035;
		cfg.steerMin = 3.2;
		cfg.steerMax = 10.5;

		cfg.damageBlinkTicks = 35;
		cfg.crashSpeedReset = 10.0;

		cfg.obstacles.maxActive = 4;
		cfg.obstacles.spawnChancePercent = 35;
		cfg.obstacles.baseDelayNoObs = 140;  cfg.obstacles.rangeDelayNoObs = 240;
		cfg.obstacles.baseDelayWithObs = 185; cfg.obstacles.rangeDelayWithObs = 170;

		enemy.maxEnemy = 6;
		enemy.spawnMinTicks = 450;
		enemy.spawnRangeTicks = 280;
	}

	setObstacleSpawnConfig(cfg.obstacles);
	nitro.setAllowed(difficulty != DIFF_EASY);
}

void goToGameOver()
{
	page = PAGE_GAMEOVER;
	if (!gameOverSoundPlayed) {
		playGameOverMusicOnce();
		gameOverSoundPlayed = true;
	}
	finalScore = score.getScore();
	finalDifficulty = difficulty;

	waitingForName = true;
	memset(playerName, 0, sizeof(playerName));
	nameInputIndex = 0;
}

// LANE
void initLanes()
{
	double usable = (ROAD_RIGHT_X - ROAD_LEFT_X - CAR_W);
	double step = usable / 3.0;
	for (int i = 0; i < 4; i++) carLaneX[i] = (int)(ROAD_LEFT_X + i * step);
}

// ==============================
//             GAME
// ==============================
void initGame()
{
	double roadCenter = (ROAD_LEFT_X + ROAD_RIGHT_X) / 2.0;
	carX = roadCenter - (CAR_W / 2.0);

	roadSpeed = MIN_SPEED;
	roadOffsetY = 0;

	initLanes();

	prevSpace = false;
	prevN = false;
	prevB = false;

	initObstacleSystem(ROAD_LEFT_X, ROAD_RIGHT_X, SCREEN_H);
	setObstacleSpawnConfig(cfg.obstacles);
	resetObstacles();

	damageBlink = false;
	blinkTicks = 0;

	batPlaying = false;
	batFrame = 0;
	currentBatSide = BAT_SWING_LEFT;

	nitro.reset();
	enemy.reset();
	health.reset();
	score.reset();
	resetDistance();
	resetLevelCompletion();
	initHealthPickups();  // Initialize health pickups

	// Initialize boss system for current difficulty
	enemy.initBossSystem(difficulty);

	gamePaused = false;
	prevP = false;
	rageRequirementFailed = false;
	gameTickCounter = 0;  // Reset game tick counter for boss timing
}

void drawGame()
{
	bool brakeHeld = (isKeyPressed('s') != 0) || (isSpecialKeyPressed(GLUT_KEY_DOWN) != 0);

	bool drawCarNow = true;
	if (damageBlink) {
		drawCarNow = ((blinkTicks / 6) % 2 == 0);
	}

	iShowImage(0, (int)roadOffsetY, SCREEN_W, SCREEN_H, roadImgA);
	iShowImage(0, (int)roadOffsetY + SCREEN_H, SCREEN_W, SCREEN_H, roadImgB);

	if (drawCarNow) {
		if (nitro.isActive()) {
			iShowImage((int)carX, CAR_Y + nitro.carDrawYOffset(), CAR_W, 204, nitro.carNitroImg);
		}
		else {
			if (batPlaying) {
				if (currentBatSide == BAT_SWING_LEFT) {
					// Left side - original drawing position
					iShowImage((int)carX + BAT_OFFSET_X, CAR_Y + BAT_OFFSET_Y,
						CAR_W + BAT_EXTRA_W, CAR_H,
						batImgL[batFrame]);
				}
				else {
					// Right side - draw at different position (no offset, starts at carX)
					iShowImage((int)carX + BAT_OFFSET_X_R, CAR_Y + BAT_OFFSET_Y_R,
						CAR_W + BAT_EXTRA_W, CAR_H,
						batImgR[batFrame]);
				}
			}
			else {
				iShowImage((int)carX, CAR_Y, CAR_W, CAR_H,
					brakeHeld ? carBrakedImg : carImg);
			}
		}
	}

	enemy.draw();

	for (int i = 0; i < OB_CAPACITY; i++) {
		if (!gObs[i].active) continue;
		iShowImage((int)gObs[i].x, (int)gObs[i].y,
			gObs[i].width, gObs[i].height,
			obsImg[gObs[i].imgIndex]);
	}

	nitro.drawPickups();
	nitro.drawMeter();

	// Draw health pickups
	drawHealthPickups();

	health.draw(SCREEN_W, SCREEN_H);
	score.draw(SCREEN_W, SCREEN_H);
	drawDistance();

	char goalText[50];
	double goal = getDistanceGoal();
	sprintf_s(goalText, "Goal: %.0f km", goal);
	iSetColor(255, 255, 0);
	iText(SCREEN_W - 680, SCREEN_H - 150, goalText, GLUT_BITMAP_HELVETICA_18);

	// Draw rage requirement - show rage left
	int rageReq = getRageRequirement();
	int currentScore = score.getScore();
	int rageLeft = rageReq - currentScore;
	if (rageLeft < 0) rageLeft = 0; // Ensure it doesn't go negative

	char rageText[50];
	sprintf_s(rageText, "Rage left: %d", rageLeft);
	iSetColor(255, 200, 0);
	iText(SCREEN_W - 680, SCREEN_H - 175, rageText, GLUT_BITMAP_HELVETICA_18);

	// Draw rage bar - shows remaining rage (drains as you score)
	int barWidth = 200;
	int barHeight = 8;
	int barX = SCREEN_W - 680;
	int barY = SCREEN_H - 190;

	// Calculate progress based on RAGE LEFT / TOTAL RAGE
	double progress = (double)rageLeft / (double)rageReq;
	if (progress > 1.0) progress = 1.0;
	if (progress < 0.0) progress = 0.0;

	// Background bar (dark gray)
	iSetColor(80, 80, 80);
	iFilledRectangle(barX, barY, barWidth, barHeight);

	// Foreground bar (orange showing remaining rage)
	iSetColor(255, 100, 0);
	iFilledRectangle(barX, barY, (int)(barWidth * progress), barHeight);

	// Draw boss warning if needed
	enemy.drawBossWarning(SCREEN_W, SCREEN_H);

	if (gamePaused) {
		iSetColor(0, 0, 0);
		iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);
		iSetColor(255, 255, 255);
		iText(SCREEN_W / 2 - 50, SCREEN_H / 2 + 20, "PAUSED", GLUT_BITMAP_HELVETICA_18);
		iText(SCREEN_W / 2 - 150, SCREEN_H / 2 - 10, "Press Esc to Resume, Q to Quit", GLUT_BITMAP_HELVETICA_18);
		iSetColor(255, 255, 0);
		iText(SCREEN_W - 650, SCREEN_H - 700, "Help Anas and his cousin to save their neighbourhood from the", GLUT_BITMAP_HELVETICA_18);
		iText(SCREEN_W - 450, SCREEN_H - 725, "AI vehicles invasion!", GLUT_BITMAP_HELVETICA_18);
	}

	iSetColor(255, 255, 255);
	char txt[160];
	const char* diffName = (difficulty == DIFF_EASY) ? "Easy" : (difficulty == DIFF_MEDIUM) ? "Medium" : "Hard";
	//sprintf_s(txt, "Mode: %s   Speed: %.1f   carX: %.0f", diffName, roadSpeed, carX);
	sprintf_s(txt, "Mode: %s   Speed: %.1f ", diffName, roadSpeed);
	iText(10, SCREEN_H - 25, txt, GLUT_BITMAP_HELVETICA_18);
}

// ==============================
//      HANDLE BAT SWING - UNCHANGED FROM YOUR WORKING VERSION
// ==============================
void handleBatSwing() {
	if (!batPlaying) return;

	int batX = (int)carX + BAT_OFFSET_X + BAT_HITBOX_OFFSET_X;
	int batY = CAR_Y + BAT_OFFSET_Y;
	int batW = CAR_W + BAT_EXTRA_W + BAT_HITBOX_EXTRA_W;
	int batH = CAR_H + BAT_HITBOX_EXTRA_H;

	static bool hitProcessed = false;

	if (batFrame == 0) {
		hitProcessed = false;
	}

	if (!hitProcessed && batFrame >= 3 && batFrame <= 7) {
		int hitIndex = enemy.checkBatHit(batX, batY, batW, batH);
		if (hitIndex >= 0) {
			// Store whether enemy was alive before this hit
			bool wasAlive = (enemy.e[hitIndex].health > 0);

			// Reduce health
			enemy.reduceHealth(hitIndex);

			// Check if enemy died from this hit
			if (enemy.e[hitIndex].health <= 0 && wasAlive) {
				// Enemy or boss defeated - add points
				int points = enemy.getDefeatPoints(hitIndex);
				score.addPoints(points);
				// Rage reduction is automatic through score system

				// 33% chance to spawn health pickup (only from normal enemies, not boss)
				if (!enemy.e[hitIndex].isBoss) {
					int randomChance = rand() % 3; // 0, 1 or 2
					if (randomChance == 0) { // 33% chance
						float pickupX = enemy.e[hitIndex].x;
						float pickupY = enemy.e[hitIndex].y + 320; // Spawn 320 pixels AHEAD (higher Y)
						spawnHealthPickup(pickupX, pickupY);
					}
				}
			}
			// REMOVED: No points added for normal hits (when enemy doesn't die)

			hitProcessed = true;

			if (hitIndex >= 0 && hitIndex < 4 && enemy.e[hitIndex].active) {
				float pushDir = (enemy.e[hitIndex].x < carX) ? -BAT_KNOCKBACK_FORCE : BAT_KNOCKBACK_FORCE;
				enemy.e[hitIndex].x += pushDir;

				int enemyW = enemy.e[hitIndex].isBoss ? 140 : 116;
				int enemyH = enemy.e[hitIndex].isBoss ? 190 : 169;

				if (enemy.e[hitIndex].x < ROAD_LEFT_X)
					enemy.e[hitIndex].x = (float)ROAD_LEFT_X;
				if (enemy.e[hitIndex].x > ROAD_RIGHT_X - enemyW)
					enemy.e[hitIndex].x = (float)(ROAD_RIGHT_X - enemyW);

				enemy.e[hitIndex].y -= 15.0f;
				if (enemy.e[hitIndex].y < CAR_Y - 100) {
					enemy.e[hitIndex].y = (float)(CAR_Y - 100);
				}
			}
		}
	}
}

// GAME REFRESH
void updateGame()
{
	// ========== IMPROVED NAME INPUT HANDLING ==========
	if (page == PAGE_GAMEOVER && waitingForName) {
		// Handle ENTER key to save name
		if (isKeyPressed(13) != 0) {
			if (strlen(playerName) > 0) {
				highScores.addScore(playerName, finalScore, finalDifficulty);
			}
			waitingForName = false;
			page = PAGE_HOME;
			gameOverSoundPlayed = false;
			playBackgroundMusic();
			return;
		}

		// Handle ESC key to skip
		if (isKeyPressed(27) != 0) {
			waitingForName = false;
			page = PAGE_HOME;
			gameOverSoundPlayed = false;
			playBackgroundMusic();
			return;
		}

		// Handle BACKSPACE
		static bool prevBackspace = false;
		bool nowBackspace = (isKeyPressed(8) != 0);
		if (nowBackspace && !prevBackspace && nameInputIndex > 0) {
			nameInputIndex--;
			playerName[nameInputIndex] = '\0';
		}
		prevBackspace = nowBackspace;

		// Track which keys are currently pressed to avoid duplicates
		static bool keyProcessed[256] = { false };

		// Process each possible character
		for (int key = 32; key <= 126; key++) { // Printable ASCII range
			if (isKeyPressed(key) != 0) {
				if (!keyProcessed[key] && nameInputIndex < 49) {
					// Valid characters: letters, numbers, space, underscore
					if ((key >= 'A' && key <= 'Z') ||
						(key >= 'a' && key <= 'z') ||
						(key >= '0' && key <= '9') ||
						key == ' ' || key == '_') {

						playerName[nameInputIndex] = (char)key;
						nameInputIndex++;
						playerName[nameInputIndex] = '\0';
					}
					keyProcessed[key] = true;
				}
			}
			else {
				keyProcessed[key] = false;
			}
		}

		return;
	}

	// ========== LEVEL COMPLETE PAGE HANDLING ==========
	if (page == PAGE_LEVEL_COMPLETE) {
		levelCompleteTimer++;

		if (levelCompleteTimer >= LEVEL_COMPLETE_DURATION) {
			waitingForName = true;
			memset(playerName, 0, sizeof(playerName));
			nameInputIndex = 0;
			isLevelCompletion = true;
			page = PAGE_GAMEOVER;
			playBackgroundMusic(); // Restart music when leaving level complete
			return;
		}
		return;
	}

	// ========== UPDATE BOSS SYSTEM ==========
	gameTickCounter++;
	enemy.updateBossSystem(gameTickCounter);

	// ========== REGULAR GAME LOGIC ==========
	bool nowEsc = (isKeyPressed(27) != 0);

	if (nowEsc && !prevP) {
		gamePaused = !gamePaused;
	}
	prevP = nowEsc;

	if (gamePaused) {
		if ((isKeyPressed('q') != 0) || (isKeyPressed('Q') != 0)) {
			page = PAGE_HOME;
			gameOverSoundPlayed = false;
			playBackgroundMusic();
		}
		return;
	}

	setObstacleSpawnEnabled(!nitro.isActive());
	bool nitroOn = nitro.isActive();

	carX = enemy.update(
		(float)roadSpeed,
		(float)carX, (float)CAR_Y, (float)CAR_W, (float)CAR_H,
		nitroOn,
		(float)ROAD_LEFT_X, (float)ROAD_RIGHT_X
		);

	for (int i = 0; i < 4; i++) {
		if (enemy.e[i].active && !enemy.e[i].isBoss) {
			if (enemy.e[i].x < ROAD_LEFT_X) enemy.e[i].x = ROAD_LEFT_X;
			if (enemy.e[i].x > ROAD_RIGHT_X - EnemySystem::ENEMY_W)
				enemy.e[i].x = ROAD_RIGHT_X - EnemySystem::ENEMY_W;
		}
		// Boss uses its own dimensions, handled in its own update logic
	}

	if (page != PAGE_GAME) {
		prevSpace = false;
		prevN = false;
		return;
	}

	// SPACE key triggers bat swing (edge-triggered)
	bool nowSpace = (isKeyPressed(' ') != 0);
	if (nowSpace && !prevSpace) {
		currentBatSide = chooseBatSwingSide();  // Choose which animation to show
		batPlaying = true;
		batFrame = 0;
	}
	prevSpace = nowSpace;

	// N key triggers nitro
	bool nowN = (isKeyPressed('n') != 0) || (isKeyPressed('N') != 0);
	if (nowN && !prevN) {
		nitro.tryActivate();
	}
	prevN = nowN;

	// Steering depends on speed
	bool leftHeld = (isKeyPressed('a') != 0) || (isSpecialKeyPressed(GLUT_KEY_LEFT) != 0);
	bool rightHeld = (isKeyPressed('d') != 0) || (isSpecialKeyPressed(GLUT_KEY_RIGHT) != 0);

	double t = (roadSpeed - MIN_SPEED) / (cfg.maxSpeed - MIN_SPEED);
	t = clampDouble(t, 0.0, 1.0);
	double steerThisFrame = cfg.steerMin + t * (cfg.steerMax - cfg.steerMin);

	if (leftHeld)  carX -= steerThisFrame;
	if (rightHeld) carX += steerThisFrame;

	bool accelerateHeld = (isKeyPressed('w') != 0) || (isSpecialKeyPressed(GLUT_KEY_UP) != 0);
	bool brakeHeld = (isKeyPressed('s') != 0) || (isSpecialKeyPressed(GLUT_KEY_DOWN) != 0);

	if (accelerateHeld) {
		roadSpeed += cfg.accelRate;
		if (roadSpeed > cfg.maxSpeed) roadSpeed = cfg.maxSpeed;
	}

	if (brakeHeld) {
		roadSpeed -= cfg.brakeRate;
		if (roadSpeed < MIN_SPEED) roadSpeed = MIN_SPEED;
		if (roadSpeed > cfg.maxSpeed) roadSpeed = cfg.maxSpeed;
	}

	if (!accelerateHeld && !brakeHeld) {
		roadSpeed -= cfg.frictionRate;
		if (roadSpeed < MIN_SPEED) roadSpeed = MIN_SPEED;
	}

	roadOffsetY -= roadSpeed;
	if (roadOffsetY <= -SCREEN_H)
		roadOffsetY += SCREEN_H;

	carX = clampDouble(carX, ROAD_LEFT_X, ROAD_RIGHT_X - CAR_W);

	float drawCarY = (float)CAR_Y + (float)nitro.carDrawYOffset();
	float drawCarH = (float)nitro.carDrawH(CAR_H);

	nitro.update(
		(float)roadSpeed,
		carLaneX,
		(float)carX, drawCarY, (float)CAR_W, drawCarH,
		cfg.maxSpeed,
		roadSpeed
		);

	updateDistance(nitro.isActive());

	// Update health pickups
	updateHealthPickups((float)roadSpeed);

	// Check collision with health pickups
	checkHealthPickupCollision();

	checkLevelCompletion();

	if (levelCompleted) {
		return;
	}

	updateObstacles((float)roadSpeed);

	if (!damageBlink && checkObstacleCollision((float)carX, (float)CAR_Y, (float)CAR_W, (float)CAR_H)) {
		damageBlink = true;
		blinkTicks = cfg.damageBlinkTicks;
		health.takeHit();

		// Play car damage sound with 50% chance
		playCarDamageSound();

		if (health.isGameOver()) {
			goToGameOver();
			return;
		}

		resetObstacles();
		roadSpeed = cfg.crashSpeedReset;
		if (roadSpeed < MIN_SPEED) roadSpeed = MIN_SPEED;
		if (roadSpeed > cfg.maxSpeed) roadSpeed = cfg.maxSpeed;
	}

	// Enemy car collision
	if (!damageBlink && enemyCollisionCooldown == 0) {
		for (int i = 0; i < 4; i++) {
			if (enemy.e[i].active) {
				int enemyW = enemy.e[i].isBoss ? 140 : 116;
				int enemyH = enemy.e[i].isBoss ? 190 : 169;

				if (carX < enemy.e[i].x + enemyW &&
					carX + CAR_W > enemy.e[i].x &&
					CAR_Y < enemy.e[i].y + enemyH &&
					CAR_Y + CAR_H > enemy.e[i].y) {

					int damageAmount = enemy.getCollisionDamage(i);
					damageBlink = true;
					blinkTicks = cfg.damageBlinkTicks;

					// Apply damage multiple times if needed (boss does 2 damage)
					for (int d = 0; d < damageAmount; d++) {
						health.takeHit();
					}

					// Play appropriate damage sounds
					if (enemy.e[i].isBoss) {
						// Boss damage sound with 33% chance
						playBossDamageSound();
					}
					playCarDamageSound();

					if (health.isGameOver()) {
						goToGameOver();
						return;
					}

					resetObstacles();
					roadSpeed = cfg.crashSpeedReset;
					if (roadSpeed < MIN_SPEED) roadSpeed = MIN_SPEED;
					if (roadSpeed > cfg.maxSpeed) roadSpeed = cfg.maxSpeed;

					int pushDir = (carX < enemy.e[i].x) ? -1 : 1;
					enemy.e[i].x += pushDir * 15.0f;
					if (enemy.e[i].x < ROAD_LEFT_X) enemy.e[i].x = ROAD_LEFT_X;
					if (enemy.e[i].x > ROAD_RIGHT_X - enemyW)
						enemy.e[i].x = ROAD_RIGHT_X - enemyW;

					enemyCollisionCooldown = 30;
					break;
				}
			}
		}
	}

	if (damageBlink) {
		blinkTicks--;
		if (blinkTicks <= 0) {
			damageBlink = false;
			blinkTicks = 0;
		}
	}

	if (enemyCollisionCooldown > 0) {
		enemyCollisionCooldown--;
	}

	updateBatAnimation();
	handleBatSwing();
}

void updateBatAnimation()
{
	static int batDelayCounter = 0;
	const int batFrameDelay = 2;

	if (!batPlaying) return;

	batDelayCounter++;

	if (batDelayCounter >= batFrameDelay) {
		batFrame++;
		batDelayCounter = 0;
	}

	if (batFrame >= BAT_FRAMES) {
		batPlaying = false;
		batFrame = 0;
		batDelayCounter = 0;
	}
}

void drawGameOver()
{
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);

	if (waitingForName) {
		iSetColor(255, 255, 255);

		if (rageRequirementFailed) {
			iText(SCREEN_W / 2 - 120, SCREEN_H / 2 + 80, "You still have rage left!", GLUT_BITMAP_HELVETICA_18);
		}
		else if (!isLevelCompletion) {
			iText(SCREEN_W / 2 - 100, SCREEN_H / 2 + 80, "GAME OVER!", GLUT_BITMAP_HELVETICA_18);
		}
		else {
			iText(SCREEN_W / 2 - 100, SCREEN_H / 2 + 80, "LEVEL COMPLETED!", GLUT_BITMAP_HELVETICA_18);
		}

		char scoreText[100];
		sprintf_s(scoreText, "Your Score: %d", finalScore);
		iText(SCREEN_W / 2 - 100, SCREEN_H / 2 + 40, scoreText, GLUT_BITMAP_HELVETICA_18);

		char distText[100];
		sprintf_s(distText, "Distance: %.1f km", distanceTraveled);
		iText(SCREEN_W / 2 - 100, SCREEN_H / 2 + 20, distText, GLUT_BITMAP_HELVETICA_18);

		iText(SCREEN_W / 2 - 100, SCREEN_H / 2, "Enter your name:", GLUT_BITMAP_HELVETICA_18);

		char nameDisplay[60];
		sprintf_s(nameDisplay, "> %s_", playerName);
		iText(SCREEN_W / 2 - 100, SCREEN_H / 2 - 40, nameDisplay, GLUT_BITMAP_HELVETICA_18);

		iText(SCREEN_W / 2 - 150, SCREEN_H / 2 - 100, "Press ENTER to save, ESC to skip", GLUT_BITMAP_HELVETICA_18);
	}
	else {
		iSetColor(255, 255, 255);

		if (rageRequirementFailed) {
			iText(SCREEN_W / 2 - 120, SCREEN_H / 2 + 30, "You still have rage left!", GLUT_BITMAP_HELVETICA_18);
		}
		else if (!isLevelCompletion) {
			iText(SCREEN_W / 2 - 70, SCREEN_H / 2 + 30, "GAME OVER", GLUT_BITMAP_HELVETICA_18);
		}
		else {
			iText(SCREEN_W / 2 - 100, SCREEN_H / 2 + 30, "LEVEL COMPLETED!", GLUT_BITMAP_HELVETICA_18);
		}

		char scoreText[50];
		sprintf_s(scoreText, "Score: %d", finalScore);
		iText(SCREEN_W / 2 - 50, SCREEN_H / 2, scoreText, GLUT_BITMAP_HELVETICA_18);

		char distText[50];
		sprintf_s(distText, "Distance: %.1f km", distanceTraveled);
		iText(SCREEN_W / 2 - 50, SCREEN_H / 2 - 25, distText, GLUT_BITMAP_HELVETICA_18);

		iText(SCREEN_W / 2 - 170, SCREEN_H / 2 - 50, "Click to return to Home", GLUT_BITMAP_HELVETICA_18);
	}
}

// ==============================
//              MAIN
// ==============================
int main()
{
	iInitialize(SCREEN_W, SCREEN_H, "Errand Day");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	initBackground();
	initImages();
	initAudio();
	playBackgroundMusic();

	setDifficulty(DIFF_EASY);

	iSetTimer(12, updateGame);

	iStart();
	return 0;
}
