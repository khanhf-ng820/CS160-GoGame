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
    explicit Game(int n = 19);
    // Khởi tạo ván game mới với bàn cờ 19x19
    int size() const;
    // Trả về kích thước của bàn cờ
    Board&       board();
    // Cho phép truy cập tới Board. Vd như cho UI vẽ,...
    const Board& board() const;
    // Truy cập const tới Board
    Stone side_to_move() const;
    // Trả về màu quân đang tới lượt
    bool  is_over() const;
    // Trả về true nếu ván đã kết thúc (ở đây chưa có gì nhiều)
    Score score() const;
    // Đếm số quân đen, trắng hiện có (không có đếm vùng, chỉ mới đếm quân đặt)
    void reset();
    // Xoá bàn cờ, đưa về trạng thái bắt đầu, đặt lại history
    bool undo();
    // Hoàn tác 1 nước (trả quân, đảo lượt, giảm pass,...)
    bool redo();
    // Làm lại 1 nước mới undo (nếu có trong redo_stack)
    void pass();
    // Thực hiện hành động pass, ghi vào history, đổi lượt,...
    bool legal(const Move& m) const;
    // Kiểm tra tính hợp lệ của nước đi (trong biên và ô trống/ pass)
    bool play(const Move& m);
    // Thực hiện nước đi nếu valid bao gồm đặt quân, ghi history, xoá redo_stack, đổi lượt,...
    std::string serialize() const;
    // Xuất toàn bộ trạng thái trò chơi thành chuỗi
    bool        deserialize(const std::string& data);
    // Nạp lại trạng thái từ chuỗi
    static Move parse_move(const std::string& raw, int N);
    // Chuyển chuỗi "D4", "Q11", pass, ... thành Move theo kích thước N
    std::string render_ascii() const;
    // Vẽ bàn ra chuỗi "ASCII art" (nhãn A...T, số hàng, ...)

public:
    double komi = 6.5;
    // Điểm bù cho trắng

private:
    int   N;
    // Kích thước bàn
    Board bd;
    // Bàn cờ: mảng ô Stone
    Stone to_move;
    // Màu quân đang tới lượt
    int   consecutive_passes = 0;
    // Số lượt pass liên tiếp
    std::vector<Move> history;
    // Lịch sử các nước đã chơi
    std::vector<Move> redo_stack;
    // Các nước hoàn tác gần nhất
};
