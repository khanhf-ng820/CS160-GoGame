/*
TÓM TẮT TRƯỚC CÁC HÀM
- opposite(s): trả màu còn lại
- stone_char(s): '.'/'X'/'O' để xuất ra ASCII
+ ***** (X: BLACK, O: WHITE) *****
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
// *** Lưu ý là nếu truyền EMPTY nó vẫn trả về BLACK để tiện cho luân phiên lượt, nhưng mà cái này hiếm trong trường hợp bthuong=)) ***
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

// Ngược lại từ chỉ số cột thành kí tự cột, vai trò giống trên thôi
char char_from_col(int c) {
    char col = char('A' + c);
    if (col >= 'I') col++;
    return col;
}

// Hàm giúp cắt khoảng trắng thừa ở đầu/ cuối chuỗi
std::string trim(std::string s) {
    // a sẽ chạy từ đầu chuỗi còn b chạy từ cuối chuỗi
    size_t a = 0, b = s.size();
    // Tăng a tới khi gặp kí tự không phải khoảng trắng, ở dưới thì ngược lại giảm b
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
    // Xong sẽ trả về đoạn [a, b)
    return s.substr(a, b - a);
}



// ********************
// BOARD IMPLEMENTATION
// ********************
// Đây là constructor, nó lưu kích thước N của bảng, và tạo mảng (grid, hasLiberty) có NxN phần tử, tất cả đều ở dạng EMPTY
Board::Board(int n) : N(n), grid(n * n, Stone::EMPTY), hasLiberty(n * n, Liberty::EMPTY) {}

// Hàm này sẽ trả về kích thước bảng, giúp truy vấn...
int Board::size() const { return N; }

// Returns the grid vector<Stone>
std::vector<Stone> Board::getGrid() const { return grid; }

// Returns 1D index of 2D coordinates (r, c)
int Board::idx1D(int r, int c) const { return c + N * r; }

// Đây là hàm sẽ kiểm tra toạ độ (r, c) có nằm trong phạm vi của bảng không
bool Board::in_bounds(int r, int c) const {
    // Valid nếu 0 ≤ r, c ≤ N
    return r >= 0 && r < N && c >= 0 && c < N;
}

// Get the coordinates of an intersection's neighbors (stones adjacent to it)
std::vector<std::pair<int, int>> Board::getNeighbors(int r, int c) const {
    std::vector<std::pair<int, int>> neighbors;

    for (const auto& [dx, dy] : offsets) {
        if (in_bounds(r + dx, c + dy)) {
            neighbors.push_back(std::make_pair(r + dx, c + dy));
        }
    }
    return neighbors;
}

// Hàm này sẽ lấy (touch, get) quân ở ô (r, c)
Stone Board::get(int r, int c) const {
    // Nó đảm bảo việc chỉ lấy quân như vậy khi vị trí đúng (valid)
    assert(in_bounds(r, c));
    // Chuyển ánh xạ 2D -> 1D theo công thức chuẩn là index = r*N + c á
    return grid[r * N + c];
}

// Hàm này gán quân s vào ô (r, c), thực hiện tất cả logic về bàn cờ và quân cờ
// Rule: Prohibition of suicide (capturing own stones). Function checks if it's suicide move
// Returns true if it's non-suicide move, false if it's suicide move
bool Board::set(int r, int c, Stone s) {
    // Phải check vị trí có nằm valid ko
    assert(in_bounds(r, c));
    // Xong sẽ gán vào mảng 1D như trên thôi

    // Step 1: Playing a stone
    grid[r * N + c] = s;
    // Step 2: Capture opponent's stones with no liberties
    std::vector<std::pair<int, int>> opponentCaptured = toBeCaptured(opposite(s));
    for (const auto& [r0, c0] : opponentCaptured)  grid[idx1D(r0, c0)] = Stone::EMPTY;
    // Step 3: Capture own stones with no liberties
    std::vector<std::pair<int, int>> ownCaptured = toBeCaptured(s);
    for (const auto& [r0, c0] : ownCaptured)  grid[idx1D(r0, c0)] = Stone::EMPTY;

    // Prohibition of suicide: Check if any of own's stones will be captured
    if (ownCaptured.size() > 0) return false;
    
    return true;
}

// Hàm giúp reset bàn cờ
void Board::clear() {
    // std::fill để đảm bảo mọi ô đều về EMPTY
    std::fill(grid.begin(), grid.end(), Stone::EMPTY);
}

// Hàm này sẽ đếm số quân đen/ trắng hiện có trên bàn cờ
void Board::count(int& black, int& white) const {
    // Reset bộ đếm ban đầu là 0
    black = white = 0;
    // Duyệt qua mọi ô trong grid
    for (auto s : grid) {
        if (s == Stone::BLACK) ++black; // Đếm như bthuong thôi
        else if (s == Stone::WHITE) ++white;
    }
}

// Check if an intersection is adjacent to a black or white stone, or an empty intersection (liberty)
bool Board::interNearStone(int r, int c, Stone stone) const {
    std::vector<std::pair<int, int>> neighbors = getNeighbors(r, c);
    for (const auto& [r0, c0] : neighbors) {
        if (get(r0, c0) == stone) return true;
    }
    return false;
}

// private member of Board class
// Recursive DFS algorithm function for Board::checkLiberty() member function
void Board::dfs(int r, int c, Stone stone, std::vector<std::pair<int, int>>& components, std::vector<bool>& visited) const {
    int idx = idx1D(r, c);
    if (visited[idx] || get(r, c) != stone) return;
    visited[idx] = true;
    components.push_back(std::make_pair(r, c));

    std::vector<std::pair<int, int>> neighbors = getNeighbors(r, c);
    for (const auto& [r0, c0] : neighbors) {
        if (get(r0, c0) == stone) dfs(r0, c0, stone, components, visited);
    }
}

// Check liberties of all intersections and output them to the hasLiberty vector
void Board::checkLiberty() {
    // Perform DFS to find all components of intersections of the same type
    std::vector<bool> visited(N * N, false);

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            std::vector<std::pair<int, int>> components;
            Stone componentStone = get(r, c);
            dfs(r, c, componentStone, components, visited); // Perform DFS

            bool libertyExists = false;
            // Check each intersection of component whether it has liberty
            for (const auto& [r0, c0] : components) {
                if (interNearStone(r0, c0, Stone::EMPTY)) {
                    libertyExists = true;
                    break;
                }
            }

            // Output results to the hasLiberty vector
            for (const auto& [r0, c0] : components) {
                int idx0 = idx1D(r0, c0);
                // We only care about liberties of stones, not liberties of an empty space
                if (componentStone == Stone::EMPTY) {
                    hasLiberty[idx0] = Liberty::EMPTY;
                } else {
                    hasLiberty[idx0] = (libertyExists ? Liberty::HAS_LIBERTY : Liberty::NO_LIBERTY);
                }
            }
        }
    }
}

// Returns a vector of all stones of a player that will be captured (removed) due to no liberties
std::vector<std::pair<int, int>> Board::toBeCaptured(Stone player) {
    checkLiberty();
    std::vector<std::pair<int, int>> noLiberties;

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            int idx = idx1D(r, c);
            // Check if stone has no liberties and belongs to player
            if (hasLiberty[idx] == Liberty::NO_LIBERTY && get(r, c) == player)
                noLiberties.push_back(std::make_pair(r, c));
        }
    }

    return noLiberties;
}

// Count the number of intersections in a player's territory (ONLY USED FOR SCORING WHEN GAME ENDS)
int Board::countTerritory(Stone player) const {
    // Perform DFS to find all components of empty intersections
    int territoryCounter = 0;
    std::vector<bool> visited(N * N, false);

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            std::vector<std::pair<int, int>> components;
            dfs(r, c, Stone::EMPTY, components, visited); // Perform DFS

            bool inBlackTerritory = false, inWhiteTerritory = false;
            // Check each empty intersection of component whether it's near a black stone
            for (const auto& [r0, c0] : components) {
                if (interNearStone(r0, c0, Stone::BLACK)) {
                    inBlackTerritory = true;
                    break;
                }
            }
            // Check each empty intersection of component whether it's near a white stone
            for (const auto& [r0, c0] : components) {
                if (interNearStone(r0, c0, Stone::WHITE)) {
                    inWhiteTerritory = true;
                    break;
                }
            }

            // Check if in player's territory and add to counter
            if ((player == Stone::BLACK && inBlackTerritory  && !inWhiteTerritory) || 
                (player == Stone::WHITE && !inBlackTerritory && inWhiteTerritory )) {
                territoryCounter += components.size();
            }
        }
    }
    return territoryCounter;
}



// Compare boards
bool Board::operator==(const Board& board2) const {
    return grid == board2.getGrid();
}

// Count how many stones were captured after playing a move
int Board::countCaptured(const Board& previousBoard, Stone played) const {
    // Count black & white stones in previous board
    int prevBlack = 0, prevWhite = 0;
    previousBoard.count(prevBlack, prevWhite);
    // Count black & white stones in current board
    int curBlack = 0, curWhite = 0;
    count(curBlack, curWhite);

    // Count how many stones were captured
    if (played == Stone::BLACK) {
        return prevWhite - curWhite;
    } else if (played == Stone::WHITE) {
        return prevBlack - curBlack;
    }
    return 0;
}



// *** SERIALIZE
// Hàm này để Serialize, kiểu xuất toàn bộ bàn này những dòng text á
std::string Board::dump_rows() const {
    // Dự trù dung lượng: kiểu N dòng, mỗi dòng có N kí tự + 1 dòng mới (newline)
    std::string out; out.reserve(N * (N + 1));
    // Quét qua từng row
    for (int r = 0; r < N; ++r) {
        // Quét qua từng column
        for (int c = 0; c < N; ++c)
            // Thêm kí tự đại diện cho từng ô (./X/O)
            out.push_back(stone_char(get(r, c)));
        // Kết thúc dòng bằng newline
        out.push_back('\n');
    }
    // Trả về chuỗi kết quả
    return out;
}

// Hàm nạp bàn cờ từ mảng chuỗi (trong đó mỗi chuỗi là 1 dòng)
bool Board::load_rows(const std::vector<std::string>& rows) {
    // Bắt buộc phải có đúng N dòng
    if ((int)rows.size() != N) return false;
    // Duyệt qua từng dòng
    for (int r = 0; r < N; ++r) {
        // Mỗi dòng phải có đúng N kí tự
        if ((int)rows[r].size() != N) return false;
        // Duyệt từng cột trên dòng
        for (int c = 0; c < N; ++c) {
            // Lấy kí tự ch tại vị trí (r, c)
            char ch = rows[r][c];
            // Mặc định gốc là EMPTY
            Stone s = Stone::EMPTY;
            // Nếu kí tự đó là 'X' thì sẽ gán là quân đen
            if (ch == 'X') s = Stone::BLACK;
            // Nếu kí tự đó là 'O' thì sẽ gán là quân trắng
            else if (ch == 'O') s = Stone::WHITE;
            // Gán lại vào board
            set(r, c, s);
        }
    }
    return true;
    // Nếu mọi thứ ok, valid hết thì trả về true
}
