#include "actor_group_gtp.h"
#include "configuration.h"
#include "create_actor.h"
#include "create_network.h"
#include "random.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <torch/cuda.h>
#include <utility>

namespace minizero::actor {

using namespace network;
using namespace utils;

bool SlaveThreadGtp::doCPUJob()
{
    size_t actor_id = getSharedData()->getAvailableActorIndex();
    if (actor_id >= getSharedData()->actors_.size()) { return false; }
    if (config::zero_actor_stop_after_enough_games && getSharedData()->actors_game_index_[actor_id] >= config::zero_num_games_per_iteration) { return true; }

    std::shared_ptr<BaseActor>& actor = getSharedData()->actors_[actor_id];
    if (!actor->continue_search_) { return true; }
    int network_id = actor_id % getSharedData()->networks_.size();
    int network_output_id = actor->getNNEvaluationBatchIndex();
    if (network_output_id >= 0) {
        assert(network_output_id < static_cast<int>(getSharedData()->network_outputs_[network_id].size()));
        actor->afterNNEvaluation(getSharedData()->network_outputs_[network_id][network_output_id]);
        if (actor->isSearchDone()) {
            handleSearchDone(actor_id);
            actor->continue_search_ = false;
        }
    }
    if (actor->continue_search_) {
        getSharedData()->all_search_done_ = false;
        actor->beforeNNEvaluation();
    }
    return true;
}

void SlaveThreadGtp::handleSearchDone(int actor_id)
{
    assert(actor_id >= 0 && actor_id < static_cast<int>(getSharedData()->actors_.size()) && getSharedData()->actors_[actor_id]->isSearchDone());

    std::shared_ptr<BaseActor>& actor = getSharedData()->actors_[actor_id];
    if (!actor->isResign()) { actor->act(actor->getSearchAction()); }
}

void ActorGroupGtp::run()
{
    while (true) {
        getSharedData()->actor_index_ = 0;
        getSharedData()->all_search_done_ = getSharedData()->do_cpu_job_;
        for (auto& t : slave_threads_) { t->start(); }
        for (auto& t : slave_threads_) { t->finish(); }
        getSharedData()->do_cpu_job_ = !getSharedData()->do_cpu_job_;
        if (getSharedData()->all_search_done_) { break; }
    }
}

void ActorGroupGtp::initialize()
{
    int num_threads = std::max(static_cast<int>(torch::cuda::device_count()), config::zero_num_threads);
    createSlaveThreads(num_threads);
    createNeuralNetworks();
    createActors();
    getSharedData()->do_cpu_job_ = true;
    getSharedData()->all_search_done_ = false;
}

} // namespace minizero::actor
