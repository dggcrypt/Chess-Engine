#ifndef CHESSENGINE_H
#define CHESSENGINE_H

#include <vector>
#include <string>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <cstdint>

// Constants
constexpr int BOARD_SIZE = 8;
constexpr int MAX_DEPTH  = 6;   
constexpr int MATE_SCORE = 100000;
constexpr int INFINITY_SCORE = std::numeric_limits<int>::max();

// Piece Encoding (for demonstration):
enum Piece {
    EMPTY = 0,
    WP = 1,  // White Pawn
    WN,      // White Knight
    WB,      // White Bishop
    WR,      // White Rook
    WQ,      // White Queen
    WK,      // White King
    BP,      // Black Pawn
    BN,
    BB,
    BR,
    BQ,
    BK
};

// Board representation: 
struct Board {
    Piece squares[BOARD_SIZE][BOARD_SIZE];
    bool whiteToMove;

    Board() {
        // Empty board by default
        for (int r = 0; r < BOARD_SIZE; ++r) {
            for (int c = 0; c < BOARD_SIZE; ++c) {
                squares[r][c] = EMPTY;
            }
        }
        whiteToMove = true;
    }
};


struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
    Piece promotion; // For pawn promotion, set to EMPTY if no promotion.

    Move(int fr, int fc, int tr, int tc, Piece prom = EMPTY)
        : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), promotion(prom) {}
};


struct TTKey {
    uint64_t positionKey;  
    int depth;
};


struct TTEntry {
    int score;
    int flag; 
    int depth;
};


struct TTKeyHash {
    std::size_t operator()(const TTKey &k) const {
        
        auto h1 = std::hash<uint64_t>()(k.positionKey);
        auto h2 = std::hash<int>()(k.depth);
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};


struct TTKeyEqual {
    bool operator()(const TTKey &a, const TTKey &b) const {
        return (a.positionKey == b.positionKey && a.depth == b.depth);
    }
};


class ChessEngine {
public:
    ChessEngine();

    
    void initBoard(Board &board);

    
    std::vector<Move> generateMoves(const Board &board);

    
    void makeMove(Board &board, const Move &move);

    
    void undoMove(Board &board, const Move &move, Piece captured);

    
    int evaluate(const Board &board);

   
    Move findBestMove(Board &board, int depth);

private:
    uint64_t computeZobristHash(const Board &board);
    int alphaBeta(Board &board, int alpha, int beta, int depth, bool maximizingPlayer);
    int quiescenceSearch(Board &board, int alpha, int beta, bool maximizingPlayer);

    
    std::unordered_map<TTKey, TTEntry, TTKeyHash, TTKeyEqual> tTable;

    
    uint64_t zobristTable[BOARD_SIZE][BOARD_SIZE][14]; 
    void initZobristTable();
};

#endif
