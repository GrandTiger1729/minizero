import sys
import torch
from .alphazero_network import AlphaZeroNetwork
from .muzero_network import MuZeroNetwork


def create_network(game_name="tietactoe",
                   num_input_channels=4,
                   input_channel_height=3,
                   input_channel_width=3,
                   num_hidden_channels=16,
                   hidden_channel_height=3,
                   hidden_channel_width=3,
                   num_blocks=1,
                   num_action_channels=1,
                   action_size=9,
                   network_type_name="alphazero"):

    network = None
    if network_type_name == "alphazero":
        network = AlphaZeroNetwork(game_name,
                                   num_input_channels,
                                   input_channel_height,
                                   input_channel_width,
                                   num_hidden_channels,
                                   hidden_channel_height,
                                   hidden_channel_width,
                                   num_blocks,
                                   num_action_channels,
                                   action_size)
    elif network_type_name == "muzero":
        network = MuZeroNetwork(game_name,
                                num_input_channels,
                                input_channel_height,
                                input_channel_width,
                                num_hidden_channels,
                                hidden_channel_height,
                                hidden_channel_width,
                                num_blocks,
                                num_action_channels,
                                action_size)

    return network