#ifndef HIGHSCORE_HPP
#define HIGHSCORE_HPP

#include <vector>
#include <string>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <algorithm>

// Structure to store high score entry
struct HighScoreEntry {
	char name[50];
	int score;
	int difficulty;  // 0=Easy, 1=Medium, 2=Hard

	HighScoreEntry() : score(0), difficulty(0) {
		strcpy_s(name, sizeof(name), "Unknown");
	}

	HighScoreEntry(const char* n, int s, int d) : score(s), difficulty(d) {
		strncpy_s(name, sizeof(name), n, 49);
		name[49] = '\0';
	}

	// Comparison operator for sorting (higher score first)
	bool operator>(const HighScoreEntry& other) const {
		return score > other.score;
	}
};

// High Score System class
class HighScoreSystem {
private:
	std::vector<HighScoreEntry> scores;
	static const int MAX_ENTRIES = 4;  // Show top 4 players by score
	const char* filename = "highscores.dat";

	// Convert difficulty to string
	const char* getDifficultyString(int diff) const {
		switch (diff) {
		case 0: return "Easy";
		case 1: return "Medium";
		case 2: return "Hard";
		default: return "Unknown";
		}
	}

	// Sort scores in descending order (highest score first)
	void sortScores() {
		std::sort(scores.begin(), scores.end(), [](const HighScoreEntry& a, const HighScoreEntry& b) {
			return a.score > b.score;
		});
	}

public:
	HighScoreSystem() {
		loadFromFile();
	}

	// Load high scores from file
	void loadFromFile() {
		scores.clear();
		std::ifstream file(filename, std::ios::binary);
		if (file.is_open()) {
			HighScoreEntry entry;
			while (file.read((char*)&entry, sizeof(HighScoreEntry))) {
				scores.push_back(entry);
			}
			file.close();
		}

		// Sort scores by highest first
		sortScores();

		// If no scores or less than MAX_ENTRIES, fill with empty entries
		while (scores.size() < MAX_ENTRIES) {
			scores.push_back(HighScoreEntry("---", 0, 0));
		}

		// Keep only top MAX_ENTRIES
		if (scores.size() > MAX_ENTRIES) {
			scores.resize(MAX_ENTRIES);
		}
	}

	// Save high scores to file
	void saveToFile() {
		std::ofstream file(filename, std::ios::binary);
		if (file.is_open()) {
			for (const auto& entry : scores) {
				file.write((char*)&entry, sizeof(HighScoreEntry));
			}
			file.close();
		}
	}

	// Add new score (will add to list, sort, and keep top 4)
	void addScore(const char* name, int score, int difficulty) {
		HighScoreEntry newEntry(name, score, difficulty);
		scores.push_back(newEntry);

		// Sort scores by highest first
		sortScores();

		// Keep only top MAX_ENTRIES
		if (scores.size() > MAX_ENTRIES) {
			scores.resize(MAX_ENTRIES);
		}

		saveToFile();
	}

	// Get all scores (returns reference to vector)
	const std::vector<HighScoreEntry>& getScores() const {
		return scores;
	}

	// Draw high score screen centered
	void draw(int screenW, int screenH) const {
		// Title
		iSetColor(255, 255, 255);
		char title[] = "HIGH SCORES";
		iText(screenW / 2 - 100, screenH - 150, title, GLUT_BITMAP_HELVETICA_18);

		// Draw line under title
		iLine(screenW / 2 - 150, screenH - 165, screenW / 2 + 150, screenH - 165);

		// Column headers
		iSetColor(255, 200, 100);
		char rankHeader[] = "RANK";
		char playerHeader[] = "PLAYER";
		char scoreHeader[] = "SCORE";
		char diffHeader[] = "DIFFICULTY";

		iText(screenW / 2 - 200, screenH - 220, rankHeader, GLUT_BITMAP_HELVETICA_18);
		iText(screenW / 2 - 100, screenH - 220, playerHeader, GLUT_BITMAP_HELVETICA_18);
		iText(screenW / 2 + 50, screenH - 220, scoreHeader, GLUT_BITMAP_HELVETICA_18);
		iText(screenW / 2 + 150, screenH - 220, diffHeader, GLUT_BITMAP_HELVETICA_18);

		iSetColor(255, 255, 255);

		// Display scores (already sorted by highest score first)
		int startY = screenH - 280;
		int rank = 1;

		// Display from highest score to lowest (already sorted)
		for (int i = 0; i < (int)scores.size(); i++) {
			const HighScoreEntry& entry = scores[i];

			// Only show if score > 0 or not empty entry
			if (entry.score > 0 || strcmp(entry.name, "---") != 0) {
				char rankText[10];
				sprintf_s(rankText, "#%d", rank);

				char scoreText[20];
				sprintf_s(scoreText, "%d", entry.score);

				char nameText[50];
				strcpy_s(nameText, entry.name);

				const char* diffText = getDifficultyString(entry.difficulty);
				char diffDisplay[20];
				strcpy_s(diffDisplay, diffText);

				iText(screenW / 2 - 200, startY, rankText, GLUT_BITMAP_HELVETICA_18);
				iText(screenW / 2 - 100, startY, nameText, GLUT_BITMAP_HELVETICA_18);
				iText(screenW / 2 + 50, startY, scoreText, GLUT_BITMAP_HELVETICA_18);
				iText(screenW / 2 + 150, startY, diffDisplay, GLUT_BITMAP_HELVETICA_18);

				startY -= 60;
				rank++;
			}
		}

		// Show instruction text
		iSetColor(200, 200, 200);
		char instruction[] = "Click BACK to return";
		iText(screenW / 2 - 100, 50, instruction, GLUT_BITMAP_HELVETICA_18);
	}

	// Check if there are any real scores
	bool hasScores() const {
		for (const auto& entry : scores) {
			if (entry.score > 0 && strcmp(entry.name, "---") != 0) {
				return true;
			}
		}
		return false;
	}
};

#endif 
