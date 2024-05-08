#pragma once

#include "base_env.h"
#include "configuration.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "movegen.h"
#include "position.h"

namespace minizero::env::xiangqi {
using namespace minizero::utils;
const std::string kXiangqiName = "xiangqi";

const int kXiangqiNumPlayer = 2;
const int kXiangqiBoardWidth = 9;
const int kXiangqiBoardHeight = 10;
const int kXiangqiBoardSize = 10;

const int kMoveHistory = 8;
const int kPlanesPerBoard = 16;
const int kAuxPlaneBase = kPlanesPerBoard * kMoveHistory;
const int kSpecial = 5;
const int kFeaturePlanesSize = kAuxPlaneBase + kSpecial;

const int kMaxMoves = 1 << 14, kValidActions = 2239;
extern int to_actionID[kMaxMoves];
extern Move to_move[kValidActions];

void initialize();
void initMappings();

class XiangqiAction : public BaseBoardAction<kXiangqiNumPlayer> {
public:
    XiangqiAction() : BaseBoardAction<kXiangqiNumPlayer>() {}
    XiangqiAction(int action_id, Player player) : BaseBoardAction<kXiangqiNumPlayer>(action_id, player) {}
    XiangqiAction(const std::vector<std::string>& action_string_args, int board_size = minizero::config::env_board_size)
    {
        // std::cerr << "OK" << std::endl;
        // for (int i = 0; i < (1 << 14); i++)
        // {
        //     if (to_actionID[i] != 0)
        //         std::cerr << XiangqiAction(to_actionID[i], Player::kPlayer1).toConsoleString() << std::endl;
        // }
        // exit(0);
        assert(action_string_args.size() == 2);
        assert(action_string_args[0].size() == 1);
        player_ = charToPlayer(action_string_args[0][0]);
        assert(static_cast<int>(player_) > 0 && static_cast<int>(player_) <= kNumPlayer); // assume kPlayer1 == 1, kPlayer2 == 2, ...
        assert(action_string_args[1].size() == 4);                                        // 01 from square, 23 to square
        // action_id_ = std::stoi(action_string_args[1]);
        std::string move_string = action_string_args[1];
        // std::cerr << square(move_string.substr(0, 2)) << ' ' << square(move_string.substr(2, 2)) << std::endl;
        // std::cerr << (int)make_move(square(move_string.substr(0, 2)), square(move_string.substr(2, 2))) << std::endl;
        action_id_ = to_actionID[make_move(square(move_string.substr(0, 2)), square(move_string.substr(2, 2)))];
        // std::cerr << "Move: " << move_string << "\nActionID: " << action_id_ << std::endl;
        // std::cerr << toConsoleString() << std::endl;
    }
    XiangqiAction(Move move, Player player) : BaseBoardAction<kXiangqiNumPlayer>(to_actionID[move], player) {}
    Move getMove() const { return to_move[action_id_]; }
    inline std::string toConsoleString() const override
    {
        Move move = getMove();
        return square(from_sq(move)) + square(to_sq(move));
        // return std::to_string(action_id_);
    }
};

class XiangqiEnv : public BaseBoardEnv<XiangqiAction> {
public:
    XiangqiEnv() : BaseBoardEnv<XiangqiAction>(kXiangqiBoardSize)
    {
        initialize();
        reset();
    }
    XiangqiEnv(const XiangqiEnv& env);
    XiangqiEnv& operator=(const XiangqiEnv& env);
    ~XiangqiEnv();

    void reset() override;
    bool act(const XiangqiAction& action) override;
    bool act(const std::vector<std::string>& action_string_args);
    std::vector<XiangqiAction> getLegalActions() const override;
    bool isLegalAction(const XiangqiAction& action) const override;
    bool isTerminal() const override;

    float getReward() const override { return 0.0f; }
    float getEvalScore(bool is_resign = false) const override;
    std::vector<float> getFeatures(utils::Rotation rotation = utils::Rotation::kRotationNone) const override;
    std::vector<float> getActionFeatures(const XiangqiAction& action, utils::Rotation rotation = utils::Rotation::kRotationNone) const override { return std::vector<float>(); };
    inline int getNumInputChannels() const override { return kFeaturePlanesSize; }
    inline int getPolicySize() const override { return kValidActions; }
    std::string toString() const override;
    inline std::string name() const override { return kXiangqiName + "_" + std::to_string(getBoardWidth()) + "x" + std::to_string(getBoardHeight()); }
    inline int getNumPlayer() const override { return kXiangqiNumPlayer; }
    inline int getRotatePosition(int position, utils::Rotation rotation) const override { return position; }
    inline int getRotateAction(int action_id, utils::Rotation rotation) const override { return action_id; }

    inline int getBoardHeight() const { return kXiangqiBoardHeight; }
    inline int getBoardWidth() const { return kXiangqiBoardWidth; }

    inline int getInputChannelHeight() const override { return getBoardHeight(); }
    inline int getInputChannelWidth() const override { return getBoardWidth(); }
    inline int getHiddenChannelHeight() const override { return getBoardHeight(); }
    inline int getHiddenChannelWidth() const override { return getBoardWidth(); }

private:
    Position pos_;

    // State info used in destruction
    StateInfo* rootBeforeCoppied = nullptr;
};

class XiangqiEnvLoader : public BaseBoardEnvLoader<XiangqiAction, XiangqiEnv> {
public:
    std::vector<float> getActionFeatures(const int pos, utils::Rotation rotation = utils::Rotation::kRotationNone) const override { return std::vector<float>(); };
    inline bool isPassAction(const XiangqiAction& action) const { return false; }
    inline std::vector<float> getValue(const int pos) const { return {getReturn()}; }
    inline std::string name() const override { return kXiangqiName + "_" + std::to_string(kXiangqiBoardWidth) + "x" + std::to_string(kXiangqiBoardHeight); }
    inline int getPolicySize() const override { return kValidActions; }
    inline int getRotatePosition(int position, utils::Rotation rotation) const override { return position; };
    inline int getRotateAction(int action_id, utils::Rotation rotation) const override { return action_id; };
};

} // namespace minizero::env::xiangqi
