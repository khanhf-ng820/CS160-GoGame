/*
TÓM TẮT SƠ
Các trường dữ liệu chính (main):
- int N: kích thước bàn (9/13/19)
- Board bd: đối tượng bàn cờ
- Stone to_move: lượt hiện tại (BLACK/WHITE)
- int consecutive_passes: đếm PASS (≥ 2 liên tiếp -> ván kết thúc) (nhưng mà có vẻ hoạt động chưa được ổn lắm !!)
- double komi: điểm bù cho trắng do trắng đi sau (tôi set là 6.5)
- vector<Move> history, redo_stack: dùng cho undo/redo.

Hàm chính:
- size(), board(), side_to_move(): đọc state cơ bản
- is_over(): true nếu có ≥ 2 lần PASS liên tiếp (check lại này giúp tôi)
- legal(m): kiểm tra biên với ô trống (có thể mở rộng thêm)
- play(m): đánh 1 nước (hoặc PASS). Ghi lịch sử, xóa redo, đổi lượt
- pass(): thêm 1 PASS, tăng consecutive_passes, đổi lượt
- undo()/redo(): quay lại/tiến tới theo lịch sử nước đi
- score(): đếm quân (BASIC), khi tính thắng thua sẽ cộng KOMI cho trắng
- serialize()/deserialize(): lưu/đọc theo tiêu đề là "GOSAVE N=.. side=.. komi=.. passes=.."
- render_ascii(): in bàn ra console
- parse_move("D4"): chuyển chuỗi tọa độ sang Move (bỏ I)
*/
#pragma once
#include <string>
#include <vector> // Dùng cho lịch sử nước đi (history), redo,...
#include <optional> // Thêm hờ thôi chứ vẫn chưa sử dụng trong đây
#include <random> // Có mỗi AI là xài cái này nhma cứ bỏ hết đại đi=))
#include <sstream> // ostringstream, istringstream trong serialize, deserialize á
#include "Board.h" // Dùng Board, Stone với 1 số cái như opposite, trim,...

// 2 kiểu chế độ chơi, PvP hoặc PvE
enum class GameMode { PVP, PVE };
// Game states
enum class GameState { PLAYING, ENDED };
// Endgame results
enum class GameResults { BLACK_WINS, WHITE_WINS, DRAW };

// Cái này mô tả 1 nước đi hàng r cột c hoặc là is_pass = true nếu skip
struct Move { int r = 0, c = 0; bool is_pass = false; };
// Đếm đơn giản số ô đen với trắng
struct Score { int black = 0, white = 0; };

// Class game này quản lý trạng thái với luật cơ bản của ván cờ (kiểu luân phiên, tính hợp lệ hay lịch sử)
class Game {
public:
    // Khởi tạo ván game mới với bàn cờ 19x19
    explicit Game(int n = 19);
    // Trả về kích thước của bàn cờ
    int size() const;
    // Returns the game's komi
    double komi() const;
    // Cho phép truy cập tới Board. Vd như cho UI vẽ,...
    Board&       board();
    // Truy cập const tới Board
    const Board& board() const;
    // Trả về màu quân đang tới lượt
    Stone side_to_move() const;
    // Trả về true nếu ván đã kết thúc (ở đây chưa có gì nhiều)
    bool  is_over() const;
    // Đếm số quân đen, trắng hiện có (không có đếm vùng, chỉ mới đếm quân đặt)
    Score score() const;
    // Xoá bàn cờ, đưa về trạng thái bắt đầu, đặt lại history
    void reset();
    // Hoàn tác 1 nước (trả quân, đảo lượt, giảm pass,...)
    bool undo();
    // Làm lại 1 nước mới undo (nếu có trong redo_stack)
    bool redo();
    // Thực hiện hành động pass, ghi vào history, đổi lượt,...
    void pass();
    // Kiểm tra tính hợp lệ của nước đi (trong biên và ô trống/ pass)
    bool legal(const Move& m) const;
    // Thực hiện nước đi nếu valid bao gồm đặt quân, ghi history, xoá redo_stack, đổi lượt,...
    bool play(const Move& m);

    // (ONLY USE WHEN GAME ENDS) Calculate score for both players
    void calcScore();
    // (ONLY USE WHEN GAME ENDS) Return game results (who wins or draws)
    GameResults results();
    // (ONLY USE WHEN GAME ENDS) Return player's score (black or white)
    double returnScore(Stone player);
    
    // Xuất toàn bộ trạng thái trò chơi thành chuỗi
    std::string serialize() const;
    // Nạp lại trạng thái từ chuỗi
    bool        deserialize(const std::string& data);
    // Chuyển chuỗi "D4", "Q11", pass, ... thành Move theo kích thước N
    static Move parse_move(const std::string& raw, int N);
    // Vẽ bàn ra chuỗi "ASCII art" (nhãn A...T, số hàng, ...)
    std::string render_ascii() const;

private:
    // Điểm bù cho trắng (WHITE) (Japanese komi)
    double komiPts = 6.5;

    // Game state
    GameState gameState = GameState::PLAYING;
    // Kích thước bàn
    int   N;
    // Bàn cờ: mảng ô Stone
    Board bd, previousBd;
    // Màu quân đang tới lượt
    Stone to_move;
    // Số lượt pass liên tiếp
    int   consecutive_passes = 0;
    // Points: number of white & black stones captured by opponents
    int   blacksCaptured = 0, whitesCaptured = 0;
    // Total points of each player (territory and captures)
    double blackScore = 0, whiteScore = 0;
    // History of all boards played
    std::vector<Board> boardHistory;
    // Lịch sử các nước đã chơi (for printing and keeping records ONLY)
    std::vector<Move> moveHistory;
    // Các bàn cờ hoàn tác gần nhất
    std::vector<Board> redo_stack;
};
