#ifndef HEALTH_SYSTEM_HPP
#define HEALTH_SYSTEM_HPP

// Image-based health system.
// Expects: Assets\\hp\\car_hp_1.png ... car_hp_10.png
// healthIndex 1 = full, 10 = game over.

struct HealthSystem {
	static const int MAX_IMAGES = 10;
	static const int START_INDEX = 1;
	static const int GAME_OVER_INDEX = 10;

	int hpImg[MAX_IMAGES + 1];
	int healthIndex;
	bool gameOver;

	HealthSystem() : healthIndex(START_INDEX), gameOver(false) {
		for (int i = 0; i <= MAX_IMAGES; i++) hpImg[i] = -1;
	}

	void loadImages() {
		for (int i = 1; i <= MAX_IMAGES; i++) {
			char path[128];
			sprintf_s(path, "Assets\\hp\\car_hp_%d.png", i);
			hpImg[i] = iLoadImage(path);
		}
	}

	void reset() {
		healthIndex = START_INDEX;
		gameOver = false;
	}

	void takeHit() {
		if (gameOver) return;
		if (healthIndex < GAME_OVER_INDEX) healthIndex++;
		if (healthIndex >= GAME_OVER_INDEX) gameOver = true;
	}

	bool isGameOver() const { return gameOver; }

	// Draw HUD at top-left.
	void draw(int screenW, int screenH) const {
		if (gameOver) return;
		if (healthIndex < 1 || healthIndex >= GAME_OVER_INDEX) return;
		int img = hpImg[healthIndex];
		if (img < 0) return;

		const int W = 170;
		const int H = 55;
		const int X = 18;
		const int Y = screenH - H - 36;
		iShowImage(X, Y, W, H, img);
	}
};

#endif