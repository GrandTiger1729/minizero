#pragma once

#include "actor_group_gtp.h"
#include "base_actor.h"
#include "console.h"
#include "network.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace minizero::console {

using namespace minizero;

class ConsoleGtp {
public:
    ConsoleGtp();
    virtual ~ConsoleGtp() = default;

    virtual void initialize();
    virtual void executeCommand(std::string command);

protected:
    class BaseFunction {
    public:
        virtual ~BaseFunction() = default;
        virtual void operator()(const std::vector<std::string>& args) = 0;
    };

    template <class I, class F>
    class Function : public BaseFunction {
    public:
        Function(I* instance, F function) : instance_(instance), function_(function) {}
        void operator()(const std::vector<std::string>& args) { (*instance_.*function_)(args); }

        I* instance_;
        F function_;
    };

    template <class I, class F>
    void RegisterFunction(const std::string& name, I* instance, F function)
    {
        function_map_[name] = std::make_shared<Function<I, F>>(instance, function);
    }

    void cmdActor(const std::vector<std::string>& args);
    void cmdGoguiAnalyzeCommands(const std::vector<std::string>& args);
    void cmdListCommands(const std::vector<std::string>& args);
    void cmdName(const std::vector<std::string>& args);
    void cmdVersion(const std::vector<std::string>& args);
    void cmdProtocalVersion(const std::vector<std::string>& args);
    void cmdClearBoard(const std::vector<std::string>& args);
    void cmdShowBoard(const std::vector<std::string>& args);
    void cmdPlay(const std::vector<std::string>& args);
    void cmdBoardSize(const std::vector<std::string>& args);
    void cmdGenmove(const std::vector<std::string>& args);
    void cmdFinalScore(const std::vector<std::string>& args);
    void cmdLoadModel(const std::vector<std::string>& args);
    bool checkArgument(const std::vector<std::string>& args, int min_argc, int max_argc);
    void reply(ConsoleResponse response, const std::string& reply);

    actor::ActorGroupGtp ag;
    std::map<std::string, std::shared_ptr<BaseFunction>> function_map_;
};

} // namespace minizero::console
