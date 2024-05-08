#include "console_gtp.h"
#include "configuration.h"
#include "create_actor.h"
#include "create_network.h"
#include "sgf_loader.h"
#include "time_system.h"
#include <algorithm>
#include <climits>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace minizero::console {

using namespace network;

ConsoleGtp::ConsoleGtp()
{
    RegisterFunction("actor", this, &ConsoleGtp::cmdActor);
    RegisterFunction("gogui-analyze_commands", this, &ConsoleGtp::cmdGoguiAnalyzeCommands);
    RegisterFunction("list_commands", this, &ConsoleGtp::cmdListCommands);
    RegisterFunction("name", this, &ConsoleGtp::cmdName);
    RegisterFunction("version", this, &ConsoleGtp::cmdVersion);
    RegisterFunction("protocol_version", this, &ConsoleGtp::cmdProtocalVersion);
    RegisterFunction("clear_board", this, &ConsoleGtp::cmdClearBoard);
    RegisterFunction("showboard", this, &ConsoleGtp::cmdShowBoard);
    RegisterFunction("play", this, &ConsoleGtp::cmdPlay);
    RegisterFunction("boardsize", this, &ConsoleGtp::cmdBoardSize);
    RegisterFunction("genmove", this, &ConsoleGtp::cmdGenmove);
    RegisterFunction("reg_genmove", this, &ConsoleGtp::cmdGenmove);
    RegisterFunction("final_score", this, &ConsoleGtp::cmdFinalScore);
    RegisterFunction("load_model", this, &ConsoleGtp::cmdLoadModel);
}

void ConsoleGtp::initialize()
{
}

void ConsoleGtp::executeCommand(std::string command)
{
    if (command.back() == '\r') { command.pop_back(); }
    if (command.empty()) { return; }

    // parse command to args
    std::stringstream ss(command);
    std::string tmp;
    std::vector<std::string> args;
    while (std::getline(ss, tmp, ' ')) { args.push_back(tmp); }

    // execute function
    if (function_map_.count(args[0]) == 0) { return reply(ConsoleResponse::kFail, "Unknown command: " + command); }
    (*function_map_[args[0]])(args);
}

void ConsoleGtp::cmdActor(const std::vector<std::string>& args)
{
    if (!checkArgument(args, 2, 2)) { return; }
    config::zero_num_parallel_games = stoi(args[1]);
    ag.initialize();
    reply(ConsoleResponse::kSuccess, "");
}

void ConsoleGtp::cmdGoguiAnalyzeCommands(const std::vector<std::string>& args)
{
    if (!checkArgument(args, 1, 1)) { return; }
    std::string registered_cmd = "";
    reply(console::ConsoleResponse::kSuccess, registered_cmd);
}

void ConsoleGtp::cmdListCommands(const std::vector<std::string>& args)
{
    if (!checkArgument(args, 1, 1)) { return; }
    std::ostringstream oss;
    for (const auto& command : function_map_) { oss << command.first << std::endl; }
    reply(ConsoleResponse::kSuccess, oss.str());
}

void ConsoleGtp::cmdName(const std::vector<std::string>& args)
{
    if (!checkArgument(args, 1, 1)) { return; }
    reply(ConsoleResponse::kSuccess, "minizero");
}

void ConsoleGtp::cmdVersion(const std::vector<std::string>& args)
{
    if (!checkArgument(args, 1, 1)) { return; }
    reply(ConsoleResponse::kSuccess, "1.0");
}

void ConsoleGtp::cmdProtocalVersion(const std::vector<std::string>& args)
{
    if (!checkArgument(args, 1, 1)) { return; }
    reply(ConsoleResponse::kSuccess, "2");
}

void ConsoleGtp::cmdClearBoard(const std::vector<std::string>& args)
{
    if (!checkArgument(args, 1, 1)) { return; }
    for (size_t actor_id = 0; actor_id < ag.getSharedData()->actors_.size(); ++actor_id) {
        std::shared_ptr<actor::BaseActor>& actor = ag.getSharedData()->actors_[actor_id];
        ag.getSharedData()->resetActor(actor_id);
        actor->continue_search_ = true;
        actor->is_end_game_ = false;
    }
    reply(ConsoleResponse::kSuccess, "");
}

void ConsoleGtp::cmdShowBoard(const std::vector<std::string>& args)
{
    if (!checkArgument(args, 1, 1)) { return; }
    std::string board_string = "";
    for (size_t actor_id = 0; actor_id < ag.getSharedData()->actors_.size(); ++actor_id) {
        std::shared_ptr<actor::BaseActor>& actor = ag.getSharedData()->actors_[actor_id];
        board_string += "\n" + actor->getEnvironment().toString();
    }
    reply(ConsoleResponse::kSuccess, board_string);
}

void ConsoleGtp::cmdPlay(const std::vector<std::string>& args)
{
    size_t num_actor = ag.getSharedData()->actors_.size();
    if (!checkArgument(args, 2 * num_actor + 1, 2 * num_actor + 1)) { return; }
    std::string invalid_act_strings = "";
    bool invalid = false;
    for (size_t actor_id = 0; actor_id < num_actor; ++actor_id) {
        std::shared_ptr<actor::BaseActor>& actor = ag.getSharedData()->actors_[actor_id];
        std::vector<std::string> act_args;
        act_args.push_back(args[actor_id * 2 + 1]);
        act_args.push_back(args[actor_id * 2 + 2]);
        if (args[actor_id * 2 + 2] == "PASS" || (args[actor_id * 2 + 2] != "-" && !actor->act(act_args))) {
            if (actor->isEnvTerminal() && args[actor_id * 2 + 2] == "PASS") {
                invalid_act_strings += "- ";
            } else {
                invalid = true;
                invalid_act_strings += args[actor_id * 2 + 2] + " ";
            }
        } else {
            invalid_act_strings += "- ";
        }
    }
    if (invalid) { return reply(ConsoleResponse::kFail, "Invalid action: \"" + invalid_act_strings + "\""); }
    reply(ConsoleResponse::kSuccess, "");
}

void ConsoleGtp::cmdBoardSize(const std::vector<std::string>& args)
{
    if (!checkArgument(args, 2, 2)) { return; }
    minizero::config::env_board_size = stoi(args[1]);
    // ag.initialize();
    std::string board_string = "";
    for (size_t actor_id = 0; actor_id < ag.getSharedData()->actors_.size(); ++actor_id) {
        std::shared_ptr<actor::BaseActor>& actor = ag.getSharedData()->actors_[actor_id];
        board_string += "\n" + actor->getEnvironment().toString();
    }
    reply(ConsoleResponse::kSuccess, board_string);
}

void ConsoleGtp::cmdGenmove(const std::vector<std::string>& args)
{
    size_t num_actor = ag.getSharedData()->actors_.size();
    if (!checkArgument(args, num_actor + 1, num_actor + 1)) { return; }
    std::vector<std::string> actions_v;
    for (size_t actor_id = 0; actor_id < num_actor; ++actor_id) {
        std::shared_ptr<actor::BaseActor>& actor = ag.getSharedData()->actors_[actor_id];
        if (args[actor_id + 1] == "-") {
            actor->continue_search_ = false;
            actions_v.push_back("- ");
        } else if (actor->is_end_game_) {
            actions_v.push_back("- ");
            actor->continue_search_ = false;
        } else if (actor->isEnvTerminal()) {
            actions_v.push_back("PASS ");
            actor->is_end_game_ = true;
            actor->continue_search_ = false;
        } else {
            actor->getEnvironment().setTurn(minizero::env::charToPlayer(args[actor_id + 1].c_str()[0]));
            actor->resetSearch();
            actions_v.push_back("");
        }
    }
    ag.run();
    std::string actions_str = "";
    for (size_t actor_id = 0; actor_id < num_actor; ++actor_id) {
        std::shared_ptr<actor::BaseActor>& actor = ag.getSharedData()->actors_[actor_id];
        // if (args[actor_id + 1] == "-" || actor->is_end_game_) {
        //     actions_str += "- ";
        //     continue;
        // }
        // if (actor->isEnvTerminal()) {
        //     actions_str += "PASS ";
        //     actor->is_end_game_ = true;
        //     continue;
        // }
        // if (actor->isResign()) {
        //     actions_str += "Resign ";
        //     actor->is_end_game_ = true;
        //     continue;
        // }
        if (actions_v[actor_id] != "") {
            actions_str += actions_v[actor_id];
            continue;
        } else if (actor->isResign()) {
            actions_str += "Resign ";
            actor->is_end_game_ = true;
            actor->continue_search_ = false;
            continue;
        }
        actor->continue_search_ = true;
        actor->resetSearch();
        const Action action = actor->getEnvironment().getActionHistory().back();
        actions_str += action.toConsoleString() + " ";
    }
    reply(ConsoleResponse::kSuccess, actions_str);
}

void ConsoleGtp::cmdFinalScore(const std::vector<std::string>& args)
{
    std::string score_str = "";
    for (size_t actor_id = 0; actor_id < ag.getSharedData()->actors_.size(); ++actor_id) {
        std::shared_ptr<actor::BaseActor>& actor = ag.getSharedData()->actors_[actor_id];
        score_str += std::to_string(actor->getEvalScore()) + " ";
    }
    reply(ConsoleResponse::kSuccess, score_str);
}

void ConsoleGtp::cmdLoadModel(const std::vector<std::string>& args)
{
    if (!checkArgument(args, 2, 2)) { return; }
    minizero::config::nn_file_name = args[1];
    // ag.initialize();
    reply(ConsoleResponse::kSuccess, "");
}

bool ConsoleGtp::checkArgument(const std::vector<std::string>& args, int min_argc, int max_argc)
{
    int size = args.size();
    if (size >= min_argc && size <= max_argc) { return true; }

    std::ostringstream oss;
    oss << "command requires ";
    if (min_argc == max_argc) {
        oss << "exactly " << min_argc << " argument" << (min_argc == 1 ? "" : "s");
    } else {
        oss << min_argc << " to " << max_argc << " arguments";
    }

    reply(ConsoleResponse::kFail, oss.str());
    return false;
}

void ConsoleGtp::reply(ConsoleResponse response, const std::string& reply)
{
    std::cout << static_cast<char>(response) << " " << reply << "\n\n";
}

} // namespace minizero::console
