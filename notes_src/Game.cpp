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
#include <algorithm> // Dùng std::max khi parse header,...

Game::Game(int n) : N(n), bd(n), to_move(Stone::BLACK) {}
// Khởi tạo N = n, tạo Board NxN rỗng với lượt đi đầu là BLACK
int Game::size() const { return N; }
// Trả về kích thước bàn
Board& Game::board() { return bd; }
// Truy cập mutable tới Board (hàm khác const)
const Board& Game::board() const { return bd; }
// Truy cập const tới Board (hàm const)
Stone Game::side_to_move() const { return to_move; }
// Trả về quân đang tới lượt
bool Game::is_over() const { return consecutive_passes >= 2; }
// Ván kết thúc nếu có ≥ 2 lượt pass liên tiếp
Score Game::score() const {
// Đếm tổng quân đen/ trắng hiện trên bàn (không tính TERRITORY)
    Score s{}; bd.count(s.black, s.white); return s;
    // Gọi Board::count để lấy số lượng quân mỗi bên
}

void Game::reset() {
// Đặt lại game về trạng thái ban đầu
    bd.clear(); to_move = Stone::BLACK;
    // Xoá bàn, đặt lượt về BLACK
    history.clear(); redo_stack.clear();
    // Xoá lịch sử và redo
    consecutive_passes = 0;
    // Xoá đếm PASS liên tiếp
}

bool Game::undo() {
// Hoàn tác 1 nước đi (nếu có)
    if (history.empty()) return false;
    // Không có gì để undo
    Move mv = history.back(); history.pop_back();
    // Lấy nước cuối cùng khỏi history
    if (mv.is_pass) {
        to_move = opposite(to_move);
        // Với pass: chỉ đảo lượt lại
        if (consecutive_passes > 0) --consecutive_passes;
        // Giảm số pass đã đến liên tiếp nếu > 0
    } else {
        bd.set(mv.r, mv.c, Stone::EMPTY);
        // Gỡ quân vừa đặt ở (r, c)
        to_move = opposite(to_move);
        // Đảo lượt lại cho bên vừa đi
        consecutive_passes = 0;
        // Một nước đặt quân sẽ reset chuỗi pass
    }
    redo_stack.push_back(mv);
    // Đưa nước vừa hoàn tác vào redo_stack để có thể redo
    return true;
}

bool Game::redo() {
// Làm lại 1 nước đã undo (nếu có)
    if (redo_stack.empty()) return false;
    // Không có gì để redo
    Move mv = redo_stack.back(); redo_stack.pop_back();
    // Lấy nước từ redo_stack và bỏ khỏi stack
    return play(mv);
    // Áp dụng lại logic của play để làm lại nước đi
}

void Game::pass() {
// Thực hiện hành động pass
    history.push_back(Move{0,0,true});
    // Ghi vào lịch sử một nước is_pass = true
    redo_stack.clear();
    // Sau khi có nước mới (kể cả pass), redo_stack bị xoá
    ++consecutive_passes;
    // Tăng số lượng pass liên tiếp
    to_move = opposite(to_move);
    // Đổi lượt cho bên còn lại
}

bool Game::legal(const Move& m) const {
// Kiểm tra tính hợp lệ
    if (m.is_pass) return true;
    // Pass luôn hợp lệ
    if (!bd.in_bounds(m.r, m.c)) return false;
    // Ngoài biên -> không hợp lệ
    return bd.get(m.r, m.c) == Stone::EMPTY;
    // Ô phải đang trống
}

bool Game::play(const Move& m) {
// Thực hiện nước đi nếu hợp lệ
    if (!legal(m)) return false;
    // Reject nếu không hợp lệ
    if (m.is_pass) { pass(); return true; }
    // Nếu là pass thì dùng logic pass ở trên, xong trả về true
    bd.set(m.r, m.c, to_move);
    // Đặt quân của bên tới lượt vào ô (r, c)
    history.push_back(m);
    // Ghi vào lịch sử để có thể undo
    redo_stack.clear();
    // Có nước mới thì không thể redo các nước cũ
    consecutive_passes = 0;
    // Một nước đặt quân sẽ phá chuỗi PASS
    to_move = opposite(to_move);
    // Đổi lượt
    return true;
}

std::string Game::serialize() const {
// Xuất trạng thái game thành chuỗi (để ghi file)
    std::ostringstream oss;
    // Tạo bộ đệm string stream
    oss << "GOSAVE N=" << N
        << " side=" << (to_move == Stone::BLACK ? 0 : 1)
        // Mã hoá lượt đi (Black = 0, White = 1)
        << " komi=" << komi
        // Ghi komi hiện tại
        << " passes=" << consecutive_passes << "\n";
        // Ghi số PASS liên tiếp
    oss << bd.dump_rows();
    // Viết ma trận bàn (N dòng, mỗi dòng N kí tự)
    return oss.str();
    // Trả về chuỗi kết quả
}

bool Game::deserialize(const std::string& data) {
// Nạp lại trạng thái từ chuỗi đã lưu
    std::istringstream iss(data);
    // Tạo input stream đọc theo dòng
    std::string first;
    // Dòng đầu tiên (header hoặc bàn)
    if (!std::getline(iss, first)) return false;
    // Nếu không đọc được dòng đầu thì trả lỗi

    if (first.size() >= 3 &&
        (unsigned char)first[0] == 0xEF &&
        (unsigned char)first[1] == 0xBB &&
        (unsigned char)first[2] == 0xBF)
        first.erase(0, 3);
    // Gỡ Byte Order Mark (BOM UTF-8)

    auto rstrip_cr = [](std::string& s){
        if (!s.empty() && s.back() == '\r') s.pop_back();
    };
    // Hàm Lambda giúp loại bỏ '\r' ở cuối dòng nếu có để tương thích

    auto only_rows = [&](std::string head)->bool{
        rstrip_cr(head);
        // Clear CR ở cuối
        if ((int)head.size() < N) return false;
        // Dòng đầu tiên phải có ít nhất N kí tự (nếu thiếu = không hợp lệ)

        std::vector<std::string> rows; rows.reserve(N);
        // Chuẩn bị chứa N dòng bàn
        rows.push_back(head.substr(0, N));
        // Lấy đúng N kí tự đầu cho dòng 0

        while ((int)rows.size() < N) {
            std::string line;
            if (!std::getline(iss, line)) return false;
            // Đọc thêm cho đủ N dòng, nếu thiếu -> lỗi
            rstrip_cr(line);
            // Bỏ CR nếu có
            if ((int)line.size() < N) continue;
            // Nếu dòng ngắn (< N) thì bỏ qua (đợi dòng khác)
            rows.push_back(line.substr(0, N));
            // Chỉ lấy đúng N kí tự cho dòng tiếp theo
        }

        bd = Board(N);
        // Khởi tạo lại bảng NxN rỗng
        to_move = Stone::BLACK;
        // Nếu chỉ có bảng không thì mặc định tới lượt là đen
        consecutive_passes = 0;
        // Mặc định pass liên tiếp về 0
        return bd.load_rows(rows);
        // Load các dòng vào Board
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
        komi = k;
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

Move Game::parse_move(const std::string& raw, int N) {
// Chuyển chuỗi nhập (vd "D4", "pass") thành Move (r, c)
    std::string s = trim(raw);
    // Chuẩn hoá bằng cách bỏ khoảng trắng đầu/ cuối
    for (char& ch : s) if (ch >= 'a' && ch <= 'z') ch = char(ch - 'a' + 'A');
    // Đổi chữ thường thành hoa
    if (s == "PASS" || s == "RESIGN") return {0,0,true};
    // PASS/ RESIGN thì xem là pass
    if (s.size() < 2) return {0,0,true};
    // Nếu quả ngắn để parse thành cột với số thì coi như PASS
    int c = col_from_char(s[0]);
    // Lấy cột (A...T) chuyển thành chỉ số 
    int r = std::stoi(s.substr(1)) - 1;
    // Lấy hàng là số phía sau
    if (c < 0 || r < 0 || r >= N) return {0,0,true};
    // Nếu cột/ hàng nằm ngoài phạm vi thì trả về PASS
    return {r, c, false};
    // Hợp lệ -> trả về Move với is_pass = false
}

std::string Game::render_ascii() const {
// Vẽ bàn ra chuỗi ASCII có cột và hàng hai bên
    std::ostringstream oss;
    // Kết quả buffer
    oss << "   ";
    // Lề trái cho nhãn cột phía trên
    for (int c = 0; c < N; ++c) oss << char_from_col(c) << ' ';
    // In A..T với khoảng cách
    oss << "\n";
    // Xuống dòng sau hàng nhãn cột trên

    for (int r = 0; r < N; ++r) {
        int rowLabel = N - r;
        // Nhãn row: Số giảm dần từ N xuống 1
        if (rowLabel < 10) oss << ' ';
        // Căn lề cho số có 1 chữ số
        oss << rowLabel << ' ';
        // In row bên phải
        for (int c = 0; c < N; ++c)
            oss << stone_char(bd.get(r, c)) << ' ';
        // In từng ô: '.', 'X', 'O' cùng khoảng trắng
        oss << ' ' << rowLabel << '\n';
        // In nhãn row bên phải cùng newline
    }

    oss << "   ";
    // Lề trái cho nhãn column phía dưới
    for (int c = 0; c < N; ++c) oss << char_from_col(c) << ' ';
    //Lặp lại nhãn column dưới
    oss << "\n";
    // Kết thúc bằng newline
    return oss.str();
    // Trả về chuỗi ASCII board
}
