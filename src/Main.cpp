#include "ChessEngine.h"
#include <iostream>

int main() {
    ChessEngine engine;
    Board board;
    engine.initBoard(board);

    std::cout << "Welcome to the C++ Chess Engine Demo!\n";
    std::cout << "Initial board setup done.\n";

    
    Move best = engine.findBestMove(board, MAX_DEPTH);

    std::cout << "Engine suggests move: ("
              << best.fromRow << ", " << best.fromCol << ") -> ("
              << best.toRow   << ", " << best.toCol << ")\n";

   
    Piece captured = board.squares[best.toRow][best.toCol];
    engine.makeMove(board, best);

    
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            std::cout << board.squares[r][c] << "\t";
        }
        std::cout << "\n";
    }

    return 0;
}
