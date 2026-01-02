#include "UI.h"

int main() {
	// For print debugging
	std::cout << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10);
	std::cout << "[ASSET_DIR] " << ASSET_DIR << std::endl << "[SAVEGAME_DIR] " << SAVEGAME_DIR << std::endl;

	// Core setup
	Game game(19);
	AIDifficulty d = AIDifficulty::EASY;
	std::random_device rd; // RNG
	std::mt19937 rng(rd());
	std::mt19937_64 rng64(rd());
	GoAI ai(d, &rng, &rng64);

	// Run GUI
	UI ui(game, ai, rng);
	ui.run_graphical();
	return 0;
}
