/*
TÓM TẮT TRƯỚC CÁC HÀM
- opposite(s): trả màu còn lại
- stone_char(s): '.'/'X'/'O' để xuất ra ASCII
- col_from_char(ch): chuyển 'A'..'T' -> 0..(N-1). Input có thể là chữ thường, sẽ có chuẩn hoá thành chữ hoa
- char_from_col(c): 0..N-1 -> 'A'..'T'
- trim(s): xóa khoảng trống 2 đầu (lệnh này bên UI xài nhiều hơn, nó viết bên Board vậy thôi)
- ctor Board(int n): tạo bảng N×N. Mặc định là EMPTY
- in_bounds(r,c): kiểm tra vị trí quân cờ
- get/set: đọc/ghi ô
- clear(): đưa toàn bộ ô về EMPTY
- count(black, white): đếm số quây, đây mới cơ bản thôi chứ nếu muốn đếm TERRITORY thì sẽ cần thêm luật bổ sung
- dump_rows(): gom N dòng ký tự cho save
- load_rows(rows): load lại bàn từ N chuỗi, bỏ qua ký tự lạ, chỉ nhận .XO
*/
#include "Board.h"

// ULTILITIES
Stone opposite(Stone s) {
// Đây là hàm trả về quân đối, kiểu BLACK trả về WHITE và ngược lại
// Lưu ý là nếu truyền EMPTY nó vẫn trả về BLACK để tiện cho luân phiên lượt, nhưng mà cái này hiếm trong trường hợp bthuong=))
    return (s == Stone::BLACK ? Stone::WHITE : Stone::BLACK);
}

char stone_char(Stone s) {
// Đây là hàm chuyển Stone thành kí tự, ví dụ quân đen <-> 'X' và quân trắng <-> 'O', trống thì <-> '.'
    if (s == Stone::BLACK) return 'X';
    if (s == Stone::WHITE) return 'O';
    return '.';
}

int col_from_char(char ch) {
// Chuyển kí tự cột thành chỉ số cột
    if (ch >= 'a' && ch <= 'z') ch = char(ch - 'a' + 'A');
    // Chuẩn hoá ở đây, chuyển kí tự thường thành kí tự in hoa, ví dụ a -> A
    if (!(ch >= 'A' && ch <= 'Z')) return -1;
    // Nếu không phải chữ cái thì trả về -1 (Invalid value)
    if (ch >= 'J') return (ch - 'A') - 1;
    // Theo quy ước chuẩn của cờ vây thì ta bỏ 'I'
    // Nếu mà kí tự ≥ 'J' thì trừ 1 để nhảy qua 'I' luôn
    return ch - 'A';
    // Với kí tự (AB..H) mà không qua I thì chỉ cần ch - 'A' là được
}

char char_from_col(int c) {
// Ngược lại từ chỉ số cột thành kí tự cột, vai trò giống trên thôi
    char col = char('A' + c);
    if (col >= 'I') col++;
    return col;
}

std::string trim(std::string s) {
// Hàm giúp cắt khoảng trắng thừa ở đầu/ cuối chuỗi
    size_t a = 0, b = s.size();
    // a sẽ chạy từ đầu chuỗi còn b chạy từ cuối chuỗi
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    // Tăng a tới khi gặp kí tự không phải khoảng trắng, ở dưới thì ngược lại giảm b
    while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
    return s.substr(a, b - a);
    // Xong sẽ trả về đoạn [a, b)
}

// BOARD IMPLEMENTATION
Board::Board(int n) : N(n), grid(n * n, Stone::EMPTY) {}
// Đây là constructor, nó lưu kích thước N của bảng, và tạo mảng (grid) có NxN phần tử, tất cả đều ở dạng EMPTY
int Board::size() const { return N; }
// Hàm này sẽ trả về kích thước bảng, giúp truy vấn...
bool Board::in_bounds(int r, int c) const {
// Đây là hàm sẽ kiểm tra toạ độ (r, c) có nằm trong phạm vi của bảng không
    return r >= 0 && r < N && c >= 0 && c < N;
    // Valid nếu 0 ≤ r, c ≤ N
}

Stone Board::get(int r, int c) const {
// Hàm này sẽ lấy (touch, get) quân ở ô (r, c)
    assert(in_bounds(r, c));
    // Nó đảm bảo việc chỉ lấy quân như vậy khi vị trí đúng (valid)
    return grid[r * N + c];
    // Chuyển ánh xạ 2D -> 1D theo công thức chuẩn là index = r*N + c á
}

void Board::set(int r, int c, Stone s) {
// Hàm này gán quân s vào ô (r, c)
    assert(in_bounds(r, c));
    // Phải check vị trí có nằm valid ko
    grid[r * N + c] = s;
    // Xong sẽ gán vào mảng 1D như trên thôi
}

void Board::clear() {
// Hàm giúp reset bàn cờ
    std::fill(grid.begin(), grid.end(), Stone::EMPTY);
    // std::fill để đảm bảo mọi ô đều về EMPTY
}

void Board::count(int& black, int& white) const {
// Hàm này sẽ đếm số quân đen/ trắng hiện có trên bàn cờ
    black = white = 0;
    // Reset bộ đếm ban đầu là 0
    for (auto s : grid) {
    // Duyệt qua mọi ô trong grid
        if (s == Stone::BLACK) ++black; // Đếm như bthuong thôi
        else if (s == Stone::WHITE) ++white;
    }
}

std::string Board::dump_rows() const {
// Hàm này để Serialize, kiểu xuất toàn bộ bàn này những dòng text á
    std::string out; out.reserve(N * (N + 1));
    // Dự trù dung lượng: kiểu N dòng, mỗi dòng có N kí tự + 1 dòng mới (newline)
    for (int r = 0; r < N; ++r) {
    // Quét qua từng row
        for (int c = 0; c < N; ++c)
        // Quét qua từng column
            out.push_back(stone_char(get(r, c)));
            // Thêm kí tự đại diện cho từng ô (./X/O)
        out.push_back('\n');
        // Kết thúc dòng bằng newline
    }
    return out;
    // Trả về chuỗi kết quả
}

bool Board::load_rows(const std::vector<std::string>& rows) {
// Hàm nạp bàn cờ từ mảng chuỗi (trong đó mỗi chuỗi là 1 dòng)
    if ((int)rows.size() != N) return false;
    // Bắt buộc phải có đúng N dòng
    for (int r = 0; r < N; ++r) {
    // Duyệt qua từng dòng
        if ((int)rows[r].size() != N) return false;
        // Mỗi dòng phải có đúng N kí tự
        for (int c = 0; c < N; ++c) {
        // Duyệt từng cột trên dòng
            char ch = rows[r][c];
            // Lấy kí tự ch tại vị trí (r, c)
            Stone s = Stone::EMPTY;
            // Mặc định gốc là EMPTY
            if (ch == 'X') s = Stone::BLACK;
            // Nếu kí tự đó là 'X' thì sẽ gán là quân đen
            else if (ch == 'O') s = Stone::WHITE;
            // Nếu kí tự đó là 'O' thì sẽ gán là quân trắng
            set(r, c, s);
            // Gán lại vào board
        }
    }
    return true;
    // Nếu mọi thứ ok, valid hết thì trả về true
}
