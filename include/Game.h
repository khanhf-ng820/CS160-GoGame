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

enum class GameMode { PVP, PVE };
// 2 kiểu chế độ chơi, PvP hoặc PvE

struct Move { int r = 0, c = 0; bool is_pass = false; };
// Cái này mô tả 1 nước đi hàng r cột c hoặc là is_pass = true nếu skip
struct Score { int black = 0, white = 0; };
// Đếm đơn giản số ô đen với trắng

class Game {
// Class game này quản lý trạng thái với luật cơ bản của ván cờ (kiểu luân phiên, tính hợp lệ hay lịch sử)
public:
    // Khởi tạo ván game mới với bàn cờ 19x19
    explicit Game(int n = 19);
    // Trả về kích thước của bàn cờ
    int size() const;
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
    // Xuất toàn bộ trạng thái trò chơi thành chuỗi
    std::string serialize() const;
    // Nạp lại trạng thái từ chuỗi
    bool        deserialize(const std::string& data);
    // Chuyển chuỗi "D4", "Q11", pass, ... thành Move theo kích thước N
    static Move parse_move(const std::string& raw, int N);
    // Vẽ bàn ra chuỗi "ASCII art" (nhãn A...T, số hàng, ...)
    std::string render_ascii() const;


    // Điểm bù cho trắng
    double komi = 6.5;

private:
    // Kích thước bàn
    int   N;
    // Bàn cờ: mảng ô Stone
    Board bd;
    // Màu quân đang tới lượt
    Stone to_move;
    // Số lượt pass liên tiếp
    int   consecutive_passes = 0;
    // Lịch sử các nước đã chơi
    std::vector<Move> history;
    // Các nước hoàn tác gần nhất
    std::vector<Move> redo_stack;
};
