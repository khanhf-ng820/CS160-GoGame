/*
    Board để lưu trạng thái bàn cờ
    Board.h sẽ khai báo tất cả thư viện và hàm cần dùng
    Board.cpp chỉ cần chạy các hàm thôi, #include "Board.h" là được
*/

#pragma once
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <cassert>

// ULTILITIES
// Định nghĩa class Stone để thể hiện trạng thái của ô cờ
enum class Stone { EMPTY = 0, BLACK = 1, WHITE = 2 };
// Hàm này giúp trả về màu quân đối (Kiểu Black -> White, White -> Black)
Stone        opposite(Stone s);
// Hàm này sẽ trả về kí tự đại diện cho quân cờ, vd 'X' là đen còn 'O' là trắng, còn '.' là ô trống á
char         stone_char(Stone s);
// Hàm này sẽ chuyển ký tự cột (A, B, C, ..., T bỏ qua I theo quy ước chuẩn của cờ vây) thành chỉ số của cột
int          col_from_char(char ch);
// Hàm này ngược lại của hàm trên th
char         char_from_col(int c);
// Hàm này sẽ giúp xoá khoảng trắng ở đầu và cuối chuỗi (Vd nếu chơi ở console nè, hoặc là đọc file save)
std::string  trim(std::string s);

// BOARD CLASS
class Board {
public:
    // Constructor giúp tạo bàn cờ  19x19, explitcit ở đây giúp complier không đánh đồng giữa 19 <-> Board(19) => Board xem là int (Dễ BUG)
    explicit Board(int n = 19);
    // Trả về kích thước của bàn cờ (Giúp truy vấn kích thước của bàn cờ)
    int  size() const;
    // Kiểm tra toạ độ (r, c) có nằm trong bàn cờ không
    bool in_bounds(int r, int c) const;
    // Lấy quân cờ tại vị trí (r, c)
    Stone get(int r, int c) const;
    // Đặt quân cờ Stone s vào (r, c)
    void  set(int r, int c, Stone s);
    // Hàm giúp xoá toàn bộ bàn cờ, reset hết về EMPTY
    void clear();
    // Hàm này giúp đếm số quân trắng và quân đen hiện có
    void count(int& black, int& white) const;

    // SERIALIZE
    // Hàm này giúp trả toàn bộ bàn cờ dưới dạng chuỗi (dòng text) để lưu file
    std::string dump_rows() const;
    // Giúp nạp lại toàn bộ bàn cờ từ chuỗi khi load game
    bool        load_rows(const std::vector<std::string>& rows);

private:
    // Kích thước của bàn cờ
    int N;
    // Vector 1 chiều giúp lưu trạng thái của bàn cờ: Mỗi phần tử là 1 ô, có 3 trạng thái là BLACK, WHITE, EMPTY
    std::vector<Stone> grid;
};
