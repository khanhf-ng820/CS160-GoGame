/*
GIẢI THÍCH FLOW
class Game:
1) legal(m):
- Nếu m.is_pass thì hợp lệ
- Ngược lại thì kiểm tra in_bounds với ô đang EMPTY
(Mở rộng thêm luật như cấm tự sát,...)

2) play(m):
- Nếu PASS: gọi pass() và return true
- Nếu đánh quân thường:
    +) Đặt quân: bd.set(r,c,to_move)
    +) push vào history với clear redo_stack
    +) reset consecutive_passes = 0
    +) Đổi lượt: to_move = opposite(to_move)
- Trả false nếu m không hợp lệ

3) pass():
- push Move{is_pass=true} vào history với xóa redo_stack
- ++consecutive_passes + đổi lượt
- is_over() -> true khi consecutive_passes ≥ 2

4) undo()/redo():
- undo: pop history -> nếu là PASS thì giảm counter còn nếu là đặt quân thì clear ô
        Sau đó đổi lượt và push move đó sang redo_stack
- redo: lấy lại từ redo_stack và gọi play(m)

5) serialize()/deserialize():
- Đầu tiên là ghi header: "GOSAVE N=.. side=.. komi=.. passes=.."
- deserialize sẽ đọc cả 2 dạng

6) score():
- Dùng bd.count(black, white). Khi quyết định thắng thua, TRẮNG + komi

7) parse_move("D4"):
- Bỏ chữ i/I, chấp nhận lower/upper

LƯU Ý
- Các utilities từ Board.h: trim, col_from_char/char_from_col, stone_char
*/
#include "Game.h"
#include <iostream>
#include <algorithm> // Dùng std::max khi parse header,...

// Khởi tạo N = n, tạo Board NxN rỗng với lượt đi đầu là BLACK
Game::Game(int n) : N(n), bd(n), previousBd(n), to_move(Stone::BLACK), boardHistory(1, Board(n)) {}
// Trả về kích thước bàn
int Game::size() const { return N; }
// Returns the game's komi
double Game::komi() const { return komiPts; }
// Truy cập mutable tới Board (hàm khác const)
Board& Game::board() { return bd; }
// Truy cập const tới Board (hàm const)
const Board& Game::board() const { return bd; }
// Trả về quân đang tới lượt
Stone Game::side_to_move() const { return to_move; }
// Ván kết thúc nếu có ≥ 2 lượt pass liên tiếp
bool Game::is_over() const { return consecutive_passes >= 2; }
// Đếm tổng quân đen/ trắng hiện trên bàn (không tính TERRITORY)
Score Game::score() const {
    // Gọi Board::count để lấy số lượng quân mỗi bên
    Score s{}; bd.count(s.black, s.white); return s;
}

// Đặt lại game về trạng thái ban đầu
void Game::reset() {
    // Game state set to Playing
    gameState = GameState::PLAYING;
    // Xoá bàn, đặt lượt về BLACK
    bd.clear(); to_move = Stone::BLACK;
    // Xoá lịch sử và redo
    boardHistory.clear();
    moveHistory.clear();
    redo_stack.clear();
    // Xoá đếm PASS liên tiếp
    consecutive_passes = 0;
}

// Hoàn tác 1 nước đi (nếu có)
bool Game::undo() {
    // Can't undo if game is ended
    if (gameState == GameState::ENDED) return false;
    // Không có gì để undo
    if (boardHistory.size() <= 1) return false;

    // // Lấy nước cuối cùng khỏi history
    // Move mv = boardHistory.back();
    // boardHistory.pop_back();
    // // Với pass: chỉ đảo lượt lại
    // if (mv.is_pass) {
    //     to_move = opposite(to_move);
    //     // Giảm số pass đã đến liên tiếp nếu > 0
    //     if (consecutive_passes > 0) --consecutive_passes;
    // } else {
    //     // Gỡ quân vừa đặt ở (r, c)
    //     bd.set(mv.r, mv.c, Stone::EMPTY);
    //     // Đảo lượt lại cho bên vừa đi
    //     to_move = opposite(to_move);
    //     // Một nước đặt quân sẽ reset chuỗi pass
    //     consecutive_passes = 0;
    // }
    // // Đưa nước vừa hoàn tác vào redo_stack để có thể redo
    // redo_stack.push_back(mv);
    // return true;

    // Lấy nước cuối cùng khỏi history
    previousBd = boardHistory[boardHistory.size() - 2];
    // Với pass: chỉ đảo lượt lại
    if (previousBd == bd) {
        to_move = opposite(to_move);
        // Giảm số pass đã đến liên tiếp nếu > 0
        if (consecutive_passes > 0) --consecutive_passes;
    } else {
        bd = previousBd;
        // Đảo lượt lại cho bên vừa đi
        to_move = opposite(to_move);
        // Một nước đặt quân sẽ reset chuỗi pass
        consecutive_passes = 0;
    }
    // Đưa nước vừa hoàn tác vào redo_stack để có thể redo
    Board undoneBoard = boardHistory.back();
    boardHistory.pop_back();
    redo_stack.push_back(undoneBoard);
    return true;
}

// Làm lại 1 nước đã undo (nếu có)
bool Game::redo() {
    // Can't redo if game is ended
    if (gameState == GameState::ENDED) return false;
    // Không có gì để redo
    if (redo_stack.empty()) return false;

    // // Lấy nước từ redo_stack và bỏ khỏi stack
    // Move mv = redo_stack.back();
    // redo_stack.pop_back();
    // // Áp dụng lại logic của play để làm lại nước đi
    // return play(mv);

    // Lấy bàn cờ từ redo_stack và bỏ khỏi stack
    Board redoneBoard = redo_stack.back();
    redo_stack.pop_back();

    // Đảo lượt lại cho bên vừa đi
    to_move = opposite(to_move);
    // Một nước đặt quân sẽ reset chuỗi pass
    consecutive_passes = (bd == redoneBoard);

    // Áp dụng lại logic của play để làm lại nước đi
    boardHistory.push_back(redoneBoard);
    bd = redoneBoard;
    return true;
}

// Thực hiện hành động pass
void Game::pass() {
    // Can't pass if game is ended
    if (gameState == GameState::ENDED) return;
    // Set previousBd equal to the current board
    previousBd = bd;
    // Ghi vào lịch sử một nước is_pass = true
    boardHistory.push_back(bd);
    moveHistory.push_back( Move{0,0,true} );
    // Sau khi có nước mới (kể cả pass), redo_stack bị xoá
    redo_stack.clear();
    // Tăng số lượng pass liên tiếp
    ++consecutive_passes;
    // Đổi lượt cho bên còn lại
    to_move = opposite(to_move);
}

// Kiểm tra tính hợp lệ
bool Game::legal(const Move& m) const {
    // Can't play if game is ended
    if (gameState == GameState::ENDED) return false;
    // Pass luôn hợp lệ
    if (m.is_pass) return true;
    // Ngoài biên -> không hợp lệ
    if (!bd.in_bounds(m.r, m.c)) return false;
    // Ô phải đang trống
    if (bd.get(m.r, m.c) != Stone::EMPTY) return false;

    // Rule: Prohibition of suicide (capturing own stones)
    Board futureBoard = bd;
    // Check if any of own's stones will be captured
    if (!futureBoard.set(m.r, m.c, to_move)) return false;

    // Ko rule: One may not play in such a way as to recreate the board position following one's previous move.
    Board prevBoard = (boardHistory.size() > 1) ? boardHistory[boardHistory.size() - 2] : Board(N);
    if (prevBoard == futureBoard) return false;

    return true;
}

// Thực hiện nước đi nếu hợp lệ
bool Game::play(const Move& m) {
    // Reject nếu không hợp lệ
    if (!legal(m)) return false;
    // Nếu là pass thì dùng logic pass ở trên, xong trả về true
    if (m.is_pass) {
        pass();
        // If 2 consecutive passes, GAME ENDS
        if (is_over()) {
            gameState = GameState::ENDED;
        }
        return true;
    }

    // Set previousBd equal to the current board
    previousBd = bd;
    // Đặt quân của bên tới lượt vào ô (r, c), thực hiện tất cả logic về bàn cờ và quân cờ
    bd.set(m.r, m.c, to_move);
    
    // Add points based on how many stones were captured
    if (to_move == Stone::BLACK) {
        whitesCaptured += bd.countCaptured(previousBd, to_move);
    } else if (to_move == Stone::WHITE) {
        blacksCaptured += bd.countCaptured(previousBd, to_move);
    }
    
    // Ghi vào lịch sử để có thể undo
    boardHistory.push_back(bd);
    moveHistory.push_back(m);
    // Có nước mới thì không thể redo các nước cũ
    redo_stack.clear();
    // Một nước đặt quân sẽ phá chuỗi PASS
    consecutive_passes = 0;
    // Đổi lượt
    to_move = opposite(to_move);
    
    return true;
}

// (ONLY USE WHEN GAME ENDS) Calculate score for both players
void Game::calcScore() {
    int blackTerritory = bd.countTerritory(Stone::BLACK);
    int whiteTerritory = bd.countTerritory(Stone::WHITE);
    std::cout << "Black territory: " << blackTerritory << std::endl;
    std::cout << "White territory: " << whiteTerritory << std::endl;
    std::cout << "White stones captured by black: " << whitesCaptured << std::endl;
    std::cout << "Black stones captured by white: " << blacksCaptured << std::endl;
    blackScore = blackTerritory + whitesCaptured;
    whiteScore = whiteTerritory + blacksCaptured;
    whiteScore += komiPts; // Komi for white
    std::cout << "Black total score: " << blackScore << std::endl;
    std::cout << "White total score: " << whiteScore << std::endl;
}

// (ONLY USE WHEN GAME ENDS) Return game results (who wins or draws)
GameResults Game::results() {
    calcScore(); // Calculate score before returning game results
    if (blackScore > whiteScore) {
        return GameResults::BLACK_WINS;
    } else if (blackScore < whiteScore) {
        return GameResults::WHITE_WINS;
    } else {
        return GameResults::DRAW;
    }
}

// (ONLY USE WHEN GAME ENDS) Return player's score (black or white)
double Game::returnScore(Stone player) {
    calcScore(); // Calculate score before returning game results
    if (player == Stone::BLACK) {
        return blackScore;
    } else if (player == Stone::WHITE) {
        return whiteScore;
    } else {
        return -1;
    }
}



// Xuất trạng thái game thành chuỗi (để ghi file)
std::string Game::serialize() const {
    // Tạo bộ đệm string stream
    std::ostringstream oss;
    oss << "GOSAVE N=" << N
        // Mã hoá lượt đi (Black = 0, White = 1)
        << " side=" << (to_move == Stone::BLACK ? 0 : 1)
        // Ghi komi hiện tại
        << " komi=" << komiPts
        // Ghi số PASS liên tiếp
        << " passes=" << consecutive_passes << "\n";
    // Viết ma trận bàn (N dòng, mỗi dòng N kí tự)
    oss << bd.dump_rows();
    // Trả về chuỗi kết quả
    return oss.str();
}

// Nạp lại trạng thái từ chuỗi đã lưu
bool Game::deserialize(const std::string& data) {
    // Tạo input stream đọc theo dòng
    std::istringstream iss(data);
    // Dòng đầu tiên (header hoặc bàn)
    std::string first;
    // Nếu không đọc được dòng đầu thì trả lỗi
    if (!std::getline(iss, first)) return false;

    if (first.size() >= 3 &&
        (unsigned char)first[0] == 0xEF &&
        (unsigned char)first[1] == 0xBB &&
        (unsigned char)first[2] == 0xBF)
        first.erase(0, 3);
    // Gỡ Byte Order Mark (BOM UTF-8)

    // Hàm Lambda giúp loại bỏ '\r' ở cuối dòng nếu có để tương thích
    auto rstrip_cr = [](std::string& s){
        if (!s.empty() && s.back() == '\r') s.pop_back();
    };

    auto only_rows = [&](std::string head)->bool{
        // Clear CR ở cuối
        rstrip_cr(head);
        // Dòng đầu tiên phải có ít nhất N kí tự (nếu thiếu = không hợp lệ)
        if ((int)head.size() < N) return false;

        // Chuẩn bị chứa N dòng bàn
        std::vector<std::string> rows; rows.reserve(N);
        // Lấy đúng N kí tự đầu cho dòng 0
        rows.push_back(head.substr(0, N));

        while ((int)rows.size() < N) {
            std::string line;
            // Đọc thêm cho đủ N dòng, nếu thiếu -> lỗi
            if (!std::getline(iss, line)) return false;
            // Bỏ CR nếu có
            rstrip_cr(line);
            // Nếu dòng ngắn (< N) thì bỏ qua (đợi dòng khác)
            if ((int)line.size() < N) continue;
            // Chỉ lấy đúng N kí tự cho dòng tiếp theo
            rows.push_back(line.substr(0, N));
        }

        // Khởi tạo lại bảng NxN rỗng
        bd = Board(N);
        // Nếu chỉ có bảng không thì mặc định tới lượt là đen
        to_move = Stone::BLACK;
        // Mặc định pass liên tiếp về 0
        consecutive_passes = 0;
        // Load các dòng vào Board
        return bd.load_rows(rows);
    };

    // Trường hợp file không có HEADER GOSAVE mà chỉ có ma trận N dòng
    if (first.rfind("GOSAVE", 0) == 0) {
    // Nếu dòng đầu bắt đầu bằng "GO SAVE" = có HEADER
        std::string header = trim(first);
        // Bỏ khoảng trắng dư thừa 2 đầu dòng
        int n = 19, side = 0, passes = 0; double k = 6.5;
        // Giá trị mặc định là n, lượt, komi là số lượt pass

        auto take = [&](const std::string& key)->std::optional<std::string>{
            auto pos = header.find(key);
            if (pos == std::string::npos) return std::nullopt;
            pos += key.size();
            size_t e = header.find_first_of(" \t", pos);
            if (e == std::string::npos) e = header.size();
            return header.substr(pos, e - pos);
        };
        // Tách giá trị theo kiểu "N=", "side=", "komi=", "passes=" từ header
        if (auto s = take("N="))      n = std::max(1, std::stoi(*s));
        // Lấy N và ép nó ≥ 1
        if (auto s = take("side="))   side = std::stoi(*s);
        // Lấy lượt (0=Black, 1=White)
        if (auto s = take("komi="))   k = std::stod(*s);
        // Lấy komi
        if (auto s = take("passes=")) passes = std::max(0, std::stoi(*s));
        // Lấy số PASS liên tiếp và ép nó ≥ 0

        N = n; bd = Board(N);
        // Cập nhật kích thước và tạo board mới với kích thước N
        komiPts = k;
        // Cập nhật komi
        to_move = (side == 0 ? Stone::BLACK : Stone::WHITE);
        // Cập nhật lượt
        consecutive_passes = passes;
        // Cập nhật số pass liên tiếp

        std::vector<std::string> rows; rows.reserve(N);
        // Chuẩn bị đọc N dòng ma trận
        while ((int)rows.size() < N) {
            std::string line;
            if (!std::getline(iss, line)) return false;
            // Thiếu dòng thì ra lỗi
            rstrip_cr(line);
            // Bỏ qua CR nếu có
            if ((int)line.size() < N) continue;
            // Bỏ qua các dòng ngắn hơn N
            rows.push_back(line.substr(0, N));
            // Lấy đúng N kí tự mỗi dòng
        }
        return bd.load_rows(rows);
        // Nạp vào board và trả về kết quả là true hay false
    } else {
        return only_rows(first);
        // Không có HEADER thì xử lý như file chỉ gồm ma trận
    }
}

// Chuyển chuỗi nhập (vd "D4", "pass") thành Move (r, c)
Move Game::parse_move(const std::string& raw, int N) {
    // Chuẩn hoá bằng cách bỏ khoảng trắng đầu/ cuối
    std::string s = trim(raw);
    // Đổi chữ thường thành hoa
    for (char& ch : s) if (ch >= 'a' && ch <= 'z') ch = char(ch - 'a' + 'A');
    // PASS/ RESIGN thì xem là pass
    if (s == "PASS" || s == "RESIGN") return {0,0,true};
    // Nếu quả ngắn để parse thành cột với số thì coi như PASS
    if (s.size() < 2) return {0,0,true};
    // Lấy cột (A...T) chuyển thành chỉ số 
    int c = col_from_char(s[0]);
    // Lấy hàng là số phía sau
    int r = std::stoi(s.substr(1)) - 1;
    // Nếu cột/ hàng nằm ngoài phạm vi thì trả về PASS
    if (c < 0 || r < 0 || r >= N) return {0,0,true};
    // Hợp lệ -> trả về Move với is_pass = false
    return {r, c, false};
}

// Vẽ bàn ra chuỗi ASCII có cột và hàng hai bên
std::string Game::render_ascii() const {
    // Kết quả buffer
    std::ostringstream oss;
    // Lề trái cho nhãn cột phía trên
    oss << "   ";
    // In A..T với khoảng cách
    for (int c = 0; c < N; ++c) oss << char_from_col(c) << ' ';
    // Xuống dòng sau hàng nhãn cột trên
    oss << "\n";

    for (int r = 0; r < N; ++r) {
        // Nhãn row: Số giảm dần từ N xuống 1
        int rowLabel = N - r;
        // Căn lề cho số có 1 chữ số
        if (rowLabel < 10) oss << ' ';
        // In row bên phải
        oss << rowLabel << ' ';
        // In từng ô: '.', 'X', 'O' cùng khoảng trắng
        for (int c = 0; c < N; ++c)
            oss << stone_char(bd.get(r, c)) << ' ';
        // In nhãn row bên phải cùng newline
        oss << ' ' << rowLabel << '\n';
    }

    // Lề trái cho nhãn column phía dưới
    oss << "   ";
    // Lặp lại nhãn column dưới
    for (int c = 0; c < N; ++c) oss << char_from_col(c) << ' ';
    // Kết thúc bằng newline
    oss << "\n";
    // Trả về chuỗi ASCII board
    return oss.str();
}