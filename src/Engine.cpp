#include "ChessEngine.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <chrono>

// Constructor
ChessEngine::ChessEngine() {
    initZobristTable();
}

// Fill zobristTable with random 64-bit numbers
void ChessEngine::initZobristTable() {
    std::mt19937_64 rng(0xDEADBEAF12345678ULL); // fixed seed for reproducibility
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            for (int piece = 0; piece < 14; ++piece) {
                zobristTable[r][c][piece] = rng();
            }
        }
    }
}

// Compute a (very) naive Zobrist hash for the board
uint64_t ChessEngine::computeZobristHash(const Board &board) {
    uint64_t h = 0ULL;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            Piece p = board.squares[r][c];
            if (p != EMPTY) {
                h ^= zobristTable[r][c][p];
            }
        }
    }
    // White to move bit
    if (!board.whiteToMove) {
        h = ~h;
    }
    return h;
}

// Standard chess initial setup
void ChessEngine::initBoard(Board &board) {
    // Clear board
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            board.squares[r][c] = EMPTY;
        }
    }

    // Place pawns
    for (int c = 0; c < BOARD_SIZE; ++c) {
        board.squares[1][c] = WP;
        board.squares[6][c] = BP;
    }

    // Rooks
    board.squares[0][0] = WR; board.squares[0][7] = WR;
    board.squares[7][0] = BR; board.squares[7][7] = BR;

    // Knights
    board.squares[0][1] = WN; board.squares[0][6] = WN;
    board.squares[7][1] = BN; board.squares[7][6] = BN;

    // Bishops
    board.squares[0][2] = WB; board.squares[0][5] = WB;
    board.squares[7][2] = BB; board.squares[7][5] = BB;

    // Queens
    board.squares[0][3] = WQ;
    board.squares[7][3] = BQ;

    // Kings
    board.squares[0][4] = WK;
    board.squares[7][4] = BK;

    board.whiteToMove = true;
}

// Generate all pseudo-legal moves (does not check for checks or pins)
std::vector<Move> ChessEngine::generateMoves(const Board &board) {
    std::vector<Move> moves;

    // For simplicity, only do single-step moves for pawns,
    // knights and captures (very incomplete).
    // Real engines handle all moves, including checks, castling, en passant, etc.

    // Directions for knights
    static const int knightOffsets[8][2] = {
        {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
        {-1, -2}, {-1, 2}, {1, -2}, {1, 2}
    };

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            Piece piece = board.squares[r][c];
            if (piece == EMPTY) continue;

            // Check color to move
            bool isWhitePiece = (piece >= WP && piece <= WK);
            if ((board.whiteToMove && !isWhitePiece) ||
                (!board.whiteToMove && isWhitePiece)) {
                continue;
            }

            switch (piece) {
                case WP: {
                    // single step forward
                    if (r + 1 < BOARD_SIZE && board.squares[r+1][c] == EMPTY) {
                        moves.emplace_back(r, c, r+1, c);
                    }
                    // capture diagonals
                    if (r + 1 < BOARD_SIZE && c + 1 < BOARD_SIZE) {
                        if (board.squares[r+1][c+1] >= BP) {
                            moves.emplace_back(r, c, r+1, c+1);
                        }
                    }
                    if (r + 1 < BOARD_SIZE && c - 1 >= 0) {
                        if (board.squares[r+1][c-1] >= BP) {
                            moves.emplace_back(r, c, r+1, c-1);
                        }
                    }
                } break;

                case BP: {
                    if (r - 1 >= 0 && board.squares[r-1][c] == EMPTY) {
                        moves.emplace_back(r, c, r-1, c);
                    }
                    // capture diagonals
                    if (r - 1 >= 0 && c + 1 < BOARD_SIZE) {
                        if (board.squares[r-1][c+1] <= WK && board.squares[r-1][c+1] != EMPTY) {
                            moves.emplace_back(r, c, r-1, c+1);
                        }
                    }
                    if (r - 1 >= 0 && c - 1 >= 0) {
                        if (board.squares[r-1][c-1] <= WK && board.squares[r-1][c-1] != EMPTY) {
                            moves.emplace_back(r, c, r-1, c-1);
                        }
                    }
                } break;

                case WN:
                case BN: {
                    for (auto &offset : knightOffsets) {
                        int rr = r + offset[0];
                        int cc = c + offset[1];
                        if (rr >= 0 && rr < BOARD_SIZE && cc >= 0 && cc < BOARD_SIZE) {
                            Piece target = board.squares[rr][cc];
                            // Capture or empty square
                            if (isWhitePiece) {
                                if (target == EMPTY || (target >= BP && target <= BK)) {
                                    moves.emplace_back(r, c, rr, cc);
                                }
                            } else {
                                if (target == EMPTY || (target >= WP && target <= WK)) {
                                    moves.emplace_back(r, c, rr, cc);
                                }
                            }
                        }
                    }
                } break;

                // Rook, Bishop, Queen, King, etc. are not fully handled here
                default: break;
            }
        }
    }

    return moves;
}

// Make a move on the board (no validation of legality)
void ChessEngine::makeMove(Board &board, const Move &move) {
    Piece movingPiece = board.squares[move.fromRow][move.fromCol];
    board.squares[move.fromRow][move.fromCol] = EMPTY;
    board.squares[move.toRow][move.toCol] = movingPiece;

    // Pawn promotion example
    if (movingPiece == WP && move.toRow == 7) {
        board.squares[move.toRow][move.toCol] = WQ; // auto-promote to queen
    }
    if (movingPiece == BP && move.toRow == 0) {
        board.squares[move.toRow][move.toCol] = BQ;
    }

    board.whiteToMove = !board.whiteToMove;
}

// Undo the move
void ChessEngine::undoMove(Board &board, const Move &move, Piece captured) {
    Piece movingPiece = board.squares[move.toRow][move.toCol];
    board.squares[move.toRow][move.toCol] = captured;
    board.squares[move.fromRow][move.fromCol] = movingPiece;
    board.whiteToMove = !board.whiteToMove;
}

// A simple evaluation function: sum of piece values
int ChessEngine::evaluate(const Board &board) {
    // Very simplistic piece values
    static const int pieceValue[] = {
        0,    // EMPTY
        100,  // WP
        300,  // WN
        300,  // WB
        500,  // WR
        900,  // WQ
        99999,// WK
        -100, // BP
        -300, // BN
        -300, // BB
        -500, // BR
        -900, // BQ
        -99999// BK
    };

    int score = 0;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            score += pieceValue[board.squares[r][c]];
        }
    }
    // If it's white to move, the score is as-is,
    // if black to move, we can invert the sign or not.
    // Some engines do (whiteToMove ? score : -score).
    return board.whiteToMove ? score : -score;
}

// Iterative deepening framework around alpha-beta
Move ChessEngine::findBestMove(Board &board, int depth) {
    // We simply do an alpha-beta from 1..depth
    // and return the best move we found at the highest iteration.
    Move bestMove(0,0,0,0);
    int bestScore = -INFINITY_SCORE;

    std::vector<Move> moves = generateMoves(board);
    if (moves.empty()) {
        // No moves - might be checkmate or stalemate
        return bestMove;
    }

    for (int d = 1; d <= depth; ++d) {
        int localBestScore = -INFINITY_SCORE;
        Move localBestMove = moves[0];

        for (auto &m : moves) {
            Piece captured = board.squares[m.toRow][m.toCol];
            makeMove(board, m);
            int score = alphaBeta(board, -INFINITY_SCORE, INFINITY_SCORE, d-1, !board.whiteToMove);
            undoMove(board, m, captured);

            if (score > localBestScore) {
                localBestScore = score;
                localBestMove = m;
            }
        }
        bestScore = localBestScore;
        bestMove = localBestMove;

        // A time cutoff can be implemented here
    }

    return bestMove;
}

// Alpha-Beta search
int ChessEngine::alphaBeta(Board &board, int alpha, int beta, int depth, bool maximizingPlayer) {
    if (depth == 0) {
        // Switch to quiescence or direct evaluation
        return evaluate(board);
    }

    // Transposition Table: check if position is in TT
    uint64_t hash = computeZobristHash(board);
    TTKey ttKey{hash, depth};
    auto it = tTable.find(ttKey);
    if (it != tTable.end()) {
        // If found and depth is enough, can return stored result
        TTEntry &entry = it->second;
        if (entry.depth >= depth) {
            if (entry.flag == 0)      return entry.score;       // exact
            else if (entry.flag == -1) alpha = std::max(alpha, entry.score); 
            else if (entry.flag == 1)  beta  = std::min(beta,  entry.score);
            if (alpha >= beta) return entry.score;
        }
    }

    std::vector<Move> moves = generateMoves(board);
    if (moves.empty()) {
        // No moves -> could be checkmate or stalemate
        // For simplicity, assume checkmate if no moves
        return maximizingPlayer ? -MATE_SCORE : MATE_SCORE;
    }

    int bestValue = maximizingPlayer ? -INFINITY_SCORE : INFINITY_SCORE;

    for (auto &m : moves) {
        Piece captured = board.squares[m.toRow][m.toCol];
        makeMove(board, m);
        int value = alphaBeta(board, alpha, beta, depth - 1, !maximizingPlayer);
        undoMove(board, m, captured);

        if (maximizingPlayer) {
            bestValue = std::max(bestValue, value);
            alpha = std::max(alpha, bestValue);
            if (alpha >= beta) {
                break; 
            }
        } else {
            bestValue = std::min(bestValue, value);
            beta = std::min(beta, bestValue);
            if (beta <= alpha) {
                break; 
            }
        }
    }

    // Store into transposition table
    TTEntry newEntry;
    newEntry.score = bestValue;
    newEntry.depth = depth;
    if (bestValue <= alpha) {
        newEntry.flag = 1; 
    } else if (bestValue >= beta) {
        newEntry.flag = -1; 
    } else {
        newEntry.flag = 0; 
    }
    tTable[ttKey] = newEntry;

    return bestValue;
}
