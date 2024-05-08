#include "xiangqi.h"
#include "random.h"
#include "sgf_loader.h"
#include <algorithm>
#include <bitset>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace minizero::env::xiangqi {
using namespace minizero::utils;

int to_actionID[kMaxMoves];
Move to_move[kValidActions];

void initialize()
{
    Bitboards::init();
    initMappings();
    Position::init();
}

void initMappings()
{
    int t = 1;
    auto make = [&](Square s1, Bitboard bb) {
        while (bb) {
            Square s2 = pop_lsb(bb);
            assert(s1 != s2);
            to_actionID[make_move(s1, s2)] = t;
            to_move[t] = make_move(s1, s2);
            ++t;
        }
    };
    for (Square s1 = SQ_A0; s1 <= SQ_I9; ++s1) {
        make(s1, attacks_bb<ROOK>(s1));
        make(s1, attacks_bb<ADVISOR>(s1));
        make(s1, attacks_bb<KNIGHT>(s1));
        make(s1, attacks_bb<BISHOP>(s1));
    }
}

XiangqiEnv::XiangqiEnv(const XiangqiEnv& env)
{
    board_size_ = env.board_size_;
    turn_ = env.turn_;

    memcpy(reinterpret_cast<void*>(&pos_), (const void*)&env.pos_, sizeof(Position));
    rootBeforeCoppied = pos_.state();
}
XiangqiEnv& XiangqiEnv::operator=(const XiangqiEnv& env)
{
    *this = XiangqiEnv(env);
    return *this;
}
XiangqiEnv::~XiangqiEnv()
{
    pos_.destruct_until(rootBeforeCoppied);
}

void XiangqiEnv::reset()
{
    StateInfo* st = new StateInfo();
    pos_.destruct_until(nullptr);
    pos_.set(StartFEN, st);

    turn_ = Player::kPlayer1;
    actions_.clear();
}
bool XiangqiEnv::act(const XiangqiAction& action)
{
    // std::cerr << action.getActionID() << ' ' << to_move[action.getActionID()] << std::endl;
    if (!isLegalAction(action)) { return false; }
    actions_.push_back(action);
    Move move = action.getMove();
    StateInfo* st = new StateInfo();
    pos_.do_move(move, *st);
    turn_ = action.nextPlayer();
    return true;
}
bool XiangqiEnv::act(const std::vector<std::string>& action_string_args)
{
    return act(XiangqiAction(action_string_args));
}
std::vector<XiangqiAction> XiangqiEnv::getLegalActions() const
{
    MoveList<LEGAL> moveList(pos_);
    std::vector<XiangqiAction> legalActions;
    for (auto& extMove : moveList) {
        legalActions.push_back(XiangqiAction(extMove.move, turn_));
    }
    return legalActions;
}
bool XiangqiEnv::isLegalAction(const XiangqiAction& action) const
{
    Move move = action.getMove();
    // std::cerr << "Move in is legal action: " << (int)move << std::endl;
    // std::cerr << "To actionID: " << to_actionID[move] << std::endl;
    if (!is_ok(move) || !is_ok(from_sq(move)) || !is_ok(to_sq(move))) {
        return false;
    }
    return pos_.pseudo_legal(move) && pos_.legal(move);
}
bool XiangqiEnv::isTerminal() const
{
    Value result;
    return pos_.rule_judge(result) || getLegalActions().empty();
}

float XiangqiEnv::getEvalScore(bool is_resign) const
{
    Player winner;
    Value result;
    if (pos_.rule_judge(result)) {
        if (result > 0) {
            winner = turn_;
        } else if (result < 0) {
            winner = getNextPlayer(turn_, kXiangqiNumPlayer);
        } else {
            winner = Player::kPlayerNone;
        }
    } else {
        winner = getNextPlayer(turn_, kXiangqiNumPlayer);
    }
    switch (winner) {
        case Player::kPlayer1: return 1.0f;
        case Player::kPlayer2: return -1.0f;
        default: return 0.0f;
    }
}

std::vector<float> XiangqiEnv::getFeatures(utils::Rotation rotation) const
{
    /* TM+L channels:
        T=8 set of channels
            0~6.  P1 piece
            7~13. P2 piece
            14~15. Repetitions
        L=5
            0. color
            1. total move count
            2. rule60 count
            3. P1 check10 count
            4. P2 check10 count
    */
    std::vector<float> features(kXiangqiBoardWidth * kXiangqiBoardHeight * kFeaturePlanesSize);
    auto encodeBitboard = [&](int base, Bitboard bb) -> void {
        for (int i = 0; i < kXiangqiBoardWidth * kXiangqiBoardHeight; i++) {
            features[base + i] = bb >> i & 1 ? 1.0f : 0.0f;
        }
    };
    Position rollback;
    memcpy(reinterpret_cast<void*>(&rollback), (const void*)&this->pos_, sizeof(Position));
    int base = kAuxPlaneBase;
    for (int i = 0; i < static_cast<int>(actions_.size()) && i < kMoveHistory && base > 0; i++) {
        base -= kPlanesPerBoard;

        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 0), rollback.pieces(Color::WHITE, PieceType::ROOK));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 1), rollback.pieces(Color::WHITE, PieceType::ADVISOR));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 2), rollback.pieces(Color::WHITE, PieceType::CANNON));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 3), rollback.pieces(Color::WHITE, PieceType::PAWN));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 4), rollback.pieces(Color::WHITE, PieceType::KNIGHT));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 5), rollback.pieces(Color::WHITE, PieceType::BISHOP));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 6), rollback.pieces(Color::WHITE, PieceType::KING));

        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 7), rollback.pieces(Color::BLACK, PieceType::ROOK));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 8), rollback.pieces(Color::BLACK, PieceType::ADVISOR));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 9), rollback.pieces(Color::BLACK, PieceType::CANNON));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 10), rollback.pieces(Color::BLACK, PieceType::PAWN));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 11), rollback.pieces(Color::BLACK, PieceType::KNIGHT));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 12), rollback.pieces(Color::BLACK, PieceType::BISHOP));
        encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 13), rollback.pieces(Color::BLACK, PieceType::KING));

        int repetitions = rollback.repetition_count();
        if (repetitions >= 1) {
            Bitboard bb;
            bb = (__uint128_t(1) << kXiangqiBoardWidth * kXiangqiBoardHeight) - 1;
            encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 14), bb);
        }
        if (repetitions >= 2) {
            Bitboard bb;
            bb = (__uint128_t(1) << kXiangqiBoardWidth * kXiangqiBoardHeight) - 1;
            encodeBitboard(kXiangqiBoardWidth * kXiangqiBoardHeight * (base + 15), bb);
        }

        rollback.undo_move(actions_.end()[-1 - i].getMove());
    }
    {
        for (int i = 0; i < kXiangqiBoardWidth * kXiangqiBoardHeight; i++) {
            features[kXiangqiBoardWidth * kXiangqiBoardHeight * (kAuxPlaneBase + 0) + i] = turn_ == Player::kPlayer1 ? 1.0f : 0.0f;
        }
        for (int i = 0; i < kXiangqiBoardWidth * kXiangqiBoardHeight; i++) {
            features[kXiangqiBoardWidth * kXiangqiBoardHeight * (kAuxPlaneBase + 1) + i] = static_cast<float>(actions_.size()) / 1000;
        }
        for (int i = 0; i < kXiangqiBoardWidth * kXiangqiBoardHeight; i++) {
            features[kXiangqiBoardWidth * kXiangqiBoardHeight * (kAuxPlaneBase + 2) + i] = static_cast<float>(pos_.rule60_count()) / 120;
        }
        for (int i = 0; i < kXiangqiBoardWidth * kXiangqiBoardHeight; i++) {
            features[kXiangqiBoardWidth * kXiangqiBoardHeight * (kAuxPlaneBase + 3) + i] = static_cast<float>(pos_.state()->check10[0]) / 100;
        }
        for (int i = 0; i < kXiangqiBoardWidth * kXiangqiBoardHeight; i++) {
            features[kXiangqiBoardWidth * kXiangqiBoardHeight * (kAuxPlaneBase + 4) + i] = static_cast<float>(pos_.state()->check10[1]) / 100;
        }
    }
    return features;
}

std::string XiangqiEnv::toString() const
{
    std::stringstream ss;
    ss << pos_;
    return ss.str();
}

} // namespace minizero::env::xiangqi
