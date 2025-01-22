#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <string>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <cstdint>
#include <chrono>

constexpr int BOARD_SIZE     = 8;
constexpr int MAX_DEPTH      = 6;           // Default max depth for iterative deepening
constexpr int MATE_SCORE     = 100000;
constexpr int INFINITY_SCORE = 100000000;
constexpr int QSEARCH_DEPTH  = 4;           // Depth limit for quiescence search (optional)
constexpr double DEFAULT_TIME_LIMIT = 5.0;  // 5 seconds as an example

// Piece Encoding
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

struct Board {
    Piece squares[BOARD_SIZE][BOARD_SIZE];
    bool whiteToMove;

   
};

// Basic Move
struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
    Piece promotion; // For pawn promotion, or EMPTY if none.

    Move(int fr, int fc, int tr, int tc, Piece prom = EMPTY)
        : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), promotion(prom) {}
    Move() : fromRow(0), fromCol(0), toRow(0), toCol(0), promotion(EMPTY) {}
};

// Transposition Table Key and Entry
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

    // Board initialization
    void initBoard(Board &board);

    // Search interface
    Move findBestMove(Board &board, int maxDepth = MAX_DEPTH, double timeLimit = DEFAULT_TIME_LIMIT);

private:
    
    std::vector<Move> generatePseudoLegalMoves(const Board &board);
    bool isKingInCheck(const Board &board, bool whiteKing);
    std::vector<Move> generateLegalMoves(const Board &board);

    
    void makeMove(Board &board, const Move &move);
    void undoMove(Board &board, const Move &move, Piece captured);

    
    int evaluate(const Board &board);

    
    int alphaBeta(Board &board, int alpha, int beta, int depth, bool doNullMove = true);
    int quiescenceSearch(Board &board, int alpha, int beta);
    
    
    int searchRoot(Board &board, int depth, Move &bestMove);

    
    uint64_t computeZobristHash(const Board &board);
    void initZobristTable();

    std::unordered_map<TTKey, TTEntry, TTKeyHash, TTKeyEqual> tTable;
    uint64_t zobristTable[BOARD_SIZE][BOARD_SIZE][14]; 

    
    std::chrono::steady_clock::time_point startTime;
    double timeLimitSec;
    bool timeIsUp();

    
    void sortMoves(Board &board, std::vector<Move> &moves);
};

#endif
