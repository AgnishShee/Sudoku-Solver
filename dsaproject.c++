/*
    Sudoku Solver
    -------------
    A 9x9 Sudoku solving application built with recursion and backtracking.

    Features:
      - Validates the initial puzzle (checks rows, columns, and 3x3 boxes
        for duplicate values before attempting to solve).
      - Solves the puzzle using recursive backtracking.
      - Minor optimization: picks the "most constrained" empty cell
        (the one with the fewest legal candidates) at each step,
        which significantly reduces the search space for harder puzzles.
      - Displays the board (both the original and the solved version)
        in a clean, human-readable grid format.

    Compile:
        g++ -std=c++17 -O2 -o sudoku_solver sudoku_solver.cpp

    Run:
        ./sudoku_solver
*/

#include <iostream>
#include <vector>
#include <array>
#include <bitset>
#include <climits>

using namespace std;

const int N = 9;      // Board dimension
const int EMPTY = 0;  // Marker for an empty cell

using Board = array<array<int, N>, N>;

// ------------------------------------------------------------------
// Display the board in a readable 9x9 grid with box separators
// ------------------------------------------------------------------
void printBoard(const Board& board) {
    for (int r = 0; r < N; ++r) {
        if (r % 3 == 0 && r != 0)
            cout << "------+-------+------\n";
        for (int c = 0; c < N; ++c) {
            if (c % 3 == 0 && c != 0)
                cout << "| ";
            if (board[r][c] == EMPTY)
                cout << ". ";
            else
                cout << board[r][c] << " ";
        }
        cout << "\n";
    }
}

// ------------------------------------------------------------------
// Check whether placing `val` at (row, col) is legal given the
// current state of the board (ignoring whatever is currently at
// that cell itself).
// ------------------------------------------------------------------
bool isSafe(const Board& board, int row, int col, int val) {
    // Row and column check
    for (int i = 0; i < N; ++i) {
        if (i != col && board[row][i] == val) return false;
        if (i != row && board[i][col] == val) return false;
    }
    // 3x3 box check
    int boxRow = (row / 3) * 3;
    int boxCol = (col / 3) * 3;
    for (int r = boxRow; r < boxRow + 3; ++r)
        for (int c = boxCol; c < boxCol + 3; ++c)
            if ((r != row || c != col) && board[r][c] == val)
                return false;

    return true;
}

// ------------------------------------------------------------------
// Validate that the *initial* puzzle has no conflicting duplicate
// values already placed (a prerequisite before attempting to solve).
// ------------------------------------------------------------------
bool isValidPuzzle(const Board& board) {
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            int val = board[r][c];
            if (val == EMPTY) continue;
            if (val < 1 || val > 9) return false;
            if (!isSafe(board, r, c, val)) return false;
        }
    }
    return true;
}

// ------------------------------------------------------------------
// Compute the bitmask of candidate values (1-9) still legal for
// a given empty cell.
// ------------------------------------------------------------------
bitset<10> candidateMask(const Board& board, int row, int col) {
    bitset<10> used;
    for (int i = 0; i < N; ++i) {
        used[board[row][i]] = true;
        used[board[i][col]] = true;
    }
    int boxRow = (row / 3) * 3;
    int boxCol = (col / 3) * 3;
    for (int r = boxRow; r < boxRow + 3; ++r)
        for (int c = boxCol; c < boxCol + 3; ++c)
            used[board[r][c]] = true;

    bitset<10> candidates;
    for (int v = 1; v <= 9; ++v)
        if (!used[v]) candidates[v] = true;
    return candidates;
}

// ------------------------------------------------------------------
// Find the empty cell with the fewest legal candidates
// (Minimum Remaining Values heuristic). Returns false if the
// board is already completely filled.
// ------------------------------------------------------------------
bool findBestEmptyCell(const Board& board, int& bestRow, int& bestCol,
                        bitset<10>& bestCandidates) {
    int bestCount = INT_MAX;
    bool found = false;

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (board[r][c] != EMPTY) continue;
            bitset<10> cand = candidateMask(board, r, c);
            int count = cand.count();

            // Early exit: a dead-end cell (no candidates) is worst case,
            // return it immediately so the caller can backtrack fast.
            if (count == 0) {
                bestRow = r; bestCol = c; bestCandidates = cand;
                return true;
            }
            if (count < bestCount) {
                bestCount = count;
                bestRow = r; bestCol = c; bestCandidates = cand;
                found = true;
            }
        }
    }
    return found;
}

// ------------------------------------------------------------------
// Recursive backtracking solver.
// ------------------------------------------------------------------
bool solveSudoku(Board& board) {
    int row, col;
    bitset<10> candidates;

    // No empty cell found => puzzle solved
    if (!findBestEmptyCell(board, row, col, candidates))
        return true;

    // No candidates for this cell => dead end, backtrack
    if (candidates.none())
        return false;

    for (int val = 1; val <= 9; ++val) {
        if (!candidates[val]) continue;

        board[row][col] = val;      // place
        if (solveSudoku(board))     // recurse
            return true;
        board[row][col] = EMPTY;    // backtrack
    }

    return false;
}

// ------------------------------------------------------------------
// Main: demonstrates validation + solving + display on a sample puzzle
// ------------------------------------------------------------------
int main() {
    // 0 represents an empty cell
    Board board = {{
        {5,3,0, 0,7,0, 0,0,0},
        {6,0,0, 1,9,5, 0,0,0},
        {0,9,8, 0,0,0, 0,6,0},

        {8,0,0, 0,6,0, 0,0,3},
        {4,0,0, 8,0,3, 0,0,1},
        {7,0,0, 0,2,0, 0,0,6},

        {0,6,0, 0,0,0, 2,8,0},
        {0,0,0, 4,1,9, 0,0,5},
        {0,0,0, 0,8,0, 0,7,9}
    }};

    cout << "Initial Puzzle:\n";
    printBoard(board);
    cout << "\n";

    if (!isValidPuzzle(board)) {
        cout << "The puzzle is INVALID (duplicate values in a row, "
                "column, or box). Cannot solve.\n";
        return 1;
    }

    if (solveSudoku(board)) {
        cout << "Solved Puzzle:\n";
        printBoard(board);
    } else {
        cout << "No solution exists for the given puzzle.\n";
    }

    return 0;
}