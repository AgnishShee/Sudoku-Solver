/*
 * ============================================================
 *  Sudoku Solver
 * ------------------------------------------------------------
 *  Description : A console-based Sudoku solving application
 *                that takes a puzzle from the user, validates
 *                it, then solves it using recursion and
 *                backtracking. Uses bitmask sets for rows,
 *                columns and 3x3 boxes so that checking
 *                whether a number can be placed is O(1)
 *                instead of scanning the row/col/box each time.
 *
 *  Skills used : C++, Recursion, Backtracking, Arrays,
 *                Algorithm Optimization
 * ============================================================
 */

#include <iostream>
#include <vector>
#include <bitset>
#include <sstream>
#include <string>
#include <chrono>

using namespace std;
using namespace std::chrono;

const int N = 9;      // board size
const int BOX = 3;    // box dimension

class SudokuSolver {
private:
    vector<vector<int>> board;

    // Bitmasks: bit k (1..9) set means digit k is already used
    // in that row / column / box.
    vector<int> rowMask, colMask, boxMask;

    static int boxIndex(int r, int c) {
        return (r / BOX) * BOX + (c / BOX);
    }

    // O(1) check using precomputed bitmasks
    bool canPlace(int r, int c, int num) const {
        int bit = 1 << num;
        return !(rowMask[r] & bit) &&
               !(colMask[c] & bit) &&
               !(boxMask[boxIndex(r, c)] & bit);
    }

    void place(int r, int c, int num) {
        int bit = 1 << num;
        board[r][c] = num;
        rowMask[r] |= bit;
        colMask[c] |= bit;
        boxMask[boxIndex(r, c)] |= bit;
    }

    void remove(int r, int c, int num) {
        int bit = ~(1 << num);
        board[r][c] = 0;
        rowMask[r] &= bit;
        colMask[c] &= bit;
        boxMask[boxIndex(r, c)] &= bit;
    }

    // Finds the empty cell with the FEWEST possible candidates.
    // This "Minimum Remaining Values" heuristic drastically cuts
    // down the branching factor compared to scanning left-to-right.
    bool findBestEmptyCell(int &br, int &bc) const {
        int bestCount = 10;
        bool found = false;

        for (int r = 0; r < N; r++) {
            for (int c = 0; c < N; c++) {
                if (board[r][c] != 0) continue;

                int used = rowMask[r] | colMask[c] | boxMask[boxIndex(r, c)];
                int count = 0;
                for (int num = 1; num <= 9; num++)
                    if (!(used & (1 << num))) count++;

                if (count < bestCount) {
                    bestCount = count;
                    br = r;
                    bc = c;
                    found = true;
                    if (bestCount == 1) return true; // can't do better
                }
            }
        }
        return found;
    }

public:
    SudokuSolver(const vector<vector<int>> &initial)
        : board(initial),
          rowMask(N, 0), colMask(N, 0), boxMask(N, 0) {
        for (int r = 0; r < N; r++)
            for (int c = 0; c < N; c++)
                if (board[r][c] != 0)
                    place(r, c, board[r][c]);
    }

    // Validates that the given puzzle has no duplicate values
    // in any row, column, or 3x3 box.
    bool isValidPuzzle() const {
        vector<int> rChk(N, 0), cChk(N, 0), bChk(N, 0);

        for (int r = 0; r < N; r++) {
            for (int c = 0; c < N; c++) {
                int val = board[r][c];
                if (val == 0) continue;
                if (val < 1 || val > 9) return false;

                int bit = 1 << val;
                int b = boxIndex(r, c);

                if (rChk[r] & bit) return false;
                if (cChk[c] & bit) return false;
                if (bChk[b] & bit) return false;

                rChk[r] |= bit;
                cChk[c] |= bit;
                bChk[b] |= bit;
            }
        }
        return true;
    }

    // Recursive backtracking solver.
    bool solve() {
        int r, c;
        if (!findBestEmptyCell(r, c))
            return true; // no empty cells left -> solved

        for (int num = 1; num <= 9; num++) {
            if (canPlace(r, c, num)) {
                place(r, c, num);

                if (solve())
                    return true;

                remove(r, c, num); // backtrack
            }
        }
        return false; // triggers backtracking in the caller
    }

    const vector<vector<int>>& getBoard() const { return board; }

    static void printBoard(const vector<vector<int>> &b) {
        for (int r = 0; r < N; r++) {
            if (r % BOX == 0 && r != 0)
                cout << "------+-------+------\n";

            for (int c = 0; c < N; c++) {
                if (c % BOX == 0 && c != 0)
                    cout << "| ";

                if (b[r][c] == 0)
                    cout << ". ";
                else
                    cout << b[r][c] << " ";
            }
            cout << "\n";
        }
    }
};

// ------------------------------------------------------------
// Input handling
// ------------------------------------------------------------

void printInstructions() {
    cout << "============================================\n";
    cout << "              SUDOKU SOLVER\n";
    cout << "============================================\n";
    cout << "Enter the puzzle row by row (9 rows total).\n";
    cout << "Use digits 1-9 for filled cells and 0 (or .)\n";
    cout << "for empty cells. Separate values with spaces\n";
    cout << "or enter them with no separator, e.g.:\n\n";
    cout << "   5 3 0 0 7 0 0 0 0\n";
    cout << "   or\n";
    cout << "   530070000\n\n";
}

// Parses one line of input into 9 integers (0-9).
// Accepts space-separated digits or a plain 9-character string.
bool parseRow(const string &line, vector<int> &row) {
    row.clear();
    stringstream ss(line);
    string token;

    // Try space-separated tokens first
    vector<int> temp;
    bool spaceFormat = true;
    stringstream test(line);
    while (test >> token) {
        if (token.size() == 1 && (token[0] == '.' || isdigit(token[0]))) {
            temp.push_back(token[0] == '.' ? 0 : token[0] - '0');
        } else {
            spaceFormat = false;
            break;
        }
    }

    if (spaceFormat && temp.size() == static_cast<size_t>(N)) {
        row = temp;
        return true;
    }

    // Fall back to compact string format, e.g. "530070000"
    string compact;
    for (char ch : line)
        if (!isspace(static_cast<unsigned char>(ch)))
            compact += ch;

    if (compact.size() != static_cast<size_t>(N))
        return false;

    row.clear();
    for (char ch : compact) {
        if (ch == '.') row.push_back(0);
        else if (isdigit(static_cast<unsigned char>(ch))) row.push_back(ch - '0');
        else return false;
    }
    return true;
}

bool readBoardFromUser(vector<vector<int>> &board) {
    board.assign(N, vector<int>(N, 0));
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int r = 0; r < N; r++) {
        vector<int> row;
        while (true) {
            cout << "Row " << (r + 1) << ": ";
            string line;
            if (!getline(cin, line)) return false;

            if (parseRow(line, row)) break;
            cout << "  Invalid row format. Please enter exactly 9 values (0-9 or '.').\n";
        }
        board[r] = row;
    }
    return true;
}

// A few built-in sample puzzles so the user can try the solver
// quickly without typing a full board.
vector<vector<int>> samplePuzzle() {
    return {
        {5,3,0, 0,7,0, 0,0,0},
        {6,0,0, 1,9,5, 0,0,0},
        {0,9,8, 0,0,0, 0,6,0},

        {8,0,0, 0,6,0, 0,0,3},
        {4,0,0, 8,0,3, 0,0,1},
        {7,0,0, 0,2,0, 0,0,6},

        {0,6,0, 0,0,0, 2,8,0},
        {0,0,0, 4,1,9, 0,0,5},
        {0,0,0, 0,8,0, 0,7,9}
    };
}

int main() {
    printInstructions();

    vector<vector<int>> board;
    char choice;

    cout << "Use the built-in sample puzzle? (y/n): ";
    cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        board = samplePuzzle();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else {
        if (!readBoardFromUser(board)) {
            cerr << "Failed to read the board. Exiting.\n";
            return 1;
        }
    }

    cout << "\nPuzzle entered:\n";
    SudokuSolver::printBoard(board);

    SudokuSolver solver(board);

    if (!solver.isValidPuzzle()) {
        cout << "\nThis puzzle is INVALID: it has a duplicate digit\n";
        cout << "in some row, column, or 3x3 box. Please check your\n";
        cout << "input and try again.\n";
        return 1;
    }

    cout << "\nSolving...\n";
    auto start = high_resolution_clock::now();
    bool solved = solver.solve();
    auto end = high_resolution_clock::now();
    double ms = duration_cast<duration<double, milli>>(end - start).count();

    if (solved) {
        cout << "\nSolved puzzle:\n";
        SudokuSolver::printBoard(solver.getBoard());
        cout << "\nSolved in " << ms << " ms.\n";
    } else {
        cout << "\nNo solution exists for this puzzle.\n";
    }

    return 0;
}
