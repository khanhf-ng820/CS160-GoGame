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
// Define enum class Liberty to denote if an intersection has liberty or not
enum class Liberty { EMPTY = 0, HAS_LIBERTY = 1, NO_LIBERTY = 2 };
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
    // Constructor giúp tạo bàn cờ  19x19, explicit ở đây giúp complier không đánh đồng giữa 19 <-> Board(19) => Board xem là int (Dễ BUG)
    explicit Board(int n = 19);
    // Trả về kích thước của bàn cờ (Giúp truy vấn kích thước của bàn cờ)
    int  size() const;
    // Returns 1D index of 2D coordinates (r, c)
    int  idx1D(int r, int c) const;
    // Kiểm tra toạ độ (r, c) có nằm trong bàn cờ không
    bool in_bounds(int r, int c) const;
    // Get the coordinates of an intersection's neighbors (stones adjacent to it)
    std::vector<std::pair<int, int>> getNeighbors(int r, int c) const;
    // Lấy quân cờ tại vị trí (r, c)
    Stone get(int r, int c) const;
    // Đặt quân cờ Stone s vào (r, c), thực hiện tất cả logic về bàn cờ và quân cờ
    void  set(int r, int c, Stone s);
    // Hàm giúp xoá toàn bộ bàn cờ, reset hết về EMPTY
    void clear();
    // Hàm này giúp đếm số quân trắng và quân đen hiện có
    void count(int& black, int& white) const;
    // Check if an intersection has liberty or not
    bool interHasLiberty(int r, int c) const;
    // Check liberties of all intersections and output them to the hasLiberty vector
    void checkLiberty();
    // Returns a vector of all stones of a player that will be captured (removed) due to no liberties
    std::vector<std::pair<int, int>> toBeCaptured(Stone player) const;

    // SERIALIZE
    // Hàm này giúp trả toàn bộ bàn cờ dưới dạng chuỗi (dòng text) để lưu file
    std::string dump_rows() const;
    // Giúp nạp lại toàn bộ bàn cờ từ chuỗi khi load game
    bool        load_rows(const std::vector<std::string>& rows);

private:
    // Consts
    // Signify which intersection coordinates are considered 'adjacent'
    std::vector<std::pair<int, int>> offsets = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

    // Kích thước của bàn cờ
    int N;
    // Vector 1 chiều giúp lưu trạng thái của bàn cờ: Mỗi phần tử là 1 ô, có 3 trạng thái là BLACK, WHITE, EMPTY
    std::vector<Stone> grid;
    // 1D vector to denote whether an intersection has liberty or not
    std::vector<Liberty> hasLiberty;

    // Recursive DFS algorithm function for Board::checkLiberty() member function
    void dfs(int r, int c, Stone stone, std::vector<std::pair<int, int>>& components, std::vector<bool>& visited) const;
};
