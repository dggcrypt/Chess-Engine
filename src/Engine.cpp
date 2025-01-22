#include "Engine.h"
#include <random>
#include <algorithm>
#include <cmath>


static const int pawnTable[64] = {
     0,  0,  0,   0,   0,  0,  0,  0,
     5,  5,  5,  -5,  -5,  0,  5,  5,
     1,  1,  1,   5,   5,  0,  1,  1,
     0,  0, 10,  20,  20, 10,  0,  0,
     5,  5,  5,   5,   5,  5,  5,  5,
    10, 10, 10,  20,  20, 10, 10, 10,
    50, 50, 50,  40,  40, 50, 50, 50,
     0,  0,  0,   0,   0,  0,  0,  0
};

static const int knightTable[64] = {
  -50,-40,-30,-30,-30,-30,-40,-50,
  -40,-20,  0,  5,  5,  0,-20,-40,
  -30,  5, 10, 15, 15, 10,  5,-30,
  -30,  0, 15, 20, 20, 15,  0,-30,
  -30,  5, 15, 20, 20, 15,  5,-30,
  -30,  0, 10, 15, 15, 10,  0,-30,
  -40,-20,  0,  0,  0,  0,-20,-40,
  -50,-40,-30,-30,-30,-30,-40,-50
};

// Basic piece values
static const int pieceValue[] = {
    0,      // EMPTY
    100,    // WP
    300,    // WN
    300,    // WB
    500,    // WR
    900,    // WQ
    99999,  // WK
   -100,    // BP
   -300,    // BN
   -300,    // BB
   -500,    // BR
   -900,    // BQ
   -99999   // BK
};

ChessEngine::ChessEngine() {
    initZobristTable();
}


void ChessEngine::initZobristTable() {
    std::mt19937_64 rng(0xDEADBEAF12345678ULL); 
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            for (int p = 0; p < 14; p++) {
                zobristTable[r][c][p] = rng();
            }
        }
    }
}

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
    
    if (!board.whiteToMove) {
        
        h = ~h;
    }
    return h;
}


void ChessEngine::initBoard(Board &board) {
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            board.squares[r][c] = EMPTY;
        }
    }
    board.whiteToMove = true;

    // Pawns
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
}



// Directions for sliding pieces
static const int rookOffsets[4][2]   = {{1,0}, {-1,0}, {0,1}, {0,-1}};
static const int bishopOffsets[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
static const int kingOffsets[8][2]   = {{1,0},{1,1},{1,-1},{0,1},{0,-1},{-1,0},{-1,1},{-1,-1}};
static const int knightOffsets[8][2] = {
    {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
    {-1, -2}, {-1, 2}, {1, -2}, {1, 2}
};

std::vector<Move> ChessEngine::generatePseudoLegalMoves(const Board &board) {
    std::vector<Move> moves;

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            Piece piece = board.squares[r][c];
            if (piece == EMPTY) continue;

            bool isWhitePiece = (piece >= WP && piece <= WK);
            if (isWhitePiece != board.whiteToMove) continue;

            switch (piece) {
                
                case WP: {
                    // Forward one
                    if (r+1 < 8 && board.squares[r+1][c] == EMPTY) {
                        
                        if (r+1 == 7) {
                            moves.emplace_back(r, c, r+1, c, WQ);
                            moves.emplace_back(r, c, r+1, c, WR);
                            moves.emplace_back(r, c, r+1, c, WB);
                            moves.emplace_back(r, c, r+1, c, WN);
                        } else {
                            moves.emplace_back(r, c, r+1, c);
                        }
                    }
                    // Captures
                    if (r+1 < 8 && c+1 < 8 && board.squares[r+1][c+1] >= BP) {
                        if (r+1 == 7) {
                            moves.emplace_back(r, c, r+1, c+1, WQ);
                            moves.emplace_back(r, c, r+1, c+1, WR);
                            moves.emplace_back(r, c, r+1, c+1, WB);
                            moves.emplace_back(r, c, r+1, c+1, WN);
                        } else {
                            moves.emplace_back(r, c, r+1, c+1);
                        }
                    }
                    if (r+1 < 8 && c-1 >= 0 && board.squares[r+1][c-1] >= BP) {
                        if (r+1 == 7) {
                            moves.emplace_back(r, c, r+1, c-1, WQ);
                            moves.emplace_back(r, c, r+1, c-1, WR);
                            moves.emplace_back(r, c, r+1, c-1, WB);
                            moves.emplace_back(r, c, r+1, c-1, WN);
                        } else {
                            moves.emplace_back(r, c, r+1, c-1);
                        }
                    }
                    
                } break;
                case BP: {
                    // Forward one
                    if (r-1 >= 0 && board.squares[r-1][c] == EMPTY) {
                        
                        if (r-1 == 0) {
                            moves.emplace_back(r, c, r-1, c, BQ);
                            moves.emplace_back(r, c, r-1, c, BR);
                            moves.emplace_back(r, c, r-1, c, BB);
                            moves.emplace_back(r, c, r-1, c, BN);
                        } else {
                            moves.emplace_back(r, c, r-1, c);
                        }
                    }
                    // Captures
                    if (r-1 >= 0 && c+1 < 8 && board.squares[r-1][c+1] <= WK && board.squares[r-1][c+1] != EMPTY) {
                        if (r-1 == 0) {
                            moves.emplace_back(r, c, r-1, c+1, BQ);
                            moves.emplace_back(r, c, r-1, c+1, BR);
                            moves.emplace_back(r, c, r-1, c+1, BB);
                            moves.emplace_back(r, c, r-1, c+1, BN);
                        } else {
                            moves.emplace_back(r, c, r-1, c+1);
                        }
                    }
                    if (r-1 >= 0 && c-1 >= 0 && board.squares[r-1][c-1] <= WK && board.squares[r-1][c-1] != EMPTY) {
                        if (r-1 == 0) {
                            moves.emplace_back(r, c, r-1, c-1, BQ);
                            moves.emplace_back(r, c, r-1, c-1, BR);
                            moves.emplace_back(r, c, r-1, c-1, BB);
                            moves.emplace_back(r, c, r-1, c-1, BN);
                        } else {
                            moves.emplace_back(r, c, r-1, c-1);
                        }
                    }
                } break;

                
                case WN: case BN: {
                    for (auto &offset : knightOffsets) {
                        int rr = r + offset[0];
                        int cc = c + offset[1];
                        if (rr >= 0 && rr < 8 && cc >= 0 && cc < 8) {
                            Piece target = board.squares[rr][cc];
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
                case WK: case BK: {
                    for (auto &offset : kingOffsets) {
                        int rr = r + offset[0];
                        int cc = c + offset[1];
                        if (rr >= 0 && rr < 8 && cc >= 0 && cc < 8) {
                            Piece target = board.squares[rr][cc];
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

                
                case WR: case BR: {
                    int steps = 4; // rook offsets
                    for (int i = 0; i < steps; ++i) {
                        int rr = r, cc = c;
                        while (true) {
                            rr += rookOffsets[i][0];
                            cc += rookOffsets[i][1];
                            if (rr < 0 || rr >= 8 || cc < 0 || cc >= 8) break;
                            Piece target = board.squares[rr][cc];
                            if (target == EMPTY) {
                                moves.emplace_back(r,c, rr,cc);
                            } else {
                                bool isOppositeColor = (isWhitePiece && target >= BP && target <= BK)
                                                   || (!isWhitePiece && target >= WP && target <= WK);
                                if (isOppositeColor) {
                                    moves.emplace_back(r,c, rr,cc);
                                }
                                break; // cannot jump
                            }
                        }
                    }
                } break;
                case WB: case BB: {
                    int steps = 4; // bishop offsets
                    for (int i = 0; i < steps; ++i) {
                        int rr = r, cc = c;
                        while (true) {
                            rr += bishopOffsets[i][0];
                            cc += bishopOffsets[i][1];
                            if (rr < 0 || rr >= 8 || cc < 0 || cc >= 8) break;
                            Piece target = board.squares[rr][cc];
                            if (target == EMPTY) {
                                moves.emplace_back(r,c, rr,cc);
                            } else {
                                bool isOppositeColor = (isWhitePiece && target >= BP && target <= BK)
                                                   || (!isWhitePiece && target >= WP && target <= WK);
                                if (isOppositeColor) {
                                    moves.emplace_back(r,c, rr,cc);
                                }
                                break;
                            }
                        }
                    }
                } break;
                case WQ: case BQ: {
                    
                    int steps = 4;
                    // Rook-like
                    for (int i = 0; i < steps; ++i) {
                        int rr = r, cc = c;
                        while (true) {
                            rr += rookOffsets[i][0];
                            cc += rookOffsets[i][1];
                            if (rr < 0 || rr >= 8 || cc < 0 || cc >= 8) break;
                            Piece target = board.squares[rr][cc];
                            if (target == EMPTY) {
                                moves.emplace_back(r,c, rr,cc);
                            } else {
                                bool isOppositeColor = (isWhitePiece && target >= BP && target <= BK)
                                                   || (!isWhitePiece && target >= WP && target <= WK);
                                if (isOppositeColor) {
                                    moves.emplace_back(r,c, rr,cc);
                                }
                                break;
                            }
                        }
                    }
                    // Bishop-like
                    for (int i = 0; i < steps; ++i) {
                        int rr = r, cc = c;
                        while (true) {
                            rr += bishopOffsets[i][0];
                            cc += bishopOffsets[i][1];
                            if (rr < 0 || rr >= 8 || cc < 0 || cc >= 8) break;
                            Piece target = board.squares[rr][cc];
                            if (target == EMPTY) {
                                moves.emplace_back(r,c, rr,cc);
                            } else {
                                bool isOppositeColor = (isWhitePiece && target >= BP && target <= BK)
                                                   || (!isWhitePiece && target >= WP && target <= WK);
                                if (isOppositeColor) {
                                    moves.emplace_back(r,c, rr,cc);
                                }
                                break;
                            }
                        }
                    }
                } break;
                default: break;
            }
        }
    }
    return moves;
}


std::vector<Move> ChessEngine::generateLegalMoves(const Board &board) {
    std::vector<Move> pseudo = generatePseudoLegalMoves(board);
    std::vector<Move> legal;
    // For each move, make it, check if king is in check
    for (auto &m : pseudo) {
        Board copy = board;
        Piece captured = copy.squares[m.toRow][m.toCol];
        makeMove(copy, m);
        
        bool whiteKing = copy.whiteToMove ? false : true; 
        if (!isKingInCheck(copy, whiteKing)) {
            legal.push_back(m);
        }
    }
    return legal;
}


bool ChessEngine::isKingInCheck(const Board &board, bool whiteKing) {
    
    Piece kingPiece = whiteKing ? WK : BK;
    int kingR = -1, kingC = -1;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (board.squares[r][c] == kingPiece) {
                kingR = r; kingC = c;
                break;
            }
        }
        if (kingR != -1) break;
    }
    if (kingR == -1) {
        
        return true;
    }

   
    bool enemySideIsWhite = !whiteKing;
    
    Board temp = board;
    temp.whiteToMove = enemySideIsWhite; 
    auto enemyMoves = generatePseudoLegalMoves(temp);
    for (auto &mv : enemyMoves) {
        if (mv.toRow == kingR && mv.toCol == kingC) {
            return true;
        }
    }
    return false;
}


void ChessEngine::makeMove(Board &board, const Move &move) {
    Piece movingPiece = board.squares[move.fromRow][move.fromCol];
    Piece capturedPiece = board.squares[move.toRow][move.toCol];

    board.squares[move.toRow][move.toCol] = movingPiece;
    board.squares[move.fromRow][move.fromCol] = EMPTY;

    // Handle promotion
    if (move.promotion != EMPTY) {
        board.squares[move.toRow][move.toCol] = move.promotion;
    }

    // Switch side
    board.whiteToMove = !board.whiteToMove;
}

void ChessEngine::undoMove(Board &board, const Move &move, Piece captured) {
    Piece movingPiece = board.squares[move.toRow][move.toCol];
    board.squares[move.fromRow][move.fromCol] = movingPiece;
    board.squares[move.toRow][move.toCol] = captured;
    board.whiteToMove = !board.whiteToMove;
}


int ChessEngine::evaluate(const Board &board) {
    

    int score = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Piece p = board.squares[r][c];
            if (p == EMPTY) continue;

            int idx64 = r*8 + c; 

            int val = pieceValue[p];
            switch (p) {
                case WP:
                    val += pawnTable[idx64];
                    break;
                case WN:
                    val += knightTable[idx64];
                    break;
                case WB:
                    
                    break;
                case WQ:
                    
                    break;
                case WR:
                    
                    break;
                case WK:
                    
                    break;
                case BP:
                    
                    break;
                case BN:
                    
                    break;
                
                default: break;
            }

            score += val;
        }
    }

    
    if (!board.whiteToMove) {
        score = -score;
    }
    return score;
}


int ChessEngine::quiescenceSearch(Board &board, int alpha, int beta) {
    // Evaluate current position
    int standPat = evaluate(board);
    if (standPat >= beta) {
        return beta;
    }
    if (standPat > alpha) {
        alpha = standPat;
    }

    
    std::vector<Move> moves;
    {
        auto pseudo = generatePseudoLegalMoves(board);
        
        for (auto &m: pseudo) {
            if (board.squares[m.toRow][m.toCol] != EMPTY) {
                moves.push_back(m);
            }
        }
    }

    
    sortMoves(board, moves);

    for (auto &m : moves) {
        Piece captured = board.squares[m.toRow][m.toCol];
        makeMove(board, m);
        if (!isKingInCheck(board, board.whiteToMove ? false : true)) {
            int score = -quiescenceSearch(board, -beta, -alpha);
            undoMove(board, m, captured);

            if (score >= beta) {
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        } else {
            undoMove(board, m, captured);
        }
    }
    return alpha;
}


int ChessEngine::alphaBeta(Board &board, int alpha, int beta, int depth, bool doNullMove) {
    
    if (timeIsUp()) {
        return evaluate(board);
    }

    if (depth == 0) {
        
        return quiescenceSearch(board, alpha, beta);
    }

    uint64_t hash = computeZobristHash(board);
    TTKey key{hash, depth};

    
    auto it = tTable.find(key);
    if (it != tTable.end()) {
        TTEntry &entry = it->second;
        if (entry.depth >= depth) {
            if (entry.flag == 0)  return entry.score;       
            if (entry.flag == -1) alpha = std::max(alpha, entry.score); 
            if (entry.flag ==  1) beta  = std::min(beta,  entry.score); 
            if (alpha >= beta) {
                return entry.score;
            }
        }
    }

    // Generate legal moves
    std::vector<Move> moves = generateLegalMoves(board);
    if (moves.empty()) {
        // no moves => checkmate or stalemate
        if (isKingInCheck(board, !board.whiteToMove)) {
            // checkmate
            return -MATE_SCORE + (MAX_DEPTH - depth);
        } else {
            // stalemate
            return 0;
        }
    }

    // Move ordering
    sortMoves(board, moves);

    bool isPV = false;
    int bestValue = -INFINITY_SCORE;
    for (auto &m : moves) {
        Piece captured = board.squares[m.toRow][m.toCol];
        makeMove(board, m);
        int score = -alphaBeta(board, -beta, -alpha, depth - 1);
        undoMove(board, m, captured);

        if (score > bestValue) {
            bestValue = score;
            if (score > alpha) {
                alpha = score;
                isPV = true;
                if (alpha >= beta) {
                    break; 
                }
            }
        }
    }

    
    TTEntry newEntry;
    newEntry.score = bestValue;
    newEntry.depth = depth;
    if (bestValue <= alpha) {
       
        if (bestValue <= alpha) newEntry.flag = -1; // alpha
        if (bestValue >= beta)  newEntry.flag =  1; // beta
        if (bestValue > alpha && bestValue < beta) newEntry.flag = 0; // exact
    }
    else {
        
        if (bestValue <= alpha) newEntry.flag = -1;
        else if (bestValue >= beta) newEntry.flag = 1;
        else newEntry.flag = 0;
    }
    tTable[key] = newEntry;

    return bestValue;
}

// Search from root to get best move
int ChessEngine::searchRoot(Board &board, int depth, Move &bestMove) {
    int alpha = -INFINITY_SCORE;
    int beta  =  INFINITY_SCORE;

    int bestScore = -INFINITY_SCORE;
    auto moves = generateLegalMoves(board);

    // Move ordering
    sortMoves(board, moves);

    bool foundMove = false;
    for (auto &m : moves) {
        Piece captured = board.squares[m.toRow][m.toCol];
        makeMove(board, m);
        int score = -alphaBeta(board, -beta, -alpha, depth - 1);
        undoMove(board, m, captured);

        if (score > bestScore) {
            bestScore = score;
            bestMove = m;
            foundMove = true;
            if (score > alpha) {
                alpha = score;
                if (alpha >= beta) {
                    break; // cutoff
                }
            }
        }
        if (timeIsUp()) break;
    }
    
    return bestScore;
}


Move ChessEngine::findBestMove(Board &board, int maxDepth, double timeLimit) {
    timeLimitSec = timeLimit;
    startTime = std::chrono::steady_clock::now();

    Move bestMove;
    // Iterative deepening
    for (int depth = 1; depth <= maxDepth; ++depth) {
        if (timeIsUp()) break; 
        Move localBest;
        int score = searchRoot(board, depth, localBest);
        if (!timeIsUp()) {
            bestMove = localBest; 
        } 
        else {
            break; 
        }
    }
    return bestMove;
}


bool ChessEngine::timeIsUp() {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - startTime).count();
    return (elapsed >= timeLimitSec);
}


static int mvvLvaScore(Piece attacker, Piece victim) {
    // "Most valuable victim, least valuable attacker"
    // Higher = higher priority
    int victimVal = abs(pieceValue[victim]);
    int attackerVal = abs(pieceValue[attacker]);
    return 10 * victimVal - attackerVal; 
}

void ChessEngine::sortMoves(Board &board, std::vector<Move> &moves) {
    // Sort captures first by MVV-LVA, then non-captures
    std::sort(moves.begin(), moves.end(), [&](const Move &a, const Move &b){
        Piece aAtt = board.squares[a.fromRow][a.fromCol];
        Piece aVic = board.squares[a.toRow][a.toCol];
        Piece bAtt = board.squares[b.fromRow][b.fromCol];
        Piece bVic = board.squares[b.toRow][b.toCol];

        bool aIsCapture = (aVic != EMPTY);
        bool bIsCapture = (bVic != EMPTY);

        if (aIsCapture && !bIsCapture) return true;
        if (!aIsCapture && bIsCapture) return false;
        if (aIsCapture && bIsCapture) {
            // compare MVV-LVA
            int scoreA = mvvLvaScore(aAtt, aVic);
            int scoreB = mvvLvaScore(bAtt, bVic);
            return scoreA > scoreB;
        }
        // neither capture
        return false;
    });
}
