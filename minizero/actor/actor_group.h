#pragma once

#include "actor.h"
#include "network.h"
#include <boost/thread.hpp>
#include <mutex>
#include <vector>

namespace minizero::actor {

class ThreadSharedData {
public:
    int GetNextActorIndex();
    void OutputRecord(const std::string& record);

    bool do_cpu_job_;
    int actor_index_;
    std::mutex mutex_;
    std::vector<std::shared_ptr<Actor>> actors_;
    std::vector<std::shared_ptr<network::Network>> networks_;
    std::vector<std::vector<std::shared_ptr<network::NetworkOutput>>> network_outputs_;
};

class SlaveThread {
public:
    SlaveThread(int id, ThreadSharedData& shared_data)
        : id_(id),
          shared_data_(shared_data),
          start_barrier_(2),
          finish_barrier_(2) {}

    void RunThread();

    inline void Start() { start_barrier_.wait(); }
    inline void Finish() { finish_barrier_.wait(); }

private:
    void DoCPUJob();
    void HandleSearchEndAndEnvEnd(const std::shared_ptr<Actor>& actor);
    void DoGPUJob();

    int id_;
    ThreadSharedData& shared_data_;
    boost::barrier start_barrier_;
    boost::barrier finish_barrier_;
};

class ActorGroup {
public:
    ActorGroup();

    void Run();

private:
    ThreadSharedData shared_data_;
    boost::thread_group thread_groups_;
    std::vector<std::shared_ptr<SlaveThread>> slave_threads_;
};

} // namespace minizero::actor