#pragma once

#include "actor_group.h"
#include "base_actor.h"
#include "network.h"
#include "paralleler.h"
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace minizero::actor {

class SlaveThreadGtp : public SlaveThread {
public:
    SlaveThreadGtp(int id, std::shared_ptr<utils::BaseSharedData> shared_data)
        : SlaveThread(id, shared_data) {}

protected:
    bool doCPUJob() override;
    void handleSearchDone(int actor_id) override;
};

class ActorGroupGtp : public ActorGroup {
public:
    ActorGroupGtp() {}

    void run();
    void initialize() override;

protected:
    std::shared_ptr<utils::BaseSlaveThread> newSlaveThread(int id) override { return std::make_shared<SlaveThreadGtp>(id, shared_data_); }
};

} // namespace minizero::actor
