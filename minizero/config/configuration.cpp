#include "configuration.h"

namespace minizero::config {

int seed = 0;
bool auto_seed = true;
int actor_num_threads = 4;
int actor_num_parallel_games = 32;
int actor_num_simulation = 50;
float actor_mcts_puct_base = 19652;
float actor_mcts_puct_init = 1.25;
bool actor_select_action_by_count = false;
bool actor_select_action_by_softmax_count = true;
float actor_select_action_softmax_temperature = 1.0f;
bool actor_use_dirichlet_noise = true;
float actor_dirichlet_noise_alpha = 0.03f;
float actor_dirichlet_noise_epsilon = 0.25f;

int zero_server_port = 9999;
std::string zero_training_directory;
int zero_num_games_per_iteration = 5000;
int zero_start_iteration = 0;
int zero_end_iteration = 100;
int zero_replay_buffer = 20;

int learner_training_step = 500;
int learner_training_display_step = 100;
int learner_batch_size = 1024;
float learner_learning_rate = 0.02;
float learner_momentum = 0.9;
float learner_weight_decay = 0.0001;
int learner_num_process = 2;

std::string nn_file_name = "model_files/tietactoe_alphazero_example.pt";
int nn_num_input_channels = 4;
int nn_input_channel_height = 3;
int nn_input_channel_width = 3;
int nn_num_hidden_channels = 16;
int nn_hidden_channel_height = 3;
int nn_hidden_channel_width = 3;
int nn_num_blocks = 1;
int nn_num_action_channels = 1;
int nn_action_size = 9;
std::string nn_type_name = "alphazero";

void SetConfiguration(ConfigureLoader& cl)
{
    cl.AddParameter("seed", seed, "", "Actor");
    cl.AddParameter("auto_seed", auto_seed, "", "Actor");

    // actor parameters
    cl.AddParameter("actor_num_threads", actor_num_threads, "", "Actor");
    cl.AddParameter("actor_num_parallel_games", actor_num_parallel_games, "", "Actor");
    cl.AddParameter("actor_num_simulation", actor_num_simulation, "", "Actor");
    cl.AddParameter("actor_mcts_puct_base", actor_mcts_puct_base, "", "Actor");
    cl.AddParameter("actor_mcts_puct_init", actor_mcts_puct_init, "", "Actor");
    cl.AddParameter("actor_select_action_by_count", actor_select_action_by_count, "", "Actor");
    cl.AddParameter("actor_select_action_by_softmax_count", actor_select_action_by_softmax_count, "", "Actor");
    cl.AddParameter("actor_select_action_softmax_temperature", actor_select_action_softmax_temperature, "", "Actor");
    cl.AddParameter("actor_use_dirichlet_noise", actor_use_dirichlet_noise, "", "Actor");
    cl.AddParameter("actor_dirichlet_noise_alpha", actor_dirichlet_noise_alpha, "", "Actor");
    cl.AddParameter("actor_dirichlet_noise_epsilon", actor_dirichlet_noise_epsilon, "", "Actor");

    // zero parameters
    cl.AddParameter("zero_server_port", zero_server_port, "", "Zero");
    cl.AddParameter("zero_training_directory", zero_training_directory, "", "Zero");
    cl.AddParameter("zero_num_games_per_iteration", zero_num_games_per_iteration, "", "Zero");
    cl.AddParameter("zero_start_iteration", zero_start_iteration, "", "Zero");
    cl.AddParameter("zero_end_iteration", zero_end_iteration, "", "Zero");
    cl.AddParameter("zero_replay_buffer", zero_replay_buffer, "", "Zero");

    // learner parameters
    cl.AddParameter("learner_training_step", learner_training_step, "", "Learner");
    cl.AddParameter("learner_training_display_step", learner_training_display_step, "", "Learner");
    cl.AddParameter("learner_batch_size", learner_batch_size, "", "Learner");
    cl.AddParameter("learner_learning_rate", learner_learning_rate, "", "Learner");
    cl.AddParameter("learner_momentum", learner_momentum, "", "Learner");
    cl.AddParameter("learner_weight_decay", learner_weight_decay, "", "Learner");
    cl.AddParameter("learner_num_process", learner_num_process, "", "Learner");

    // network parameters
    cl.AddParameter("nn_file_name", nn_file_name, "", "Network");
    cl.AddParameter("nn_num_input_channels", nn_num_input_channels, "", "Network");
    cl.AddParameter("nn_input_channel_height", nn_input_channel_height, "", "Network");
    cl.AddParameter("nn_input_channel_width", nn_input_channel_width, "", "Network");
    cl.AddParameter("nn_num_hidden_channels", nn_num_hidden_channels, "", "Network");
    cl.AddParameter("nn_hidden_channel_height", nn_hidden_channel_height, "", "Network");
    cl.AddParameter("nn_hidden_channel_width", nn_hidden_channel_width, "", "Network");
    cl.AddParameter("nn_num_blocks", nn_num_blocks, "", "Network");
    cl.AddParameter("nn_num_action_channels", nn_num_action_channels, "", "Network");
    cl.AddParameter("nn_action_size", nn_action_size, "", "Network");
    cl.AddParameter("nn_type_name", nn_type_name, "", "Network");
}

} // namespace minizero::config