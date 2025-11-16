#pragma once
#include <functional>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <future>
#include <random>
#include <string>
#include <iostream>

#include "Game.h"
#include "AI.h"

enum class UIType { CONSOLE, GRAPHICAL };

class UI {
	public: 
		UI(Game&g, GoAI& a, std::mt19937& rng);
		void run_console();
		void run_graphical();

	private:
		// Console helpers
		void print_menu() const;
		void handle_command(const std::string& line);
		void ai_turn();

		// GUI
		struct Theme {
			sf::Color board, border, grid, star;
			float borderThickness, gridThickness;
		};

		struct Button {
			sf::RectangleShape rect;
			std::optional<sf::Text> label; // Create only after font is set
			std::function<void()> onClick;
			bool hovered = false; // Hover state
		};

		// Modals
		enum class Modal { None, Save, Load, Theme, Music, ConfirmSwitch, ConfirmDifficulty, ConfirmOverwrite, ConfirmNewGame, BoardSize, ConfirmResize, ConfirmQuit, GameOver, EndGame };

		void gui_handle_events();
		void gui_update();
		void update_hover_state();
		void gui_render();
		void gui_start_ai();
		void gui_poll_ai();
		void gui_reset();
		void gui_change_theme(int idx);

		void build_main_buttons(int gridW);
		void build_save_load_modal(Modal type, int gridW);
		void build_theme_modal(int gridW);

		// Music modal
		void build_music_modal(int gridW);
		void scan_music_folder();
		void play_music_at(int idx);

		// Volume slider
		void set_volume(float v);
		void layout_volume_slider(float x, float y, float w);
		float volume_from_x(float x) const;

		void draw_board();
		void draw_stones();
		void draw_hud();
		void draw_modal();

		void request_switch_mode(int gridW);
  		void build_confirm_switch_modal(int gridW);
		void build_confirm_diff_modal(AIDifficulty newDiff, int gridW);
		void build_confirm_overwrite_modal(int gridW, const std::string& path);
		void gui_update_window_size();
		void center_modal_vertically();
		sf::Vector2u compute_window_px() const;
		float panelScroll = 0.f;
		float panelScrollMax = 0.f;
		sf::FloatRect panelViewport;
		sf::View logicalView{};
		sf::Vector2u baseWindow{}; // Logical size at begin
		bool lockAspect = true;
		bool suppressResize = false;

		void on_window_resized(sf::Vector2u newSize);
		static sf::FloatRect make_letterbox(sf::Vector2u win, sf::Vector2u base);
		inline sf::Vector2f to_world(sf::Vector2i px) const {
			return window.mapPixelToCoords(px); // Current view
		}
		void build_confirm_quit_modal();
		sf::Vector2u lastWinSize{0,0};
		sf::Clock    resizeClock;
		void build_game_over_modal(int gridW);

		// Ultilities
		static inline sf::Vector2f gridToPixel(int x, int y, int CELL, int MARGIN) {
			return { float(MARGIN + x * CELL), float(MARGIN + y * CELL) };
		}
		static std::vector<sf::Vector2i>hoshi_points(int N);

	private:
		// Core
		Game& game;
		GoAI& ai;
		std::mt19937& rng;
		GameMode mode = GameMode::PVP;
		AIDifficulty diff = AIDifficulty::EASY;

		// GUI state
		int BOARD_SIZE = 19;
		int CELL = 40;
		int MARGIN = 50;

		sf::RenderWindow window;
		sf::Font font;
		sf::SoundBuffer placeBuf;
		sf::Music bgm;
		bool hasMusic = false;

		static constexpr int PLACE_VOICES = 8;
		std::vector<sf::Sound> placeSnds;
		int placeVoiceIdx = 0;
		
		void play_place_sound();
    	void build_confirm_newgame_modal(int gridW);
		void build_board_size_modal(int gridW);
    	void build_confirm_resize_modal(int newN, int gridW);
    	void gui_apply_board_size(int newN);
    	bool board_has_any_stone() const;
		void sync_view_to_window();
		sf::FloatRect view_rect() const;

		std::vector<Theme> themes;
		int themeIdx = 0;
		Theme theme { sf::Color(219, 178, 92), sf::Color(185, 147, 77),
					  sf::Color::Black, sf::Color::Black, 18.f, 2.f };

		sf::CircleShape blackCircle{0.f}, whiteCircle{0.f};

		std::optional<sf::Text> turnText, infoText, thinkingText;
		float panel_top_y() const;
		float panel_min_height() const;

		std::optional<sf::Text> turnSideText; // "Black" or "White"
		std::optional<sf::Text> trackText; // Now plaing
		sf::Vector2f            mousePos{0.f, 0.f};

		// Fade effect
		float trackAlpha = 255.f;
		float trackFadeTarget = 255.f; // 0 or 255
		float trackFadeSpeed = 300.f; // Alpha per second
		sf::Clock trackFadeClock;

		// Buttons
		std::vector<Button> buttons; // Right side buttons
		std::vector<Button> modalButtons; // Popup modal buttons
		Modal activeModal = Modal::None;

		// Highlight last move
		std::optional<sf::Vector2i> lastMove;

		// Music list and playback
		std::vector<std::string> musicFiles;
		float musicScroll = 0.f; // Scroll position (px)
		float musicScrollMax = 0.f; // Max scroll (px)
		sf::FloatRect musicListRect{}; // Area of music list

		int musicIdx = -1; // none/ paused

		// Modals layout
		sf::FloatRect modalPanelRect{}; // Dynamic panel rectangle (for draw_modal)
		
		// Volume
		float volume = 100.f;
		bool  volDragging = false;
		sf::RectangleShape volTrack; // Background bar
		sf::RectangleShape volFill; // Filled bar
		sf::CircleShape    volThumb; // Knob
		std::optional<sf::Text> volLabel; // Volume: xx%
		sf::FloatRect      volBounds{}; // Hit-test rect for the track
		float              volThumbRadius = 8.f;

		// AI async
		bool aiThinking = false;
		std::future<Move> aiFuture;

		std::function<void()> deferredAction = nullptr;
};
