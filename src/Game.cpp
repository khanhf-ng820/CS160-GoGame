#include "Game.h"
#include <algorithm>

Game::Game(int n) : N(n), bd(n), to_move(Stone::BLACK) {}

int Game::size() const { return N; }
Board& Game::board() { return bd; }
const Board& Game::board() const { return bd; }
Stone Game::side_to_move() const { return to_move; }

bool Game::is_over() const { return consecutive_passes >= 2; }

Score Game::score() const {
    Score s{}; bd.count(s.black, s.white); return s;
}

void Game::reset() {
    bd.clear(); to_move = Stone::BLACK;
    history.clear(); redo_stack.clear();
    consecutive_passes = 0;
}

bool Game::undo() {
    if (history.empty()) return false;
    Move mv = history.back(); history.pop_back();
    if (mv.is_pass) {
        to_move = opposite(to_move);
        if (consecutive_passes > 0) --consecutive_passes;
    } else {
        bd.set(mv.r, mv.c, Stone::EMPTY);
        to_move = opposite(to_move);
        consecutive_passes = 0;
    }
    redo_stack.push_back(mv);
    return true;
}

bool Game::redo() {
    if (redo_stack.empty()) return false;
    Move mv = redo_stack.back(); redo_stack.pop_back();
    return play(mv);
}

void Game::pass() {
    history.push_back(Move{0,0,true});
    redo_stack.clear();
    ++consecutive_passes;
    to_move = opposite(to_move);
}

bool Game::legal(const Move& m) const {
    if (m.is_pass) return true;
    if (!bd.in_bounds(m.r, m.c)) return false;
    return bd.get(m.r, m.c) == Stone::EMPTY;
}

bool Game::play(const Move& m) {
    if (!legal(m)) return false;
    if (m.is_pass) { pass(); return true; }
    bd.set(m.r, m.c, to_move);
    history.push_back(m);
    redo_stack.clear();
    consecutive_passes = 0;
    to_move = opposite(to_move);
    return true;
}

std::string Game::serialize() const {
    std::ostringstream oss;
    oss << "GOSAVE N=" << N
        << " side=" << (to_move == Stone::BLACK ? 0 : 1)
        << " komi=" << komi
        << " passes=" << consecutive_passes << "\n";
    oss << bd.dump_rows();
    return oss.str();
}

bool Game::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string first;
    if (!std::getline(iss, first)) return false;

    if (first.size() >= 3 &&
        (unsigned char)first[0] == 0xEF &&
        (unsigned char)first[1] == 0xBB &&
        (unsigned char)first[2] == 0xBF)
        first.erase(0, 3);

    auto rstrip_cr = [](std::string& s){
        if (!s.empty() && s.back() == '\r') s.pop_back();
    };

    auto only_rows = [&](std::string head)->bool{
        rstrip_cr(head);
        if ((int)head.size() < N) return false;

        std::vector<std::string> rows; rows.reserve(N);
        rows.push_back(head.substr(0, N));

        while ((int)rows.size() < N) {
            std::string line;
            if (!std::getline(iss, line)) return false;
            rstrip_cr(line);
            if ((int)line.size() < N) continue;
            rows.push_back(line.substr(0, N));
        }

        bd = Board(N);
        to_move = Stone::BLACK;
        consecutive_passes = 0;
        return bd.load_rows(rows);
    };

    if (first.rfind("GOSAVE", 0) == 0) {
        std::string header = trim(first);
        int n = 19, side = 0, passes = 0; double k = 6.5;

        auto take = [&](const std::string& key)->std::optional<std::string>{
            auto pos = header.find(key);
            if (pos == std::string::npos) return std::nullopt;
            pos += key.size();
            size_t e = header.find_first_of(" \t", pos);
            if (e == std::string::npos) e = header.size();
            return header.substr(pos, e - pos);
        };
        if (auto s = take("N="))      n = std::max(1, std::stoi(*s));
        if (auto s = take("side="))   side = std::stoi(*s);
        if (auto s = take("komi="))   k = std::stod(*s);
        if (auto s = take("passes=")) passes = std::max(0, std::stoi(*s));

        N = n; bd = Board(N);
        komi = k;
        to_move = (side == 0 ? Stone::BLACK : Stone::WHITE);
        consecutive_passes = passes;

        std::vector<std::string> rows; rows.reserve(N);
        while ((int)rows.size() < N) {
            std::string line;
            if (!std::getline(iss, line)) return false;
            rstrip_cr(line);
            if ((int)line.size() < N) continue;
            rows.push_back(line.substr(0, N));
        }
        return bd.load_rows(rows);
    } else {
        return only_rows(first);
    }
}

Move Game::parse_move(const std::string& raw, int N) {
    std::string s = trim(raw);
    for (char& ch : s) if (ch >= 'a' && ch <= 'z') ch = char(ch - 'a' + 'A');
    if (s == "PASS" || s == "RESIGN") return {0,0,true};
    if (s.size() < 2) return {0,0,true};
    int c = col_from_char(s[0]);
    int r = std::stoi(s.substr(1)) - 1;
    if (c < 0 || r < 0 || r >= N) return {0,0,true};
    return {r, c, false};
}

std::string Game::render_ascii() const {
    std::ostringstream oss;
    oss << "   ";
    for (int c = 0; c < N; ++c) oss << char_from_col(c) << ' ';
    oss << "\n";

    for (int r = 0; r < N; ++r) {
        int rowLabel = N - r;
        if (rowLabel < 10) oss << ' ';
        oss << rowLabel << ' ';
        for (int c = 0; c < N; ++c)
            oss << stone_char(bd.get(r, c)) << ' ';
        oss << ' ' << rowLabel << '\n';
    }

    oss << "   ";
    for (int c = 0; c < N; ++c) oss << char_from_col(c) << ' ';
    oss << "\n";
    return oss.str();
}
