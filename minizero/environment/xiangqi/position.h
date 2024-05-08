#pragma once
#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include <cassert>
#include <cstring>
#include <deque>
#include <iosfwd>
#include <memory>
#include <stdint.h>
#include <string>
#include <utility>

#include "bitboard.h"
#include "types.h"

namespace minizero::env::xiangqi {

const uint8_t MateThreatDepth = 0;
const bool ChineseRule = false;

// FEN string for the initial position in standard xiangqi
const std::string StartFEN = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";

std::string square(Square s);
Square square(std::string s);

// StateInfo struct stores information needed to restore a Position object to
// its previous state when we retract a move. Whenever a move is made on the
// board (by calling Position::do_move), a StateInfo object must be passed.

struct StateInfo {
    // Copied when making a move
    Key pawnKey;
    Value majorMaterial[COLOR_NB];
    int16_t check10[COLOR_NB];
    int rule60;
    int pliesFromNull;

    // Not copied when making a move (will be recomputed anyhow)
    Key key;
    Bitboard checkersBB;
    StateInfo* previous;
    Bitboard blockersForKing[COLOR_NB];
    Bitboard pinners[COLOR_NB];
    Bitboard checkSquares[PIECE_TYPE_NB];
    bool needSlowCheck;
    Piece capturedPiece;
    Move move;
};

class Position {
public:
    static void init();

    Position() = default;
    Position(const Position&) = delete;
    Position& operator=(const Position&) = delete;

    // destruct
    void destruct_until(const StateInfo* si);

    // FEN string input/output
    Position& set(const std::string& fenStr, StateInfo* si);
    Position& set(const Position& pos, StateInfo* si);
    std::string fen() const;

    // Position representation
    Bitboard pieces(PieceType pt = ALL_PIECES) const;
    template <typename... PieceTypes>
    Bitboard pieces(PieceType pt, PieceTypes... pts) const;
    Bitboard pieces(Color c) const;
    template <typename... PieceTypes>
    Bitboard pieces(Color c, PieceTypes... pts) const;
    Piece piece_on(Square s) const;
    bool empty(Square s) const;
    template <PieceType Pt>
    int count(Color c) const;
    template <PieceType Pt>
    int count() const;
    template <PieceType Pt>
    Square square(Color c) const;

    // Checking
    Bitboard checkers() const;
    Bitboard blockers_for_king(Color c) const;
    Bitboard check_squares(PieceType pt) const;
    Bitboard pinners(Color c) const;

    // Attacks to/from a given square
    Bitboard attackers_to(Square s) const;
    Bitboard attackers_to(Square s, Bitboard occupied) const;
    template <Color c>
    void update_blockers() const;
    Bitboard checkers_to(Color c, Square s) const;
    Bitboard checkers_to(Color c, Square s, Bitboard occupied) const;
    template <PieceType Pt>
    Bitboard attacks_by(Color c) const;

    // Properties of moves
    bool legal(Move m) const;
    bool pseudo_legal(const Move m) const;
    bool capture(Move m) const;
    bool gives_check(Move m) const;
    Piece moved_piece(Move m) const;
    Piece captured_piece() const;

    // Doing and undoing moves
    void do_move(Move m, StateInfo& newSt);
    void do_move(Move m, StateInfo& newSt, bool givesCheck);
    void undo_move(Move m);
    void do_null_move(StateInfo& newSt);
    void undo_null_move();

    // Accessing hash keys
    Key key() const;
    Key key_after(Move m) const;
    Key pawn_key() const;

    // Other properties of the position
    Color side_to_move() const;
    int game_ply() const;
    bool rule_judge(Value& result, int ply = 0) const;
    int repetition_count() const;
    int rule60_count() const;
    bool has_mate_threat(Depth d = -1);
    uint16_t chased(Color c);
    Value major_material(Color c) const;
    Value major_material() const;

    // Position consistency check, for debugging
    bool pos_is_ok() const;
    void flip();

    // Used by NNUE
    StateInfo* state() const;

    void put_piece(Piece pc, Square s);
    void remove_piece(Square s);

private:
    // Initialization helpers (used while setting up a position)
    void set_state() const;
    void set_check_info() const;

    // Other helpers
    void move_piece(Square from, Square to);
    std::pair<Piece, int> light_do_move(Move m);
    void light_undo_move(Move m, Piece captured, int id = 0);
    Value detect_chases(int d, int ply = 0);
    bool chase_legal(Move m) const;
    template <bool AfterMove>
    Key adjust_key60(Key k) const;

    // Data members
    Piece board[SQUARE_NB];
    Bitboard byTypeBB[PIECE_TYPE_NB];
    Bitboard byColorBB[COLOR_NB];
    int pieceCount[PIECE_NB];
    StateInfo* st = nullptr;
    int gamePly;
    Color sideToMove;

    // Bloom filter for fast repetition filtering
    BloomFilter filter;

    // Board for chasing detection
    int idBoard[SQUARE_NB];
};

std::ostream& operator<<(std::ostream& os, const Position& pos);

inline Color Position::side_to_move() const { return sideToMove; }

inline Piece Position::piece_on(Square s) const
{
    assert(is_ok(s));
    return board[s];
}

inline bool Position::empty(Square s) const { return piece_on(s) == NO_PIECE; }

inline Piece Position::moved_piece(Move m) const { return piece_on(from_sq(m)); }

inline Bitboard Position::pieces(PieceType pt) const { return byTypeBB[pt]; }

template <typename... PieceTypes>
inline Bitboard Position::pieces(PieceType pt, PieceTypes... pts) const
{
    return pieces(pt) | pieces(pts...);
}

inline Bitboard Position::pieces(Color c) const { return byColorBB[c]; }

template <typename... PieceTypes>
inline Bitboard Position::pieces(Color c, PieceTypes... pts) const
{
    return pieces(c) & pieces(pts...);
}

template <PieceType Pt>
inline int Position::count(Color c) const
{
    return pieceCount[make_piece(c, Pt)];
}

template <PieceType Pt>
inline int Position::count() const
{
    return count<Pt>(WHITE) + count<Pt>(BLACK);
}

template <PieceType Pt>
inline Square Position::square(Color c) const
{
    assert(count<Pt>(c) == 1);
    return lsb(pieces(c, Pt));
}

inline Bitboard Position::attackers_to(Square s) const { return attackers_to(s, pieces()); }

inline Bitboard Position::checkers_to(Color c, Square s) const
{
    return checkers_to(c, s, pieces());
}

template <PieceType Pt>
inline Bitboard Position::attacks_by(Color c) const
{
    Bitboard threats = 0;
    Bitboard attackers = pieces(c, Pt);
    while (attackers)
        if (Pt == PAWN)
            threats |= pawn_attacks_bb(c, pop_lsb(attackers));
        else
            threats |= attacks_bb<Pt>(pop_lsb(attackers), pieces());
    return threats;
}

inline Bitboard Position::checkers() const { return st->checkersBB; }

inline Bitboard Position::blockers_for_king(Color c) const { return st->blockersForKing[c]; }

inline Bitboard Position::pinners(Color c) const { return st->pinners[c]; }

inline Bitboard Position::check_squares(PieceType pt) const { return st->checkSquares[pt]; }

inline Key Position::key() const { return adjust_key60<false>(st->key); }

template <bool AfterMove>
inline Key Position::adjust_key60(Key k) const
{
    return st->rule60 < 14 - AfterMove ? k : k ^ make_key((st->rule60 - (14 - AfterMove)) / 8);
}

inline Key Position::pawn_key() const { return st->pawnKey; }

inline Value Position::major_material(Color c) const { return st->majorMaterial[c]; }

inline Value Position::major_material() const
{
    return major_material(WHITE) + major_material(BLACK);
}

inline int Position::game_ply() const { return gamePly; }

inline int Position::rule60_count() const { return st->rule60; }

inline bool Position::capture(Move m) const
{
    assert(is_ok(m));
    return !empty(to_sq(m));
}

inline Piece Position::captured_piece() const { return st->capturedPiece; }

inline void Position::put_piece(Piece pc, Square s)
{
    board[s] = pc;
    byTypeBB[ALL_PIECES] |= byTypeBB[type_of(pc)] |= s;
    byColorBB[color_of(pc)] |= s;
    pieceCount[pc]++;
    pieceCount[make_piece(color_of(pc), ALL_PIECES)]++;
}

inline void Position::remove_piece(Square s)
{
    Piece pc = board[s];
    byTypeBB[ALL_PIECES] ^= s;
    byTypeBB[type_of(pc)] ^= s;
    byColorBB[color_of(pc)] ^= s;
    board[s] = NO_PIECE;
    pieceCount[pc]--;
    pieceCount[make_piece(color_of(pc), ALL_PIECES)]--;
}

inline void Position::move_piece(Square from, Square to)
{
    Piece pc = board[from];
    Bitboard fromTo = from | to;
    byTypeBB[ALL_PIECES] ^= fromTo;
    byTypeBB[type_of(pc)] ^= fromTo;
    byColorBB[color_of(pc)] ^= fromTo;
    board[from] = NO_PIECE;
    board[to] = pc;
}

inline void Position::do_move(Move m, StateInfo& newSt) { do_move(m, newSt, gives_check(m)); }

inline StateInfo* Position::state() const { return st; }

inline Position& Position::set(const Position& pos, StateInfo* si)
{
    set(pos.fen(), si);

    // Special cares for bloom filter
    std::memcpy(&filter, &pos.filter, sizeof(BloomFilter));

    return *this;
}

} // namespace minizero::env::xiangqi

#endif // #ifndef POSITION_H_INCLUDED
