#ifndef SCORE_HPP
#define SCORE_HPP

// Score system for tracking player points
struct ScoreSystem {
	int currentScore;

	ScoreSystem() : currentScore(0) {}

	void reset() {
		currentScore = 0;
	}

	void addPoints(int points) {
		currentScore += points;
	}

	int getScore() const {
		return currentScore;
	}

	// Draw score on screen (top-right corner)
	void draw(int screenW, int screenH) const {
		char scoreText[50];
		sprintf_s(scoreText, "Score: %d", currentScore);

		// Set text color to white
		iSetColor(255, 255, 255);

		// Draw score in top-right corner with some padding
		int textWidth = 150; // Approximate width for positioning
		iText(screenW - textWidth - 20, screenH - 40, scoreText, GLUT_BITMAP_HELVETICA_18);
	}
};

#endif