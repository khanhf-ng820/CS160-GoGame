#include "UI.h"

int main() {
	// For print debugging
	std::cout << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10);

	// Core setup
	Game game(19);
	AIDifficulty d = AIDifficulty::EASY;
	GoAI ai(d);
	std::mt19937 rng{std::random_device{}()};

	// Run GUI
	UI ui(game, ai, rng);
	ui.run_graphical();
	return 0;
}
