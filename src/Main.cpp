#include "Engine.h"
#include <iostream>

int main() {
    ChessEngine engine;
    Board board;
    engine.initBoard(board);

    std::cout << "=== Welcome to the Updated C++ Chess Engine Demo ===\n";
    std::cout << "Initializing standard board...\n";

    
    Move best = engine.findBestMove(board, MAX_DEPTH, 5.0);

    std::cout << "Engine suggests move: ("
              << best.fromRow << ", " << best.fromCol << ") -> ("
              << best.toRow   << ", " << best.toCol << ")";
    if (best.promotion != EMPTY) {
        std::cout << " promotion to " << best.promotion;
    }
    std::cout << std::endl;

    
    Piece captured = board.squares[best.toRow][best.toCol];
    engine.makeMove(board, best);

    
    std::cout << "\nBoard after engine's move:\n";
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            std::cout << board.squares[r][c] << "\t";
        }
        std::cout << "\n";
    }

    return 0;
}
