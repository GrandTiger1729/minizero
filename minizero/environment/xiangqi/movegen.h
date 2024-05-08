#pragma once
#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include <algorithm> // IWYU pragma: keep
#include <cstddef>

#include "types.h"

namespace minizero::env::xiangqi {

class Position;

enum GenType {
    CAPTURES,
    QUIETS,
    QUIET_CHECKS,
    EVASIONS,
    PSEUDO_LEGAL,
    LEGAL
};

struct ExtMove {
    Move move;
    int value;

    operator Move() const { return move; }
    void operator=(Move m) { move = m; }

    // Inhibit unwanted implicit conversions to Move
    // with an ambiguity that yields to a compile error.
    operator float() const = delete;
};

inline bool operator<(const ExtMove& f, const ExtMove& s) { return f.value < s.value; }

template <GenType>
ExtMove* generate(const Position& pos, ExtMove* moveList);

// The MoveList struct wraps the generate() function and returns a convenient
// list of moves. Using MoveList is sometimes preferable to directly calling
// the lower level generate() function.
template <GenType T>
struct MoveList {
    explicit MoveList(const Position& pos) : last(generate<T>(pos, moveList)) {}
    const ExtMove* begin() const { return moveList; }
    const ExtMove* end() const { return last; }
    size_t size() const { return last - moveList; }
    bool contains(Move move) const { return std::find(begin(), end(), move) != end(); }

private:
    ExtMove moveList[MAX_MOVES], *last;
};

} // namespace minizero::env::xiangqi

#endif // #ifndef MOVEGEN_H_INCLUDED
