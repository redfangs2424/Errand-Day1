#include "iGraphics.h"
#include "Button.hpp"
#include "Obstacle.hpp"
#include "HealthSystem.hpp"
#include "Constants.hpp"
#include "Car.hpp"
#include "GameState.hpp"
#include "MenuButtons.hpp"
#include "Bat.hpp"
#include "Enemy.hpp"
#include <cstdlib>     

// Resource variables (couldn't be moved to headers due to iGraphics dependencies)
int roadImgA;
int roadImgB;
int obsImg[OB_COUNT];

// Conditions
bool prevO = false;

// Pause variables
bool isPaused = false;
bool prevEscState = false; // To detect ESC key press (not hold)

// Function declarations
void drawHome();
void drawStart();
void drawOptions();
void drawControls();
void drawCredits();
void drawEasyGame();
void drawGameOver();
void drawPauseScreen();
void initGame();

//=============================================================================
// Fixed Update (called by iGraphics for continuous key press handling)
//=============================================================================

void fixedUpdate()
{
	// Handle continuous key presses for smooth movement
	// This function is called automatically by iGraphics

	if (isKeyPressed('w') || isSpecialKeyPressed(GLUT_KEY_UP))
	{
		// Accelerate - handled in updateGame
	}
	if (isKeyPressed('a') || isSpecialKeyPressed(GLUT_KEY_LEFT))
	{
		// Turn left - handled in updateGame
	}
	if (isKeyPressed('s') || isSpecialKeyPressed(GLUT_KEY_DOWN))
	{
		// Brake - handled in updateGame
	}
	if (isKeyPressed('d') || isSpecialKeyPressed(GLUT_KEY_RIGHT))
	{
		// Turn right - handled in updateGame
	}

	// Space bar for sound
	if (isKeyPressed(' ')) {
		// Playing the audio once
		mciSendString("play ggsong from 0", NULL, 0, NULL);
	}

	// Check for ESC key press (27 is ASCII for Escape)
	bool currentEscState = isKeyPressed(27); // Check if ESC is pressed

	// Toggle pause only when ESC is pressed (not held)
	if (easyGame && !isGameOver() && currentEscState && !prevEscState) {
		isPaused = !isPaused; // Toggle pause state
		printf("Game %s\n", isPaused ? "Paused" : "Resumed");
	}

	prevEscState = currentEscState; // Remember current state for next frame
}

//=============================================================================
// INITIALIZATION
//=============================================================================

void initImages() {
	// Menu images
	initMenuImages();

	// Game images
	roadImgA = iLoadImage("Assets\\game\\road.png");
	roadImgB = iLoadImage("Assets\\game\\road.png");
	carImg = iLoadImage("Assets\\game\\car.png");
	carBrakedImg = iLoadImage("Assets\\game\\car_braked.png");

	// Health images
	loadHealthImages();

	// Bat images
	loadBatImages();

	// Enemy image
	loadEnemyImage();

	// Obstacle images
	for (int i = 0; i < OB_COUNT; i++) {
		char path[128];
		sprintf_s(path, "Assets//game//obstacles//obs%d.png", i);
		obsImg[i] = iLoadImage(path);
	}
}

void initGame() {
	initCar();
	initObstacleSystem(ROAD_LEFT_X, ROAD_RIGHT_X, SCREEN_H);
	resetObstacles();
	resetCar();
	resetHealth();
	resetBat();
	resetEnemy();
	roadSpeed = MIN_SPEED;
	roadOffsetY = 0;
	isPaused = false; // Reset pause state when starting new game
	prevEscState = false;
}



//=============================================================================
// DRAW FUNCTIONS
//=============================================================================

void drawHome() {
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);
	iShowImage(0, 0, SCREEN_W, SCREEN_H, menuBG);

	// Buttons
	iShowImage(316, 240, 286, 81, btn.start.normal);
	iShowImage(316, 154, 346, 81, btn.highScores.normal);
	iShowImage(316, 68, 233, 81, btn.options.normal);
	iShowImage(553, 68, 81, 81, btn.gameExit.normal);

	// Hovers
	if (isMouseInside(mouseX, mouseY, 316, 602, 240, 321))
		iShowImage(316, 240, 286, 81, btn.start.hover);
	else if (isMouseInside(mouseX, mouseY, 316, 546, 154, 235))
		iShowImage(316, 154, 346, 81, btn.highScores.hover);
	else if (isMouseInside(mouseX, mouseY, 316, 549, 68, 149))
		iShowImage(316, 68, 233, 81, btn.options.hover);
	else if (isMouseInside(mouseX, mouseY, 553, 634, 68, 149))
		iShowImage(553, 68, 81, 81, btn.gameExit.hover);
}

void drawStart() {
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);
	iShowImage(0, 0, SCREEN_W, SCREEN_H, menuBG);

	// Buttons
	iShowImage(312, 240, 80, 80, btn.back.normal);
	iShowImage(402, 240, 231, 80, btn.easy.normal);
	iShowImage(402, 154, 231, 80, btn.medium.normal);
	iShowImage(402, 66, 231, 80, btn.hard.normal);

	// Hovers
	if (isMouseInside(mouseX, mouseY, 312, 392, 240, 320))
		iShowImage(312, 240, 80, 80, btn.back.hover);
	else if (isMouseInside(mouseX, mouseY, 402, 633, 240, 320))
		iShowImage(402, 240, 231, 80, btn.easy.hover);
	else if (isMouseInside(mouseX, mouseY, 402, 633, 154, 234))
		iShowImage(402, 154, 231, 80, btn.medium.hover);
	else if (isMouseInside(mouseX, mouseY, 402, 633, 38, 118))
		iShowImage(402, 66, 231, 80, btn.hard.hover);
}

void drawOptions() {
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);
	iShowImage(0, 0, SCREEN_W, SCREEN_H, menuBG);

	// Buttons
	iShowImage(268, 240, 80, 80, btn.back.normal);
	iShowImage(356, 240, 275, 80, btn.controls.normal);
	iShowImage(356, 152, 274, 80, btn.credits.normal);

	// Hovers
	if (isMouseInside(mouseX, mouseY, 268, 348, 240, 320))
		iShowImage(268, 240, 80, 80, btn.back.hover);
	else if (isMouseInside(mouseX, mouseY, 356, 631, 240, 320))
		iShowImage(356, 240, 275, 80, btn.controls.hover);
	else if (isMouseInside(mouseX, mouseY, 356, 630, 152, 232))
		iShowImage(356, 152, 274, 80, btn.credits.hover);
}

void drawControls() {
	static int controlsBG = iLoadImage("Assets\\options\\controls\\controls_bg.png");
	static int goBack = iLoadImage("Assets\\options\\controls\\go_back.png");
	static int controlsPreview = iLoadImage("Assets\\options\\controls\\controls_preview.png");

	iShowImage(0, 0, SCREEN_W, SCREEN_H, controlsBG);
	iShowImage(64, 652, 80, 80, btn.back.normal);
	iShowImage(156, 652, 221, 80, goBack);
	iShowImage(64, 96, 471, 489, controlsPreview);

	if (isMouseInside(mouseX, mouseY, 64, 144, 652, 732))
		iShowImage(64, 652, 80, 80, btn.back.hover);
}

void drawCredits() {
	static int creditsBG = iLoadImage("Assets\\options\\credits\\credits_bg.png");
	iShowImage(0, 0, SCREEN_W, SCREEN_H, creditsBG);
	iShowImage(74, 532, 80, 80, btn.back.normal);

	if (isMouseInside(mouseX, mouseY, 74, 154, 532, 612))
		iShowImage(74, 532, 80, 80, btn.back.hover);
}


void drawGameOver() {
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);
	iSetColor(255, 0, 0);
	iText(SCREEN_W / 2 - 100, SCREEN_H / 2, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
	iSetColor(255, 255, 255);
	iText(SCREEN_W / 2 - 150, SCREEN_H / 2 - 50, "Click anywhere to return to menu", GLUT_BITMAP_HELVETICA_18);
}

void drawPauseScreen() {
	// Draw a semi-transparent overlay using a pattern
	iSetColor(0, 0, 0);
	// Draw a grid pattern to simulate transparency
	for (int i = 0; i < SCREEN_W; i += 4) {
		for (int j = 0; j < SCREEN_H; j += 4) {
			iFilledRectangle(i, j, 2, 2);
		}
	}

	// Draw pause text
	iSetColor(255, 255, 255);
	iText(SCREEN_W / 2 - 80, SCREEN_H / 2 + 50, "GAME PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
	iText(SCREEN_W / 2 - 150, SCREEN_H / 2, "Click anywhere to resume", GLUT_BITMAP_HELVETICA_18);
	iText(SCREEN_W / 2 - 120, SCREEN_H / 2 - 30, "Press ESC to pause/unpause", GLUT_BITMAP_HELVETICA_18);
}

void drawEasyGame() {
	if (isGameOver()) return;

	bool brakeHeld = isKeyPressed('s') || isSpecialKeyPressed(GLUT_KEY_DOWN);
	bool drawCarNow = shouldDrawCar();

	// Road
	iShowImage(0, (int)roadOffsetY, SCREEN_W, SCREEN_H, roadImgA);
	iShowImage(0, (int)roadOffsetY + SCREEN_H, SCREEN_W, SCREEN_H, roadImgB);

	// Car
	if (drawCarNow) {
		if (isBatPlaying()) {
			iShowImage((int)carX + BAT_OFFSET_X, CAR_Y + BAT_OFFSET_Y,
				CAR_W + BAT_EXTRA_W, CAR_H, getCurrentBatImage());
		}
		else {
			iShowImage((int)carX, CAR_Y, CAR_W, CAR_H,
				brakeHeld ? carBrakedImg : carImg);
		}
	}

	// Obstacles
	for (int i = 0; i < MAX_OBS_ON_SCREEN; i++) {
		if (!gObs[i].active) continue;
		iShowImage((int)gObs[i].x, (int)gObs[i].y,
			gObs[i].width, gObs[i].height, obsImg[gObs[i].imgIndex]);
	}

	// Draw enemy
	drawEnemy();

	// Health display
	int currentHpImg = getCurrentHealthImage();
	if (currentHpImg != -1)
		iShowImage(20, SCREEN_H - 100, 200, 50, currentHpImg);

	iSetColor(255, 255, 255);
	char healthTxt[50];
	sprintf_s(healthTxt, "Health: %d/9", getRemainingHealth());
	iText(20, SCREEN_H - 120, healthTxt, GLUT_BITMAP_HELVETICA_18);

	// Debug info
	char txt[120];
	sprintf_s(txt, "Speed: %.1f   carX: %.0f      %s",
		roadSpeed, carX, isPaused ? "PAUSED" : "PLAYING");
	iText(10, SCREEN_H - 25, txt, GLUT_BITMAP_HELVETICA_18);

	// Draw pause screen on top if game is paused
	if (isPaused) {
		drawPauseScreen();
	}

	
}

//=============================================================================
// iGraphics Callbacks
//=============================================================================

void iDraw() {
	iClear();

	if (homePage) drawHome();
	else if (startPage) drawStart();
	else if (optionsPage) drawOptions();
	else if (controlsPage) drawControls();
	else if (creditsPage) drawCredits();
	else if (easyGame) drawEasyGame();
	else if (gameOverPage) drawGameOver();
}

void iMouseMove(int mx, int my) {
	// Optional: Add mouse move handling if needed
}

void iPassiveMouseMove(int mx, int my) {
	mouseX = mx;
	mouseY = my;
}

void iMouse(int button, int state, int mx, int my) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		printf("Mouse Click - x: %d\t y: %d\n", mx, my);

		// If game is paused, clicking anywhere resumes the game
		if (easyGame && isPaused) {
			isPaused = false;
			printf("Game Resumed by mouse click\n");
			return; // Don't process other clicks when paused
		}

		if (homePage) {
			if (isMouseInside(mx, my, 316, 602, 240, 321)) goToStart();
			else if (isMouseInside(mx, my, 316, 549, 68, 149)) goToOptions();
			else if (isMouseInside(mx, my, 553, 634, 68, 149)) exit(0);
		}
		else if (startPage) {
			if (isMouseInside(mx, my, 312, 392, 240, 320)) goToHome();
			else if (isMouseInside(mx, my, 402, 633, 240, 320)) {
				goToEasyGame();
				initGame();
			}
		}
		else if (optionsPage) {
			if (isMouseInside(mx, my, 268, 348, 240, 320)) goToHome();
			else if (isMouseInside(mx, my, 356, 631, 240, 320)) goToControls();
			else if (isMouseInside(mx, my, 356, 630, 152, 232)) goToCredits();
		}
		else if (controlsPage) {
			if (isMouseInside(mx, my, 64, 144, 652, 732)) goToOptions();
		}
		else if (creditsPage) {
			if (isMouseInside(mx, my, 74, 154, 532, 612)) goToOptions();
		}
		else if (easyGame && !isPaused) {
			startBat();
		}
		else if (gameOverPage) {
			goToHome();
		}
	}
}

// We don't need iKeyboard or iSpecialKeyboard functions anymore
// All keyboard handling is done in fixedUpdate

void updateGame() {
	// Don't update game logic if paused or game over or not in game
	if (isGameOver() || !easyGame || isPaused) {
		return;
	}

	// Bat trigger
	bool nowO = (isKeyPressed('o') != 0);
	if (nowO && !prevO) startBat();
	prevO = nowO;

	// Input states
	bool leftHeld = isKeyPressed('a') || isSpecialKeyPressed(GLUT_KEY_LEFT);
	bool rightHeld = isKeyPressed('d') || isSpecialKeyPressed(GLUT_KEY_RIGHT);
	bool accelerateHeld = isKeyPressed('w') || isSpecialKeyPressed(GLUT_KEY_UP);
	bool brakeHeld = isKeyPressed('s') || isKeyPressed(' ') || isSpecialKeyPressed(GLUT_KEY_DOWN);

	// Speed control
	if (accelerateHeld) {
		roadSpeed += ACCEL_RATE;
		if (roadSpeed > MAX_SPEED) roadSpeed = MAX_SPEED;
	}
	if (brakeHeld) {
		roadSpeed -= BRAKE_RATE;
		if (roadSpeed < MIN_SPEED) roadSpeed = MIN_SPEED;
	}
	if (!accelerateHeld && !brakeHeld) {
		roadSpeed -= FRICTION_RATE;
		if (roadSpeed < MIN_SPEED) roadSpeed = MIN_SPEED;
	}

	// Update positions
	updateCarPosition(leftHeld, rightHeld, roadSpeed);
	roadOffsetY -= roadSpeed;
	if (roadOffsetY <= -SCREEN_H) roadOffsetY += SCREEN_H;

	// Update obstacles
	updateObstacles((float)roadSpeed);

	// Update enemy
	updateEnemy(roadSpeed);

	// Collision detection
	if (!damageBlink) {
		// Check obstacle collision
		if (checkObstacleCollision((float)carX, (float)CAR_Y, (float)CAR_W, (float)CAR_H)) {
			triggerDamage();
			decreaseHealth();

			if (isGameOver()) {
				goToGameOver();
				mciSendString("stop bgsong", NULL, 0, NULL);  // Stop background music
				mciSendString("play ggsong from 0", NULL, 0, NULL);  // Play game over song
			}

			resetObstacles();
			roadSpeed = 8;
			if (roadSpeed < MIN_SPEED) roadSpeed = MIN_SPEED;
		}

		// Check enemy collision (separate from obstacles)
		if (checkEnemyCollision((float)carX, (float)CAR_Y, (float)CAR_W, (float)CAR_H)) {
			triggerDamage();
			decreaseHealth();

			if (isGameOver()) {
				goToGameOver();
				mciSendString("stop bgsong", NULL, 0, NULL);  // Stop background music
				mciSendString("play ggsong from 0", NULL, 0, NULL);  // Play game over song
			}

			// Enemy disappears on collision
			enemy.active = 0;
			roadSpeed = 8;
			if (roadSpeed < MIN_SPEED) roadSpeed = MIN_SPEED;
		}
	}

	// Update effects
	updateDamageBlink();
	updateBatAnimation();
}

//=============================================================================
// Main
//=============================================================================

int main() {
	//Opening/Loading the audio files
	mciSendString("open \"Audios//background.mp3\" alias bgsong", NULL, 0, NULL);
	mciSendString("open \"Audios//gameover.mp3\" alias ggsong", NULL, 0, NULL);

	// Playing the background audio on repeat
	mciSendString("play bgsong repeat", NULL, 0, NULL);

	iInitialize(SCREEN_W, SCREEN_H, "Errand Day");

	initImages();
	iSetTimer(12, updateGame);
	
	printf("Game started. Press ESC to pause.\n");

	iStart();

	return 0;
}
