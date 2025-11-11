#include "UI.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <cmath>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

static constexpr float PANEL_W   = 120.f;
static constexpr float PANEL_GAP = 60.f;
static constexpr float RIGHT_PAD = 10.f;
static constexpr unsigned MIN_WIN_H = 720;

float UI::panel_top_y() const {
    float floorY = (BOARD_SIZE == 9 ? 120.f : 170.f);
    if (infoText) {
        auto gb = infoText->getGlobalBounds();
        float extra =
            (BOARD_SIZE == 9  ? 12.f :
             BOARD_SIZE >= 13 ? 6.f :
                                16.f);
        return std::max(floorY, gb.position.y + gb.size.y + extra);
    }
    return floorY;
}

float UI::panel_min_height() const {
    const float BTN_H   = 32.f;
    const float ROW_GAP = (BOARD_SIZE == 9 ? 8.f : 10.f);

    float y = panel_top_y();

    if (BOARD_SIZE == 9) {
        // Size
        y += BTN_H + ROW_GAP;
        y += (BTN_H + ROW_GAP) * 5; // Undo/ Redo, Pass/ Mode, NewGame/ Save, Load/ Theme, Music/ Quit
        y += 8.f;
    } else {
        // Size, Undo/ Redo, Pass, Mode
        // New Game, Save/ Load, Theme/ Music, Quit
        const int span_rows = 5;
        const int pair_rows = 3;

        y += (BTN_H + ROW_GAP) * span_rows;
        y += (BTN_H + ROW_GAP) * pair_rows;
        y += 10.f;
    }
    return y;
}

sf::FloatRect UI::view_rect() const {
    sf::Vector2f sz  = logicalView.getSize();
    sf::Vector2f ctr = logicalView.getCenter();
    return {
        { ctr.x - sz.x * 0.5f, ctr.y - sz.y * 0.5f },
        { sz.x, sz.y }
    };
}

// Console UI
UI::UI(Game& g, GoAI& a, std::mt19937& rng_)
	: game(g), ai(a), rng(rng_) {}

void UI::print_menu() const {
	std::cout <<
	"Commands:\n"
	" move like D4, or 'pass'\n"
	" undo | redo | save <file> | load <file> | reset | mode | help | quit\n";
}

void UI::handle_command(const std::string& line_in) {
	std::string line = trim(line_in);
	if(line=="") return;

	if(line=="quit"||line=="exit"){ std::exit(0); }
	if(line=="help"){ print_menu(); return; }
	if(line=="reset"){ game.reset(); return; }
	if(line=="undo"){ if(!game.undo()) std::cout << "Cannot undo.\n"; return; }
	if(line=="redo"){ if(!game.redo()) std::cout << "Cannot redo.\n"; return; }
	if(line=="pass"){ game.pass(); return; }

	if(line.rfind("save ",0)==0){
		std::string path = trim(line.substr(5));
		std::ofstream f(path);
		if(!f){ std::cout<<"Failed to open file for saving.\n"; return; }
		f << game.serialize();
		std::cout<<"Game saved to "<<path<<".\n";
		return;
	}

	if(line.rfind("load ",0)==0){
		std::string path = trim(line.substr(5));
		std::ifstream f(path);
		if(!f){ std::cout<<"Failed to open file for loading.\n"; return; }
		std::string data((std::istreambuf_iterator<char>(f)), {});
		if(!game.deserialize(data)) std::cout<<"Failed to load game from file.\n";
		else std::cout<<"Game loaded from "<<path<<".\n";
		return;
	}

	if(line=="mode") {
		std::cout<<"Current mode: " <<(mode==GameMode::PVP?"2-Player":"Player vs AI")<<"\n";
		std::cout<<"Enter 'pvp' or 'pve'. For PvE, choose difficulty: 'easy', 'medium', 'hard'.\n";
		std::cout<<"mode> ";
		std::string mline; std::getline(std::cin,mline); mline=trim(mline);
		if(mline=="pvp"){ mode = GameMode::PVP; return; }
		if(mline=="pve"){
			mode = GameMode::PVE;
			std::cout<<"Choose AI difficulty (easy, medium, hard)> ";
			std::string dline; std::getline(std::cin,dline); dline=trim(dline);
			if(dline=="medium") diff = AIDifficulty::MEDIUM;
			else if(dline=="hard") diff = AIDifficulty::HARD;
			else diff = AIDifficulty::EASY;
			ai = GoAI(diff);
			return;
		}
		std::cout<<"Unchanged.\n"; return;
	}

	Move m = Game::parse_move(line, game.size());
	if(m.is_pass){ game.pass(); return; }
	if(!game.legal(m)){ std::cout<<"Illegal move.\n"; return; }
	game.play(m);
}

void UI::ai_turn() {
	Move m = ai.choose_move(game, rng);
	if(m.is_pass){ game.pass(); return; }
	else { game.play(m); std::cout<<"[AI] plays "<<char_from_col(m.c)<<(m.r+1)<<"\n"; }
}

void UI::run_console() {
	std::cout << "Welcome to Console Go!\n";
	print_menu();
	while(true) {
		std::cout << game.render_ascii();
		if(game.is_over()){
			Score sc = game.score();
			std::cout << "Game over.\n";
			std::cout << "Black: "<<sc.black<<" | White: "<<sc.white<<" (komi "<<game.komi<<")\n";
			std::cout << ((sc.black>sc.white)?"Black wins!\n":"White wins!\n");
		}
		if(mode==GameMode::PVE && game.side_to_move()==Stone::WHITE) {
			ai_turn();
			continue;
		}
		std::cout << "> ";
		std::string line;
		if(!std::getline(std::cin,line)) break;
		handle_command(line);
	}
}

// Graphical UI (SFML 3)
std::vector<sf::Vector2i> UI::hoshi_points(int N) {
	std::vector<int> p;
	if (N == 19) p = {3, 9, 15};
	else if (N == 13) p = {3, 6, 9};
	else if (N == 9)  p = {2, 4, 6};
	else return {};
	std::vector<sf::Vector2i> pts;
	for (int r : p) for (int c : p) pts.push_back({c, r});
	if (N == 13) pts.push_back({6, 6});
	if (N == 9)  pts.push_back({4, 4});
	return pts;
}

void UI::run_graphical() {
	BOARD_SIZE = game.size();

	const int gridW = MARGIN * 2 + CELL * (BOARD_SIZE - 1);
	const int gridH = MARGIN * 2 + CELL * (BOARD_SIZE - 1);

	const unsigned winW = unsigned(gridW + PANEL_GAP + PANEL_W + RIGHT_PAD);
	const unsigned winH = unsigned(gridH);

	// Create window with anti-aliasing
	window.create(
		sf::VideoMode({winW, winH}),
		"CS 160 - Go Game Project",
		sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
	);
	sync_view_to_window();
	baseWindow = window.getSize();
	logicalView.setSize({float(baseWindow.x), float(baseWindow.y)});
	logicalView.setCenter({float(baseWindow.x)*0.5f, float(baseWindow.y)*0.5f});
	logicalView.setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));

	window.setView(logicalView);
	// window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);
	gui_update_window_size();

	// Assets
	(void)font.openFromFile("assets/fonts/OpenSans-Regular.ttf");
	(void)placeBuf.loadFromFile("assets/sounds/place.wav");

	// Init pool
	if (placeBuf.getSampleCount() > 0) {
		placeSnds.clear();
		placeSnds.reserve(PLACE_VOICES);
		for (int i = 0; i < PLACE_VOICES; ++i) {
			placeSnds.emplace_back(placeBuf);   // Sound with buffer
			placeSnds.back().setVolume(volume);
		}
	} else {
		std::cerr << "[WARN] Cannot open assets/sounds/place.wav\n";
	}

	// Create text right after has font
	turnText.emplace(font);
	infoText.emplace(font);
	thinkingText.emplace(font);

	// Split Black/ White + songs name
	turnSideText.emplace(font);
	turnSideText->setCharacterSize(22);

	trackText.emplace(font);
	trackText->setCharacterSize(16);
	trackText->setFillColor(sf::Color(30,30,30));
	trackText->setOutlineColor(sf::Color::Black);
	trackText->setOutlineThickness(1.f);
	trackText->setString("Now playing: (none)");

	// Sound
	hasMusic = bgm.openFromFile("assets/music/bg_music.ogg");
	if (hasMusic) {
		bgm.setLooping(true);
		bgm.play();
	}

	// Check music folder
	scan_music_folder();
	if (!musicFiles.empty()) {
		int def = 0;
		for (int i = 0; i < (int)musicFiles.size(); ++i)
			if (fs::path(musicFiles[i]).filename().string() == "bg_music.ogg") { def = i; break; }
		play_music_at(def); //play_music_at will automatically update trackText (fade effect)	
	}

	// Volume label
	volLabel.emplace(font);
	volLabel->setCharacterSize(16);
	volLabel->setFillColor(sf::Color::Black);
	set_volume(volume); // Sync volume

	// Stones
	blackCircle = sf::CircleShape(CELL * 0.45f);
	whiteCircle = sf::CircleShape(CELL * 0.45f);

	blackCircle.setOrigin({ blackCircle.getRadius(), blackCircle.getRadius() });
	whiteCircle.setOrigin({ whiteCircle.getRadius(), whiteCircle.getRadius() });

	// Make stones smoother
	blackCircle.setPointCount(64);
	whiteCircle.setPointCount(64);

	// Stones colors with small changes
	blackCircle.setFillColor(sf::Color(30, 30, 30));
	whiteCircle.setFillColor(sf::Color(250, 250, 250));

	// Remove border of stones
	blackCircle.setOutlineThickness(0.f);
	whiteCircle.setOutlineThickness(0.f);

	// Themes
    themes = {
        { sf::Color(219,178,92),  sf::Color(185,147,77),  sf::Color::Black, sf::Color::Black, 18.f, 2.f },
        { sf::Color(235,210,130), sf::Color(205,175,105), sf::Color(40,40,40), sf::Color(40,40,40), 18.f, 2.f },
        { sf::Color(168,120,60),  sf::Color(130,95,50),   sf::Color(230,230,230), sf::Color(230,230,230), 18.f, 2.f },
        { sf::Color(190,205,160), sf::Color(150,170,130), sf::Color(30,60,30), sf::Color(30,60,30), 18.f, 2.f },
        { sf::Color(190,200,220), sf::Color(150,160,185), sf::Color(30,40,65), sf::Color(30,40,65), 18.f, 2.f }
    };
	theme = themes[themeIdx];

	// Heads-Up Display position
	float titleY = float(MARGIN - 30);
	float titleX = float(MARGIN + (BOARD_SIZE-1)*CELL/2 - 50);
	turnText->setCharacterSize(20);
	turnText->setFillColor(sf::Color::Black);
	turnText->setPosition({float(gridW + 20), 40.f});

	thinkingText->setCharacterSize(18);
	thinkingText->setFillColor(sf::Color(200,32,32));
	thinkingText->setString("AI thinking...");
	thinkingText->setPosition({float(gridW - 160.f), 12.f});

	// Main Buttons (Right)
	build_main_buttons(gridW);

	while (window.isOpen()) {
		gui_handle_events();
		gui_update();
		gui_render();
	}
}

void UI::build_main_buttons(int gridW) {
    buttons.clear();

	auto makeBtn = [&](const std::string& txt, float x, float y,
					std::function<void()> fn,
					sf::Vector2f size = {120.f, 32.f})
	{
		Button b;
		b.rect.setSize(size);
		b.rect.setFillColor(sf::Color(220,200,120));
		b.rect.setOutlineColor(sf::Color::Black);
		b.rect.setOutlineThickness(1.f);
		b.rect.setPosition({x, y});

		b.label.emplace(font);
		b.label->setString(txt);

		// Auto fit text width
		unsigned minSize = 12;
		unsigned curSize = 16;     // default size
		b.label->setCharacterSize(curSize);
		b.label->setFillColor(sf::Color::Black);

		auto fits = [&](unsigned s)->bool{
			b.label->setCharacterSize(s);
			auto lb = b.label->getLocalBounds();
			float avail = size.x - 20.f; // 10px padding each side
			return lb.size.x <= avail;
		};
		while (curSize > minSize && !fits(curSize)) --curSize;
		// If not fit, cut and replace with ...
		if (!fits(curSize)) {
			std::string t = txt;
			while (t.size() > 1 && !fits(curSize)) {
				t.pop_back();
				b.label->setString(t + "…");
			}
		}

		auto lb = b.label->getLocalBounds();
		b.label->setPosition({
			x + 10.f,
			y + (size.y - lb.size.y) * 0.5f - lb.position.y
		});

		b.onClick = std::move(fn);
		buttons.push_back(std::move(b));
	};

    const float bx = float(MARGIN + (BOARD_SIZE - 1) * CELL + PANEL_GAP);
    float y = panel_top_y() - panelScroll;

    // Column
    const float COL_GAP = 10.f;
    const float ROW_GAP = (BOARD_SIZE == 9 ? 8.f : 10.f);
    const float BTN_H   = 32.f;
    const float colW    = (PANEL_W - COL_GAP) * 0.5f;

    auto addRow2 = [&](const std::string& left, std::function<void()> lfn,
                       const std::string& right, std::function<void()> rfn)
    {
        makeBtn(left,  bx,                 y, std::move(lfn), {colW, BTN_H});
        makeBtn(right, bx + colW + COL_GAP, y, std::move(rfn), {colW, BTN_H});
        y += BTN_H + ROW_GAP;
    };
    auto addSpan2 = [&](const std::string& txt, std::function<void()> fn)
    {
        makeBtn(txt, bx, y, std::move(fn), {PANEL_W, BTN_H});
        y += BTN_H + ROW_GAP;
    };

    // Size
    addSpan2("Size", [this, gridW]{ build_board_size_modal(gridW); });

    if (BOARD_SIZE == 9) {
        // 5-5 layout
        addRow2("Undo",
            [this]{ if (!aiThinking) { game.undo(); lastMove.reset(); } },
                "Redo",
            [this]{ if (!aiThinking) { game.redo(); lastMove.reset(); } });

        addRow2("Pass",
            [this]{
                if (!aiThinking) {
                    game.pass(); lastMove.reset();
                    if (mode==GameMode::PVE && game.side_to_move()==Stone::WHITE) gui_start_ai();
                }
            },
                "Mode",
            [this, gridW]{ request_switch_mode(gridW); });

        addRow2("New Game",
            [this, gridW]{
                if (board_has_any_stone()) build_confirm_newgame_modal(gridW);
                else gui_reset();
            },
                "Save",
            [this, gridW]{ build_save_load_modal(Modal::Save, gridW); });

        addRow2("Load",
            [this, gridW]{ build_save_load_modal(Modal::Load, gridW); },
                "Theme",
            [this, gridW]{ build_theme_modal(gridW); });

		addRow2("Music",
			[this, gridW]{ build_music_modal(gridW); },
			"Quit",
			[this]{ build_confirm_quit_modal(); }
		);
    } else {
        // Layout for 13×13 or 19×19
        addRow2("Undo",
            [this]{ if (!aiThinking) { game.undo(); lastMove.reset(); } },
                "Redo",
            [this]{ if (!aiThinking) { game.redo(); lastMove.reset(); } });

        addSpan2("Pass", [this]{
            if (!aiThinking) {
                game.pass(); lastMove.reset();
                if (mode==GameMode::PVE && game.side_to_move()==Stone::WHITE) gui_start_ai();
            }
        });

        addSpan2("Mode", [this, gridW]{ request_switch_mode(gridW); });

        addSpan2("New Game", [this, gridW]{
            if (board_has_any_stone()) build_confirm_newgame_modal(gridW);
            else gui_reset();
        });

        addRow2("Save",
            [this, gridW]{ build_save_load_modal(Modal::Save, gridW); },
                "Load",
            [this, gridW]{ build_save_load_modal(Modal::Load, gridW); });

        addRow2("Theme",
            [this, gridW]{ build_theme_modal(gridW); },
                "Music",
            [this, gridW]{ build_music_modal(gridW); });

        addSpan2("Quit", [this]{ build_confirm_quit_modal(); });
    }

    // gui_update_window_size();

	// Update scroll limit
	float topY = panel_top_y();
	float visibleH = float(window.getSize().y) - topY - 10.f;
	float totalH = panel_min_height() - topY;
	panelScrollMax = std::max(0.f, totalH - visibleH);

	// Panel
	panelViewport = sf::FloatRect({ bx, topY }, { PANEL_W, std::max(0.f, visibleH) });
}

namespace {
    bool file_nonempty(const std::string& p) {
        std::error_code ec;
        return std::filesystem::exists(p, ec)
            && std::filesystem::is_regular_file(p, ec)
            && std::filesystem::file_size(p, ec) > 0;
    }
}

// Save/ load modal
void UI::build_save_load_modal(Modal type, int /*gridW*/) {
    activeModal = type;
    modalButtons.clear();
    fs::create_directories("saves");
    const int SLOTS = 6;

    const float panelW = 260.f;
	auto vr   = view_rect();
	float panelX = vr.position.x + vr.size.x - panelW - 20.f;
	float panelY = vr.position.y + 160.f;
    const float pad    = 14.f;

    modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, 0});

    auto makeBtn = [&](const std::string& txt, float x, float y, std::function<void()> fn) {
        Button b;
        b.rect.setSize({panelW - 2*pad, 32.f});
        b.rect.setFillColor(sf::Color(240,220,150));
        b.rect.setOutlineColor(sf::Color::Black);
        b.rect.setOutlineThickness(1.f);
        b.rect.setPosition({x, y});
        b.label.emplace(font);
        b.label->setString(txt);
        b.label->setCharacterSize(16);
        b.label->setFillColor(sf::Color::Black);
        auto lb = b.label->getLocalBounds();
        b.label->setPosition({ x + 10.f, y + (32.f - lb.size.y) * 0.5f - lb.position.y });
        b.onClick = std::move(fn);
        modalButtons.push_back(std::move(b));
    };

    const float xLeft = panelX + pad;
    float y = panelY + 40.f; // Below title

    for (int i = 1; i <= SLOTS; ++i) {
        std::string path = "saves/slot" + std::to_string(i) + ".sav";
        bool occupied = file_nonempty(path);
        std::string label = (type == Modal::Save)
            ? ("Save to Slot " + std::to_string(i) + (occupied ? " (occupied)" : " (empty)"))
            : ("Load Slot "    + std::to_string(i) + (occupied ? " (saved)"    : " (empty)"));

        makeBtn(label, xLeft, y, [this, path, occupied, type]{
            if (type == Modal::Save) {
                if (occupied) { build_confirm_overwrite_modal( /*gridW*/ 0, path ); return; }
                std::ofstream f(path, std::ios::binary);
                if (f) {
                    f << game.serialize();
                    if (deferredAction) { auto act = std::move(deferredAction); deferredAction = nullptr; act(); }
                }
                activeModal = Modal::None;
            } else {
                if (occupied) {
                    std::ifstream f(path, std::ios::binary);
                    if (f) { std::string s((std::istreambuf_iterator<char>(f)), {}); game.deserialize(s); lastMove.reset(); }
                }
                activeModal = Modal::None;
            }
        });
        y += 40.f;
    }

    makeBtn("Cancel", xLeft, y + 8.f, [this]{ activeModal = Modal::None; });
	y += 8.f + 32.f;

	// Update panel size
	modalPanelRect.size.y = (y + 8.f + pad) - panelY;

	// Centerline
	center_modal_vertically();
}

void UI::build_theme_modal(int /*gridW*/) {
    activeModal = Modal::Theme;
    modalButtons.clear();

    const float panelW = 260.f;
    const float pad    = 14.f;

    auto vr   = view_rect();
    float panelX = vr.position.x + vr.size.x - panelW - 20.f;
    float panelY = vr.position.y + 160.f;

    modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, 0.f});

    auto makeBtn = [&](const std::string& name, float x, float y, int idx) {
        Button b;

        const auto& th = themes[idx];
        b.rect.setSize({ panelW - 2*pad, 32.f });
        b.rect.setFillColor(th.board);
        b.rect.setOutlineColor(th.border);
        b.rect.setOutlineThickness(2.f);
        b.rect.setPosition({ x, y });

        b.label.emplace(font);
        b.label->setString(name);
        b.label->setCharacterSize(16);

        float br = 0.299f*th.board.r + 0.587f*th.board.g + 0.114f*th.board.b;
        b.label->setFillColor(br < 110 ? sf::Color::White : sf::Color::Black);

        auto lb = b.label->getLocalBounds();
        b.label->setPosition({ x + 10.f, y + (32.f - lb.size.y) * 0.5f - lb.position.y });

        b.onClick = [this, idx] {
            gui_change_theme(idx);
            activeModal = Modal::None;
        };

        modalButtons.push_back(std::move(b));
    };

    const float xLeft = panelX + pad;
    float y = panelY + 40.f;

    makeBtn("Wood", xLeft, y, 0); y += 40.f;
    makeBtn("Sand",   xLeft, y, 1); y += 40.f;
    makeBtn("Caramel",    xLeft, y, 2); y += 40.f;
    makeBtn("Sage",   xLeft, y, 3); y += 40.f;
    makeBtn("Blue",    xLeft, y, 4);  y += 48.f;

    // Cancel
    {
        Button c;
        c.rect.setSize({ panelW - 2*pad, 32.f });
        c.rect.setFillColor(sf::Color(240,220,150));
        c.rect.setOutlineColor(sf::Color::Black);
        c.rect.setOutlineThickness(1.f);
        c.rect.setPosition({ xLeft, y });

        c.label.emplace(font);
        c.label->setString("Cancel");
        c.label->setCharacterSize(16);
        auto lb = c.label->getLocalBounds();
        c.label->setFillColor(sf::Color::Black);
        c.label->setPosition({ xLeft + 10.f, y + (32.f - lb.size.y) * 0.5f - lb.position.y });

        c.onClick = [this]{ activeModal = Modal::None; };
        modalButtons.push_back(std::move(c));

        y += 32.f;
    }

    // Update panel height
    modalPanelRect.size.y = (y + pad) - panelY;

	// Centerline
	center_modal_vertically();
}

void UI::request_switch_mode(int gridW) {
    // Check whether play or not
    if (!lastMove) {
        mode = (mode==GameMode::PVP ? GameMode::PVE : GameMode::PVP);
        gui_reset();
        if (mode==GameMode::PVE && game.side_to_move()==Stone::WHITE) gui_start_ai();
        return;
    }

    // Ask
    build_confirm_switch_modal(gridW);
}

void UI::build_confirm_switch_modal(int gridW) {
    activeModal = Modal::ConfirmSwitch;
    modalButtons.clear();

    const float panelW = 260.f;
    const float pad    = 14.f;

    auto vr   = view_rect();
    float panelX = vr.position.x + vr.size.x - panelW - 20.f;
    float panelY = vr.position.y + 160.f;

    modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, 0.f});

    auto makeBtn = [&](const std::string& txt, float x, float y,
                       std::function<void()> fn, sf::Vector2f size) {
        Button b;
        b.rect.setSize(size);
        b.rect.setFillColor(sf::Color(240,220,150));
        b.rect.setOutlineColor(sf::Color::Black);
        b.rect.setOutlineThickness(1.f);
        b.rect.setPosition({x, y});
        b.label.emplace(font);
        b.label->setString(txt);
        b.label->setCharacterSize(16);
        b.label->setFillColor(sf::Color::Black);
        auto lb = b.label->getLocalBounds();
        b.label->setPosition({ x + 10.f, y + (size.y - lb.size.y) * 0.5f - lb.position.y });
        b.onClick = std::move(fn);
        modalButtons.push_back(std::move(b));
    };

    const float xLeft = panelX + pad;
    float y = panelY + 40.f; // Below title

    // Save & Switch
    makeBtn("Save & Switch", xLeft, y, [this, gridW]{
        // After save -> change mode + reset
        deferredAction = [this]{
            mode = (mode==GameMode::PVP ? GameMode::PVE : GameMode::PVP);
            gui_reset();
            if (mode==GameMode::PVE && game.side_to_move()==Stone::WHITE) gui_start_ai();
            activeModal = Modal::None;
        };
        build_save_load_modal(Modal::Save, gridW);
    }, {panelW - 2*pad, 32.f}); y += 40.f;

    // Switch (Don't Save)
    makeBtn("Switch (Don't Save)", xLeft, y, [this]{
        mode = (mode==GameMode::PVP ? GameMode::PVE : GameMode::PVP);
        gui_reset();
        if (mode==GameMode::PVE && game.side_to_move()==Stone::WHITE) gui_start_ai();
        activeModal = Modal::None;
    }, {panelW - 2*pad, 32.f}); y += 40.f;

    // Cancel
    makeBtn("Cancel", xLeft, y, [this]{ activeModal = Modal::None; },
            {panelW - 2*pad, 32.f}); y += 40.f;

    // Panel height
    modalPanelRect.size.y = (y + pad) - panelY;

	center_modal_vertically();
}

void UI::build_confirm_overwrite_modal(int gridW, const std::string& path) {
    activeModal = Modal::ConfirmOverwrite;
    modalButtons.clear();

    const float panelW = 260.f;
    const float pad    = 14.f;

    auto vr   = view_rect();
    float panelX = vr.position.x + vr.size.x - panelW - 20.f;
    float panelY = vr.position.y + 160.f;
	
    modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, 0.f});

    auto makeBtn = [&](const std::string& txt, float x, float y,
                       std::function<void()> fn, sf::Vector2f size) {
        Button b;
        b.rect.setSize(size);
        b.rect.setFillColor(sf::Color(240,220,150));
        b.rect.setOutlineColor(sf::Color::Black);
        b.rect.setOutlineThickness(1.f);
        b.rect.setPosition({x, y});
        b.label.emplace(font);
        b.label->setString(txt);
        b.label->setCharacterSize(16);
        b.label->setFillColor(sf::Color::Black);
        auto lb = b.label->getLocalBounds();
        b.label->setPosition({ x + 10.f, y + (size.y - lb.size.y) * 0.5f - lb.position.y });
        b.onClick = std::move(fn);
        modalButtons.push_back(std::move(b));
    };

    const float xLeft = panelX + pad;
    float y = panelY + 40.f; // Below title

    // Overwrite
	makeBtn("Overwrite", xLeft, y, [this, path]{
		std::ofstream f(path);
		if (f) {
			f << game.serialize();
			if (deferredAction) { auto act = std::move(deferredAction); deferredAction = nullptr; act(); }
		}
		activeModal = Modal::None;
	}, {panelW - 2*pad, 32.f});
	y += 40.f;

	// Cancel
	makeBtn("Cancel", xLeft, y, [this, gridW]{
		build_save_load_modal(Modal::Save, gridW);
	}, {panelW - 2*pad, 32.f});
	y += 40.f;

	// Panel height
	modalPanelRect.size.y = (y + pad) - panelY;

	center_modal_vertically();
}

void UI::build_confirm_diff_modal(AIDifficulty newDiff, int gridW) {
    activeModal = Modal::ConfirmDifficulty;
    modalButtons.clear();

    const float panelW = 260.f;
    const float pad    = 14.f;

    auto vr   = view_rect();
    float panelX = vr.position.x + vr.size.x - panelW - 20.f;
    float panelY = vr.position.y + 160.f;
	
    modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, 0.f});

    auto makeBtn = [&](const std::string& txt, float x, float y,
                       std::function<void()> fn, sf::Vector2f size) {
        Button b;
        b.rect.setSize(size);
        b.rect.setFillColor(sf::Color(240,220,150));
        b.rect.setOutlineColor(sf::Color::Black);
        b.rect.setOutlineThickness(1.f);
        b.rect.setPosition({x, y});

        b.label.emplace(font);
        b.label->setString(txt);
        b.label->setCharacterSize(16);
        b.label->setFillColor(sf::Color::Black);
        auto lb = b.label->getLocalBounds();
        b.label->setPosition({
            x + 10.f,
            y + (size.y - lb.size.y) * 0.5f - lb.position.y
        });

        b.onClick = std::move(fn);
        modalButtons.push_back(std::move(b));
    };

    const float xLeft = panelX + pad;
    float y = panelY + 40.f;

    // 1) Save & Change
    makeBtn("Save & Change", xLeft, y, [this, newDiff, gridW]{
        deferredAction = [this, newDiff] {
            diff = newDiff;
            ai = GoAI(diff);
            gui_reset();
            if (mode == GameMode::PVE && game.side_to_move() == Stone::WHITE)
                gui_start_ai();
            activeModal = Modal::None;
        };
        build_save_load_modal(Modal::Save, gridW);
    }, {panelW - 2*pad, 32.f});
    y += 40.f;

    // 2) Change (Don't Save)
    makeBtn("Change (Don't Save)", xLeft, y, [this, newDiff] {
        diff = newDiff;
        ai = GoAI(diff);
        gui_reset();
        if (mode == GameMode::PVE && game.side_to_move() == Stone::WHITE)
            gui_start_ai();
        activeModal = Modal::None;
    }, {panelW - 2*pad, 32.f});
    y += 40.f;

    // 3) Cancel
    makeBtn("Cancel", xLeft, y, [this]{ activeModal = Modal::None; },
            {panelW - 2*pad, 32.f});
    y += 40.f;

    modalPanelRect.size.y = (y + pad) - panelY;

	center_modal_vertically();
}

void UI::gui_apply_board_size(int newN) {
    if (newN != 9 && newN != 13 && newN != 19) return;
    BOARD_SIZE = newN;
	gui_update_window_size();
    gui_reset(); // Reset game
    int gridW = MARGIN * 2 + CELL * (BOARD_SIZE - 1);
    build_main_buttons(gridW);
    activeModal = Modal::None;
}

void UI::build_board_size_modal(int gridW) {
    activeModal = Modal::BoardSize;
    modalButtons.clear();

    const float panelW = 260.f;
    const float pad    = 14.f;

    auto vr   = view_rect();
    float panelX = vr.position.x + vr.size.x - panelW - 20.f;
    float panelY = vr.position.y + 160.f;
	
    modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, 0.f});

    auto makeBtn = [&](const std::string& txt, float x, float y,
                       std::function<void()> fn, sf::Vector2f size) {
        Button b;
        b.rect.setSize(size);
        b.rect.setFillColor(sf::Color(240,220,150));
        b.rect.setOutlineColor(sf::Color::Black);
        b.rect.setOutlineThickness(1.f);
        b.rect.setPosition({x, y});
        b.label.emplace(font);
        b.label->setString(txt);
        b.label->setCharacterSize(16);
        b.label->setFillColor(sf::Color::Black);
        auto lb = b.label->getLocalBounds();
        b.label->setPosition({ x + 10.f, y + (size.y - lb.size.y) * 0.5f - lb.position.y });
        b.onClick = std::move(fn);
        modalButtons.push_back(std::move(b));
    };

    const float xLeft = panelX + pad;
    float y = panelY + 40.f;

    auto pick = [&](int targetN){
        if (board_has_any_stone()) {
            build_confirm_resize_modal(targetN, gridW);
        } else {
            gui_apply_board_size(targetN);
        }
    };

    makeBtn("9 x 9",   xLeft, y, [this, pick]{ pick(9);   }, {panelW - 2*pad, 32.f}); y += 40.f;
    makeBtn("13 x 13", xLeft, y, [this, pick]{ pick(13);  }, {panelW - 2*pad, 32.f}); y += 40.f;
    makeBtn("19 x 19", xLeft, y, [this, pick]{ pick(19);  }, {panelW - 2*pad, 32.f}); y += 48.f;

    makeBtn("Cancel",  xLeft, y, [this]{ activeModal = Modal::None; }, {panelW - 2*pad, 32.f});
    y += 40.f;

    modalPanelRect.size.y = (y + pad) - panelY;

	center_modal_vertically();
}

void UI::build_confirm_resize_modal(int newN, int gridW) {
    activeModal = Modal::ConfirmResize;
    modalButtons.clear();

    const float panelW = 260.f;
    const float pad    = 14.f;

    auto vr   = view_rect();
    float panelX = vr.position.x + vr.size.x - panelW - 20.f;
    float panelY = vr.position.y + 160.f;
	
    modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, 0.f});

    auto makeBtn = [&](const std::string& txt, float x, float y,
                       std::function<void()> fn, sf::Vector2f size) {
        Button b;
        b.rect.setSize(size);
        b.rect.setFillColor(sf::Color(240,220,150));
        b.rect.setOutlineColor(sf::Color::Black);
        b.rect.setOutlineThickness(1.f);
        b.rect.setPosition({x, y});
        b.label.emplace(font);
        b.label->setString(txt);
        b.label->setCharacterSize(16);
        b.label->setFillColor(sf::Color::Black);
        auto lb = b.label->getLocalBounds();
        b.label->setPosition({ x + 10.f, y + (size.y - lb.size.y) * 0.5f - lb.position.y });
        b.onClick = std::move(fn);
        modalButtons.push_back(std::move(b));
    };

    const float xLeft = panelX + pad;
    float y = panelY + 40.f;

    // Save & Resize
    makeBtn("Save & Resize", xLeft, y, [this, newN, gridW]{
        deferredAction = [this, newN]{
            gui_apply_board_size(newN);
            activeModal = Modal::None;
        };
        build_save_load_modal(Modal::Save, gridW);
    }, {panelW - 2*pad, 32.f}); y += 40.f;

    // Resize (Don't Save)
    makeBtn("Resize (Don't Save)", xLeft, y, [this, newN]{
        gui_apply_board_size(newN);
        activeModal = Modal::None;
    }, {panelW - 2*pad, 32.f}); y += 40.f;

    // Cancel
    makeBtn("Cancel", xLeft, y, [this]{ activeModal = Modal::None; },
            {panelW - 2*pad, 32.f}); y += 40.f;

    modalPanelRect.size.y = (y + pad) - panelY;

	center_modal_vertically();
}

void UI::build_confirm_newgame_modal(int gridW) {
    activeModal = Modal::ConfirmNewGame;
    modalButtons.clear();

    const float panelW = 260.f;
    const float pad    = 14.f;

    auto vr   = view_rect();
    float panelX = vr.position.x + vr.size.x - panelW - 20.f;
    float panelY = vr.position.y + 160.f;
	
    modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, 0.f});

    auto makeBtn = [&](const std::string& txt, float x, float y,
                       std::function<void()> fn, sf::Vector2f size) {
        Button b;
        b.rect.setSize(size);
        b.rect.setFillColor(sf::Color(240,220,150));
        b.rect.setOutlineColor(sf::Color::Black);
        b.rect.setOutlineThickness(1.f);
        b.rect.setPosition({x, y});

        b.label.emplace(font);
        b.label->setString(txt);
        b.label->setCharacterSize(16);
        b.label->setFillColor(sf::Color::Black);
        auto lb = b.label->getLocalBounds();
        b.label->setPosition({ x + 10.f, y + (size.y - lb.size.y) * 0.5f - lb.position.y });

        b.onClick = std::move(fn);
        modalButtons.push_back(std::move(b));
    };

    const float xLeft = panelX + pad;
    float y = panelY + 40.f; // Below Title

    // 1) Save & New Game
    makeBtn("Save & New Game", xLeft, y, [this, gridW]{
        deferredAction = [this]{
            gui_reset();
            activeModal = Modal::None;
        };
        build_save_load_modal(Modal::Save, gridW);
    }, {panelW - 2*pad, 32.f});
    y += 40.f;

    // 2) New Game (Don't Save)
    makeBtn("New (Don't Save)", xLeft, y, [this]{
        gui_reset();
        activeModal = Modal::None;
    }, {panelW - 2*pad, 32.f});
    y += 40.f;

    // 3) Cancel
    makeBtn("Cancel", xLeft, y, [this]{ activeModal = Modal::None; },
            {panelW - 2*pad, 32.f});
    y += 40.f;

    // Update panel height
    modalPanelRect.size.y = (y + pad) - panelY;

	center_modal_vertically();
}

void UI::build_confirm_quit_modal() {
    activeModal = Modal::ConfirmQuit;
    modalButtons.clear();

    const float panelW = 260.f;
    const float pad    = 14.f;

    auto vr   = view_rect();
    float panelX = vr.position.x + vr.size.x - panelW - 20.f;
    float panelY = vr.position.y + 160.f;

    modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, 0.f});

    auto makeBtn = [&](const std::string& txt, float x, float y,
                       std::function<void()> fn, sf::Vector2f size) {
        Button b;
        b.rect.setSize(size);
        b.rect.setFillColor(sf::Color(240,220,150));
        b.rect.setOutlineColor(sf::Color::Black);
        b.rect.setOutlineThickness(1.f);
        b.rect.setPosition({x, y});

        b.label.emplace(font);
        b.label->setString(txt);
        b.label->setCharacterSize(16);
        b.label->setFillColor(sf::Color::Black);
        auto lb = b.label->getLocalBounds();
        b.label->setPosition({ x + 10.f, y + (size.y - lb.size.y) * 0.5f - lb.position.y });

        b.onClick = std::move(fn);
        modalButtons.push_back(std::move(b));
    };

    const float xLeft = panelX + pad;
    float y = panelY + 40.f; // Below title

    // Save and Quit: autosave then exit
    makeBtn("Save & Quit", xLeft, y, [this]{
        fs::create_directories("saves");
        std::ofstream f("saves/autosave.sav", std::ios::binary);
        if (f) f << game.serialize();
        window.close();
    }, {panelW - 2*pad, 32.f});
    y += 40.f;

    // Quit (Don't Save)
    makeBtn("Quit (Don't Save)", xLeft, y, [this]{
        window.close();
    }, {panelW - 2*pad, 32.f});
    y += 40.f;

    // Cancel
    makeBtn("Cancel", xLeft, y, [this]{
        activeModal = Modal::None;
    }, {panelW - 2*pad, 32.f});
    y += 40.f;

    modalPanelRect.size.y = (y + pad) - panelY;
    center_modal_vertically();
}

// Music picker
void UI::scan_music_folder() {
	musicFiles.clear();
	try {
		if (!fs::exists("assets/music")) return;
		for (auto& p : fs::directory_iterator("assets/music")) {
			if (!p.is_regular_file()) continue;
			auto ext = p.path().extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			if (ext == ".ogg" || ext == ".wav" || ext == ".flac")
				musicFiles.push_back(p.path().string());
		}
		std::sort(musicFiles.begin(), musicFiles.end());
	} catch (...) { /* ignore errors if have */ }
}

void UI::play_music_at(int idx) {
	if (idx < 0 || idx >= (int)musicFiles.size()) return;

	if (bgm.getStatus() == sf::SoundSource::Status::Playing)
		bgm.stop();
	
	hasMusic = bgm.openFromFile(musicFiles[idx]);
	if (hasMusic) {
		bgm.setLooping(false);
		bgm.play();
		musicIdx = idx;

		// "Now playing"
		if (trackText) {
			// Fade out then change text
			trackFadeTarget = 0.f;
			trackFadeClock.restart();

			// Small delay to change text after fade out
			sf::Clock delayClock;
			while (delayClock.getElapsedTime().asSeconds() < 0.15f) {
				sf::sleep(sf::milliseconds(10));
			}

			// Change song's name
			auto name = fs::path(musicFiles[idx]).filename().string();
			trackText->setString("Now playing: " + name);

			// Fade in again
			trackFadeTarget = 255.f;
			trackFadeClock.restart();
		}
	}
}

// Helpers for Volume
void UI::set_volume(float v) {
	volume = std::clamp(v, 0.f, 100.f);
	bgm.setVolume(volume);

	if (volBounds.size.x > 0.f) {
		float t = volume / 100.f;

		volFill.setSize({ volBounds.size.x * t, volBounds.size.y });
		volFill.setPosition({ volBounds.position.x, volBounds.position.y });
		float cx = volBounds.position.x + volBounds.size.x * t;
		float cy = volBounds.position.y + volBounds.size.y * 0.5f;
		volThumb.setPosition({ cx, cy });
		volThumb.setFillColor(sf::Color(60, 60, 60));
		volThumb.setOutlineColor(sf::Color::Black);
		volThumb.setOutlineThickness(1.f);
	}

	if (volLabel) volLabel->setString("Volume: " + std::to_string(int(std::round(volume))) + "%");
	for (auto& s : placeSnds) s.setVolume(volume);
}

void UI::play_place_sound() {
    if (placeSnds.empty()) return;

    for (int k = 0; k < PLACE_VOICES; ++k) {
        placeVoiceIdx = (placeVoiceIdx + 1) % PLACE_VOICES;
        if (placeSnds[placeVoiceIdx].getStatus() != sf::SoundSource::Status::Playing) {
            placeSnds[placeVoiceIdx].play();
            return;
        }
    }
    placeVoiceIdx = (placeVoiceIdx + 1) % PLACE_VOICES;
    placeSnds[placeVoiceIdx].stop();
    placeSnds[placeVoiceIdx].play();
}

void UI::layout_volume_slider(float x, float y, float w) {
    // Thumb
    volThumb.setRadius(volThumbRadius);
    volThumb.setOrigin({ volThumbRadius, volThumbRadius });

    // Track
    volTrack.setSize({ w, 6.f });
    volTrack.setFillColor(sf::Color(220, 210, 170));
    volTrack.setOutlineColor(sf::Color::Black);
    volTrack.setOutlineThickness(1.f);
    volTrack.setPosition({ x, y });

    // Bounds 
    volBounds = sf::FloatRect({ x, y }, { w, 6.f });

    // Update fill + thumb
    set_volume(volume);
}

float UI::volume_from_x(float mx) const {
	if (volBounds.size.x <= 0) return volume;
	float t = (mx - volBounds.position.x) / volBounds.size.x;
	t = std::clamp(t, 0.f, 1.f);
	return t * 100.f;
}

void UI::build_music_modal(int gridW) {
	activeModal = Modal::Music;
	modalButtons.clear();

    const float panelW = 260.f;
    const float pad    = 14.f;

    auto vr   = view_rect();
    float panelX = vr.position.x + vr.size.x - panelW - 20.f;
    float panelY = vr.position.y + 160.f;
	
    modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, 0.f});

	// Default argument
	auto makeBtn = [&](const std::string& txt, float x, float y,
					   std::function<void()> fn,
					   sf::Vector2f size)
	{
		Button b;
		b.rect.setSize(size);
		b.rect.setFillColor(sf::Color(240,220,150));
		b.rect.setOutlineColor(sf::Color::Black);
		b.rect.setOutlineThickness(1.f);
		b.rect.setPosition({x, y});

		b.label.emplace(font);
		b.label->setString(txt);
		b.label->setCharacterSize(16);
		b.label->setFillColor(sf::Color::Black);
		auto lb = b.label->getLocalBounds();
		b.label->setPosition({
			x + 10.f,
			y + (size.y - lb.size.y) * 0.5f - lb.position.y
		});

		b.onClick = std::move(fn);
		modalButtons.push_back(std::move(b));
	};

	const float xLeft = panelX + pad;
	float y = panelY + 40.f; // Below title

	// 1) Refresh
	makeBtn("Refresh list", xLeft, y, [this, gridW]{
		scan_music_folder();
		build_music_modal(gridW);
	}, {panelW - 2*pad, 32.f});
	y+= 40.f;

	// 2) Stop
	makeBtn("Stop", xLeft, y,[this, gridW]{
		if (bgm.getStatus() == sf::SoundSource::Status::Playing) bgm.stop();
		musicIdx = -1;
		if (trackText) trackText->setString("Now playing: (stopped)");
		build_music_modal(gridW);
	}, {panelW - 2*pad, 32.f});
	y += 50.f;

	// 3) Slider
	const float sx = xLeft + 6.f;
	const float sw = panelW - 2*pad - 12.f;

	if (volLabel) {
		volLabel->setPosition({ sx, y });
		auto lb = volLabel->getLocalBounds();
		y += (lb.size.y - lb.position.y) + 20.f;
	}

	layout_volume_slider(sx, y, sw);

	y += 6.f + 16.f;

	// 4) Mute/ max
	const float gap   = 10.f;
	const float rowW    = panelW - 2*pad;
	const float halfW   = (rowW - gap) * 0.5f;
	makeBtn("Mute", xLeft, y, [this, gridW]{ set_volume(0.f); build_music_modal(gridW); }, {halfW, 28.f});
	makeBtn("Max", xLeft + halfW + gap, y, [this, gridW]{ set_volume(100.f); build_music_modal(gridW); }, {halfW, 28.f});
	y += 28.f + 12.f;

	// 5) Music list
	const float listTopY = y;
	float       listHeight;

	// List height calculating base on view
	const float bottomPad   = pad + 14.f + 32.f + pad;
	const float usedAbove   = listTopY - panelY;
	float avail = (vr.position.y + vr.size.y) - (panelY + usedAbove) - bottomPad;
	// Limit
	listHeight = std::clamp(avail, 80.f, 260.f);

	const float rowH   = 32.f;
	const float rowGap = 2.f;

	musicListRect = sf::FloatRect({xLeft, listTopY}, {panelW - 2*pad, listHeight});

	const int total = (int)musicFiles.size();
	const float contentH = total * (rowH + rowGap);
	musicScrollMax = std::max(0.f, contentH - listHeight);
	musicScroll    = std::clamp(musicScroll, 0.f, musicScrollMax);

	float yy = listTopY - musicScroll;
	for (int i = 0; i < total; ++i) {
		if (yy + rowH < listTopY) { yy += rowH + rowGap; continue; }
		if (yy > listTopY + listHeight) break;

		auto name = fs::path(musicFiles[i]).filename().string();
		if (i == musicIdx) name += " (playing)";
		makeBtn(name, xLeft, yy, [this, i, gridW]{
			play_music_at(i);
			build_music_modal(gridW);
		}, {panelW - 2*pad, rowH});

		yy += rowH + rowGap;
	}

	// 6) Cancel
	float cancelY = listTopY + listHeight + 14.f;
	makeBtn("Cancel", xLeft, cancelY, [this]{ activeModal = Modal::None; }, {panelW - 2*pad, 32.f});

	// 7) Update panel size
	float panelBottom = cancelY + 32.f + pad;
	modalPanelRect = sf::FloatRect({panelX, panelY}, {panelW, panelBottom - panelY});

	center_modal_vertically();
}

void UI::center_modal_vertically() {
    if (activeModal == Modal::None) return;
    if (modalPanelRect.size.y <= 0.f) return;

    auto vr  = view_rect();
    float winH = vr.size.y;

    float wantY = std::max(12.f, vr.position.y + (winH - modalPanelRect.size.y) * 0.5f);

    float bottomLimit = vr.position.y + winH - 12.f;
    if (wantY + modalPanelRect.size.y > bottomLimit) {
        wantY = bottomLimit - modalPanelRect.size.y;
    }

    float dy = wantY - modalPanelRect.position.y;

    modalPanelRect.position.y += dy;
    for (auto& b : modalButtons) {
        auto p = b.rect.getPosition();
        b.rect.setPosition({ p.x, p.y + dy });
        if (b.label) {
            auto lp = b.label->getPosition();
            b.label->setPosition({ lp.x, lp.y + dy });
        }
    }
    if (activeModal == Modal::Music) {
        if (volLabel) { auto p = volLabel->getPosition(); volLabel->setPosition({ p.x, p.y + dy }); }
        { auto p = volTrack.getPosition(); volTrack.setPosition({ p.x, p.y + dy }); }
        { auto p = volFill .getPosition(); volFill .setPosition({ p.x, p.y + dy }); }
        { auto p = volThumb.getPosition(); volThumb.setPosition({ p.x, p.y + dy }); }
        volBounds.position.y     += dy;
        musicListRect.position.y += dy;
    }
}

sf::FloatRect UI::make_letterbox(sf::Vector2u win, sf::Vector2u base) {
    float winAspect  = win.x  / float(win.y ? win.y : 1);
    float baseAspect = base.x / float(base.y ? base.y : 1);

    if (winAspect > baseAspect) {
        float width = baseAspect / winAspect;
        float left  = (1.f - width) * 0.5f;
        return sf::FloatRect({ left, 0.f }, { width, 1.f });
    } else {
        float height = winAspect / baseAspect;
        float top    = (1.f - height) * 0.5f;
        return sf::FloatRect({ 0.f, top }, { 1.f, height });
    }
}

void UI::on_window_resized(sf::Vector2u ns) {
    if (suppressResize) return;

    // Locking ratio
    if (lockAspect) {
        const float R = baseWindow.x / float(baseWindow.y ? baseWindow.y : 1);
        unsigned reqW = ns.x, reqH = ns.y;
        if (lastWinSize.x == 0 || lastWinSize.y == 0) lastWinSize = window.getSize();

        int dW = std::abs(int(reqW) - int(lastWinSize.x));
        int dH = std::abs(int(reqH) - int(lastWinSize.y));
        bool widthLed = (dW >= dH);

        unsigned w = reqW, h = reqH;
        if (widthLed) {
            h = (unsigned)std::lround(w / R);
        } else {
            w = (unsigned)std::lround(h * R);
        }

        // Ratio
        if (w != reqW || h != reqH) {
            suppressResize = true;
            window.setSize({ w, h });
            suppressResize = false;
            ns = { w, h };
        }
        lastWinSize = ns;
    }

    // Viewport letterbox
    logicalView.setViewport(make_letterbox(ns, baseWindow));
    window.setView(logicalView);

    // Smoother
    if (resizeClock.getElapsedTime().asMilliseconds() >= 16) {
        int gridW = MARGIN * 2 + CELL * (BOARD_SIZE - 1);
        build_main_buttons(gridW);
        center_modal_vertically();
        resizeClock.restart();
    }
}

void UI::sync_view_to_window() {
    baseWindow = window.getSize();
    logicalView.setSize({ float(baseWindow.x), float(baseWindow.y) });
    logicalView.setCenter({ float(baseWindow.x)*0.5f, float(baseWindow.y)*0.5f });
    logicalView.setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
    window.setView(logicalView);
}

void UI::gui_handle_events() {
	while (const auto event = window.pollEvent()) {

		// 1) Close window
		if (event->is<sf::Event::Closed>()) {
			build_confirm_quit_modal();
		}

		else if (const auto* rz = event->getIf<sf::Event::Resized>()) {
			on_window_resized(rz->size);
		}

		// 2) Left-mouse click
		else if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
			if (mb->button == sf::Mouse::Button::Left) {
				sf::Vector2f mp = to_world({ mb->position.x, mb->position.y });

				// If modal is opening, prioritize modal
				if (activeModal != Modal::None) {

					// Music modal: Volume slider
					if (activeModal == Modal::Music) {
						// Start
						if (volBounds.contains(mp)) {
							volDragging = true;
							set_volume(volume_from_x(mp.x));
							continue; // Stop considering button
						}
						// Click near round button
						float dx = mp.x - volThumb.getPosition().x;
						float dy = mp.y - volThumb.getPosition().y;
						if (dx*dx + dy*dy <= (volThumbRadius + 4) * (volThumbRadius + 4)) {
							volDragging = true;
							continue;
						}
					}

					// Click buttons in modal (Save/ Load/ Theme/ Music)
					bool handled = false;
					for (auto& b : modalButtons) {
						if (b.rect.getGlobalBounds().contains(mp) && b.onClick) {
							b.onClick();
							handled = true;
							break;
						}
					}
					if (handled) continue;

					if (activeModal != Modal::ConfirmQuit) {
						activeModal = Modal::None;
					}
					continue;
				}

				// If not open modal: Handle right modal + stone
				// 2a) Click right button (panel)
				for (auto& b : buttons) {
					if (b.rect.getGlobalBounds().contains(mp) && b.onClick) {
						b.onClick();
						goto after_mouse_press; // Fast exit MousePressed branch
					}
				}

				// 2b) Put stone on board if not AI turn
				if (!aiThinking) {
					int gx = int(std::round((mp.x - float(MARGIN)) / float(CELL)));
					int gy = int(std::round((mp.y - float(MARGIN)) / float(CELL)));
					if (gx>=0 && gy>=0 && gx<BOARD_SIZE && gy<BOARD_SIZE) {
						Move m{gy, gx, false}; // r=y, c=x
						if (game.legal(m) && game.play(m)) {
							play_place_sound();
							lastMove = sf::Vector2i(gx, gy);
							if (mode==GameMode::PVE && game.side_to_move()==Stone::WHITE) {
								gui_start_ai();
							}
						}
					}
				}
			}
		after_mouse_press:;
		}

		// 3) Mouse scrolling (Only if Music modal is opening)
		else if (const auto* mw = event->getIf<sf::Event::MouseWheelScrolled>()) {
			if (activeModal == Modal::Music) {
				// 24px each time
				float step = 24.f;
				if (mw->delta > 0.f) musicScroll -= step;
				else if (mw->delta < 0.f) musicScroll += step;
				musicScroll = std::clamp(musicScroll, 0.f, musicScrollMax);

				int gridW = MARGIN*2 + CELL*(BOARD_SIZE-1);
				build_music_modal(gridW);
			}
			else {
				sf::Vector2f mp = to_world({ mw->position.x, mw->position.y });
				if (panelViewport.contains(mp)) {
					float step = 24.f * (mw->delta > 0 ? -1.f : 1.f);
					panelScroll = std::clamp(panelScroll + step, 0.f, panelScrollMax);
					int gridW = MARGIN*2 + CELL*(BOARD_SIZE-1);
					build_main_buttons(gridW);
				}
			}
		}

		// 4) Mouse Scrolling (Update volume slider while scrolling in Music modal)
		else if (const auto* mm = event->getIf<sf::Event::MouseMoved>()) {
			if (activeModal == Modal::Music && volDragging) {
				sf::Vector2f mp = to_world({ mm->position.x, mm->position.y });
				set_volume(volume_from_x(mp.x));
			}
		}

		// 5) End scrolling
		else if (event->is<sf::Event::MouseButtonReleased>()) {
			if (activeModal == Modal::Music) volDragging = false;
		}

		// 6) Click button
		else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
			using sc = sf::Keyboard::Scancode;

			// Esc: Close modal if opening
			if (key->scancode == sc::Escape) {
				if (activeModal != Modal::None) { activeModal = Modal::None; continue; }
			}

			if (key->scancode == sc::Z) {
				if (!aiThinking) { game.undo(); lastMove.reset(); }
			}
			else if (key->scancode == sc::Y) {
				if (!aiThinking) { game.redo(); lastMove.reset(); }
			}
			else if (key->scancode == sc::Space) {
				if (!aiThinking) {
					game.pass(); lastMove.reset();
					if (mode==GameMode::PVE && game.side_to_move()==Stone::WHITE) gui_start_ai();
				}
			}
			else if (key->scancode == sc::N) {
				gui_reset();
			}
			else if (key->scancode == sc::M) {
				int gridW = MARGIN*2 + CELL*(BOARD_SIZE-1);
				request_switch_mode(gridW);
			}
			else if (key->scancode == sf::Keyboard::Scancode::C) {
				int gridW = MARGIN*2 + CELL*(BOARD_SIZE-1);
				build_theme_modal(gridW);
			}

			else if (key->scancode == sc::Num1) {
				int gridW = MARGIN*2 + CELL*(BOARD_SIZE-1);
				build_confirm_diff_modal(AIDifficulty::EASY, gridW);
			}
			else if (key->scancode == sc::Num2) {
				int gridW = MARGIN*2 + CELL*(BOARD_SIZE-1);
				build_confirm_diff_modal(AIDifficulty::MEDIUM, gridW);
			}
			else if (key->scancode == sc::Num3) {
				int gridW = MARGIN*2 + CELL*(BOARD_SIZE-1);
				build_confirm_diff_modal(AIDifficulty::HARD, gridW);
			}

			// Background music: Turn on/ Pause
			else if (key->scancode == sc::V) {
				if (hasMusic) {
					if (bgm.getStatus() == sf::SoundSource::Status::Playing) bgm.pause();
					else bgm.play();
				}
			}

			// Volume shortcuts (SFML 3)
			else if (key->scancode == sc::LBracket) {
				set_volume(std::max(0.f, volume - 5.f));
			}
			else if (key->scancode == sc::RBracket) {
				set_volume(std::min(100.f, volume + 5.f));
			}
		}
	}
}

void UI::gui_update() {
	gui_poll_ai();
	update_hover_state();
	// Auto next track
	if (hasMusic
		&& musicIdx >= 0
		&& bgm.getStatus() == sf::SoundSource::Status::Stopped
		&& !musicFiles.empty())
	{
		int next = (musicIdx + 1) % (int)musicFiles.size();
		play_music_at(next);
	}

	// Update fade for song
	float dt = trackFadeClock.restart().asSeconds();
	if (trackAlpha != trackFadeTarget) {
		float dir = (trackFadeTarget > trackAlpha) ? 1.f : -1.f;
		trackAlpha += dir * trackFadeSpeed * dt;
		trackAlpha = std::clamp(trackAlpha, 0.f, 255.f);

		if (trackText) {
			auto c = trackText->getFillColor();
			c.a = static_cast<std::uint8_t>(trackAlpha);
			trackText->setFillColor(c);
		}
	}
}

void UI::update_hover_state() {
	// Take mouse's position (relative)
	auto p = sf::Mouse::getPosition(window);
	mousePos = to_world({ p.x, p.y });

	auto refresh = [&](std::vector<Button>& arr){
		for (auto& b : arr) {
			b.hovered = b.rect.getGlobalBounds().contains(mousePos);
		}
	};

	if (activeModal != Modal::None) refresh(modalButtons);
	else							refresh(buttons);
}

bool UI::board_has_any_stone() const {
    for (int y = 0; y < BOARD_SIZE; ++y)
        for (int x = 0; x < BOARD_SIZE; ++x)
            if (game.board().get(y, x) != Stone::EMPTY)
                return true;
    return false;
}

sf::Vector2u UI::compute_window_px() const {
    const int gridW = MARGIN * 2 + CELL * (BOARD_SIZE - 1);
    const int gridH = MARGIN * 2 + CELL * (BOARD_SIZE - 1);
    const unsigned winW = static_cast<unsigned>(gridW + PANEL_GAP + PANEL_W + RIGHT_PAD);

    unsigned panelMin = static_cast<unsigned>(std::ceil(panel_min_height()));
    unsigned wantH = std::max<unsigned>(gridH, panelMin);

    // Limit the height
    auto desk = sf::VideoMode::getDesktopMode();
    unsigned safe = 96; // Changeable
    unsigned maxH = (desk.size.y > safe ? desk.size.y - safe : desk.size.y);

    unsigned winH = std::min<unsigned>(wantH, std::max<unsigned>(MIN_WIN_H, maxH));
    return { winW, winH };
}

void UI::gui_update_window_size() {
    auto size = compute_window_px();
    // Change window size
    window.setSize(size);
	sync_view_to_window();
    // Update view
	// sf::View v;
    /* v.setSize(sf::Vector2f(static_cast<float>(size.x), static_cast<float>(size.y)));
    v.setCenter(sf::Vector2f(static_cast<float>(size.x) * 0.5f,
                             static_cast<float>(size.y) * 0.5f));
	window.setView(v); */
}

void UI::gui_render() {
	window.clear(theme.border);
	draw_board();
	draw_stones();
	draw_hud();
	draw_modal();
	window.display();
}

void UI::gui_start_ai() {
	if (aiThinking) return;
	ai = GoAI(diff);
	Game snapshot = game;
	aiThinking = true;
	aiFuture = std::async(std::launch::async, [this, snapshot]() mutable {
		return ai.choose_move(snapshot, rng);
	});
}

void UI::gui_poll_ai() {
	if (!aiThinking) return;
	if (aiFuture.wait_for(0ms) == std::future_status::ready) {
		aiThinking = false;
		Move mv = aiFuture.get();
		if (!mv.is_pass && game.legal(mv)) {
			game.play(mv);
			lastMove = sf::Vector2i(mv.c, mv.r);
			play_place_sound();
		} else {
			game.pass();
			lastMove.reset();
		}
	}
}

void UI::gui_reset() {
	aiThinking = false;
	lastMove.reset();
	game = Game(BOARD_SIZE);
}

void UI::gui_change_theme(int idx) {
	themeIdx = idx % int(themes.size());
	theme = themes[themeIdx];
}

void UI::draw_board() {
	sf::RectangleShape inner({float(CELL*(BOARD_SIZE-1)), float(CELL*(BOARD_SIZE-1))});
	inner.setFillColor(theme.board);
	inner.setPosition({float(MARGIN), float(MARGIN)});
	sf::RectangleShape frame({inner.getSize().x + 2*theme.borderThickness,
							  inner.getSize().y + 2*theme.borderThickness});
	frame.setFillColor(theme.border);
	frame.setPosition({float(MARGIN - theme.borderThickness), float(MARGIN - theme.borderThickness)});
	window.draw(frame);
	window.draw(inner);

	sf::VertexArray lines(sf::PrimitiveType::Lines);
	auto addLine = [&](sf::Vector2f a, sf::Vector2f b){
		sf::Vertex va; va.position=a; va.color=theme.grid;
		sf::Vertex vb; vb.position=b; vb.color=theme.grid;
		lines.append(va); lines.append(vb);
	};
	for (int i=0; i<BOARD_SIZE; ++i){
		addLine(gridToPixel(0,i,CELL,MARGIN), gridToPixel(BOARD_SIZE-1,i,CELL,MARGIN));
		addLine(gridToPixel(i,0,CELL,MARGIN), gridToPixel(i,BOARD_SIZE-1,CELL,MARGIN));
	}
	window.draw(lines);

	for (auto p : hoshi_points(BOARD_SIZE)) {
		sf::CircleShape dot(3.0f);
		dot.setFillColor(theme.star);
		dot.setOrigin({3.f,3.f});
		dot.setPosition(gridToPixel(p.x, p.y, CELL, MARGIN));
		window.draw(dot);
	}

	const std::string cols = "ABCDEFGHJKLMNOPQRST";
	sf::Text lbl(font);
	lbl.setCharacterSize(14);
	lbl.setFillColor(sf::Color::Black);

	for (int i=0; i<BOARD_SIZE; ++i) {
		lbl.setString(std::to_string(BOARD_SIZE - i));
		float y = MARGIN + i * CELL - 8;
		lbl.setPosition({float(MARGIN - 25), y});
		window.draw(lbl);
		lbl.setPosition({float(MARGIN + CELL * (BOARD_SIZE - 1) + 10), y});
		window.draw(lbl);
	}

	for (int i = 0; i < BOARD_SIZE; ++i) {
		char ch = cols[i];
		lbl.setString(std::string(1, ch));
		float x = MARGIN + i * CELL - 5.f;
		lbl.setPosition({x, float(MARGIN - 24)});
		window.draw(lbl);
		lbl.setPosition({x, float(MARGIN + CELL * (BOARD_SIZE - 1) + 8)});
		window.draw(lbl);
	}
}

void UI::draw_stones() {
    sf::CircleShape specCircle(std::max(6.f, CELL * 0.12f));
    specCircle.setOrigin({ specCircle.getRadius(), specCircle.getRadius() });

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            Stone s = game.board().get(y, x);
            if (s == Stone::EMPTY) continue;

            sf::Vector2f p = gridToPixel(x, y, CELL, MARGIN);

            // Shadow
            sf::CircleShape shadow(CELL * 0.45f);
            shadow.setOrigin({ shadow.getRadius(), shadow.getRadius() });
            shadow.setPosition(p + sf::Vector2f{2.f, 2.f});
            shadow.setFillColor(sf::Color(0, 0, 0, 60));
            window.draw(shadow);

            // Stone
            sf::CircleShape& base = (s == Stone::BLACK ? blackCircle : whiteCircle);
            base.setPosition(p);
            window.draw(base);

            specCircle.setPosition(p + sf::Vector2f{-base.getRadius()*0.35f, -base.getRadius()*0.35f});
            specCircle.setFillColor(sf::Color(255, 255, 255, (s == Stone::BLACK ? 50 : 35)));
            window.draw(specCircle);
        }
    }

    if (lastMove) {
        sf::CircleShape glow(CELL * 0.48f);
        glow.setOrigin({ glow.getRadius(), glow.getRadius() });
        glow.setPosition(gridToPixel(lastMove->x, lastMove->y, CELL, MARGIN));
        glow.setFillColor(sf::Color(255, 240, 0, 40));
        window.draw(glow);
    }
}

void UI::draw_hud() {
	// Now playing
	if (trackText) {
		trackText->setCharacterSize(18);
		trackText->setStyle(sf::Text::Style::Italic);
		trackText->setFillColor(sf::Color(20, 20, 20));
		trackText->setOutlineColor(sf::Color(255, 255, 255, 130));
		trackText->setOutlineThickness(0.8f);

		// Aligned
		float centerX = float(MARGIN + (BOARD_SIZE - 1) * CELL * 0.5f);
		auto lb = trackText->getLocalBounds();
		trackText->setOrigin({ lb.position.x + lb.size.x * 0.5f, 0.f });
		trackText->setPosition({ centerX, 1.f });

		// Background behind text
		sf::RectangleShape ribbon;
		float padX = 10.f, padY = 3.f;
		ribbon.setSize({ lb.size.x + 2*padX, lb.size.y + 2*padY });
		ribbon.setOrigin({ (lb.size.x + 2*padX) * 0.5f, padY });
		ribbon.setPosition(trackText->getPosition());
		ribbon.setFillColor(sf::Color(255, 245, 220, 110));
		ribbon.setOutlineColor(sf::Color(0,0,0,60));
		ribbon.setOutlineThickness(1.f);

		// window.draw(ribbon);
		window.draw(*trackText);
	}

	// Turn
	float panelX = float(MARGIN + (BOARD_SIZE - 1) * CELL + PANEL_GAP);
	float panelY = 10.f;

	if (turnText && turnSideText) {
		// "Turn:"
		turnText->setString("Turn:");
		turnText->setCharacterSize(22);
		turnText->setFillColor(sf::Color(20, 80, 160));
		turnText->setOutlineColor(sf::Color::Black);
		turnText->setOutlineThickness(1.2f);
		turnText->setStyle(sf::Text::Style::Bold);
		turnText->setOrigin({0.f, 0.f});
		turnText->setPosition({panelX, panelY});
		window.draw(*turnText);

		// Black or White
		bool blackTurn = (game.side_to_move() == Stone::BLACK);
		turnSideText->setString(blackTurn ? "Black" : "White");
		turnSideText->setCharacterSize(22);
		if (blackTurn) {
			turnSideText->setFillColor(sf::Color::Black);
			turnSideText->setOutlineColor(sf::Color::White);
		} else {
			turnSideText->setFillColor(sf::Color::White);
			turnSideText->setOutlineColor(sf::Color::Black);
		}
		turnSideText->setOutlineThickness(1.2f);
		turnSideText->setStyle(sf::Text::Style::Bold);

		auto a = turnText->getGlobalBounds();
		turnSideText->setPosition({ a.position.x + a.size.x + 4.f, panelY });
		window.draw(*turnSideText);

		//
		panelY = a.position.y + a.size.y + 8.f;
	} else {
		panelY = 50.f; // Fallback if text is missing
	}

	// Info (below turn)
	if (infoText) {
		auto modeStr = (mode==GameMode::PVP ? "PVP" : "PVE");
		auto diffStr = (diff==AIDifficulty::HARD ? "Hard" : diff==AIDifficulty::MEDIUM ? "Medium" : "Easy");
		infoText->setString(
			std::string("Mode (M): ") + modeStr +
			"\nAI (1/2/3): " + diffStr +
			"\nTheme (C): change" +
			"\nMusic (V): play/pause"
		);
		infoText->setCharacterSize(18);
		infoText->setFillColor(sf::Color::Black);
		infoText->setPosition({ panelX, panelY });
		window.draw(*infoText);

		auto ib = infoText->getGlobalBounds();
		panelY = ib.position.y + ib.size.y + 14.f;
	}

	// Buttons (hover lighter)
	for (auto& b : buttons) {
		b.rect.setFillColor(b.hovered ? sf::Color(235,220,160) : sf::Color(220,200,120));
		window.draw(b.rect);
		if (b.label) window.draw(*b.label);
	}

	// AI Thinking
	if (aiThinking && thinkingText)
		window.draw(*thinkingText);
}

void UI::draw_modal() {
    if (activeModal == Modal::None) return;

    auto vr = view_rect();

    // Dim
    sf::RectangleShape dim({ vr.size.x, vr.size.y });
    dim.setPosition({ vr.position.x, vr.position.y });
    dim.setFillColor(sf::Color(0,0,0,80));
    window.draw(dim);

    // Position + panel size
    float x = modalPanelRect.size.x > 0 ? modalPanelRect.position.x
                                        : (vr.position.x + vr.size.x - 260.f - 20.f);
    float y = modalPanelRect.size.y > 0 ? modalPanelRect.position.y
                                        : (vr.position.y + 160.f);
    float w = modalPanelRect.size.x > 0 ? modalPanelRect.size.x : 260.f;
    float h = modalPanelRect.size.y > 0 ? modalPanelRect.size.y : 320.f;

    // Panel
    sf::RectangleShape panel({w, h});
    panel.setFillColor(sf::Color(255,245,200));
    panel.setOutlineColor(sf::Color::Black);
    panel.setOutlineThickness(2.f);
    panel.setPosition({x, y});
    window.draw(panel);

    // Title
    sf::Text title(font);
    title.setCharacterSize(18);
    title.setFillColor(sf::Color::Black);
    title.setString(
        activeModal==Modal::Save             ? "Save Game"          :
        activeModal==Modal::Load             ? "Load Game"          :
        activeModal==Modal::Theme            ? "Themes"             :
        activeModal==Modal::Music            ? "Music Picker"       :
        activeModal==Modal::BoardSize        ? "Board Size"         :
        activeModal==Modal::ConfirmResize    ? "Change Board Size?" :
        activeModal==Modal::ConfirmDifficulty? "Change Difficulty?" :
        activeModal==Modal::ConfirmOverwrite ? "Overwrite"          :
        activeModal==Modal::ConfirmNewGame   ? "Start New Game?"    :
		activeModal==Modal::ConfirmQuit      ? "Quit Game?"         :
                                               "Switch Mode?"
    );
    title.setPosition({x + 12.f, y + 10.f});
    window.draw(title);

    // UI of Music
    if (activeModal == Modal::Music) {
        if (musicListRect.size.x > 0.f && musicListRect.size.y > 0.f) {
            sf::RectangleShape listBorder;
            listBorder.setSize({ musicListRect.size.x, musicListRect.size.y });
            listBorder.setPosition({ musicListRect.position.x, musicListRect.position.y });
            listBorder.setFillColor(sf::Color(0,0,0,0));
            listBorder.setOutlineColor(sf::Color::Black);
            listBorder.setOutlineThickness(1.f);
            window.draw(listBorder);
        }
        if (volLabel) window.draw(*volLabel);
        window.draw(volTrack);
        window.draw(volFill);
        window.draw(volThumb);
    }

    // Buttons in modal
	for (auto& b : modalButtons) {
		b.rect.setFillColor(b.hovered ? sf::Color(235,220,160) : sf::Color(240,220,150));
		window.draw(b.rect);
		if (b.label) window.draw(*b.label);
	}
}
