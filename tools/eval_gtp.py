#!/usr/bin/env python

import numpy as np
import pandas as pd
import os
import argparse
import matplotlib.pyplot as plt


def getPlayerName(lines, black=True):
    if black:
        command = 'Black'
    else:
        command = 'White'
    try:
        return lines.split(command)[1].split('\n')[0].split('weight_iter_')[1][:-3]
    except BaseException:
        return '0'


def getWinRate(file, game0_white):
    with open(file, 'r') as f:
        lines = f.read()
        if game0_white:
            player2 = getPlayerName(lines, True)
            player1 = getPlayerName(lines, False)
        else:
            player1 = getPlayerName(lines, True)
            player2 = getPlayerName(lines, False)

        total = 0
        p1_black_win = 0
        p1_white_win = 0
        draw = 0
        dup_game = 0

        part1 = lines.split('Black')[1].split('\n')[2:-1]
        total += len(part1)

        for line_id, line in enumerate(part1):
            game_info = line.split('\t')
            if game_info[1] == "1.000000":
                p1_black_win += 1
            elif game_info[1] == "0.000000":
                draw += 1
        
        part2 = lines.split('Black')[2].split('\n')[2:-1]
        total += len(part2)

        for line_id, line in enumerate(part2):
            game_info = line.split('\t')

            if game_info[1] == "-1.000000":
                p1_white_win += 1
            elif game_info[1] == "0.000000":
                draw += 1

        p1_win_rate = (p1_black_win + p1_white_win + draw / 2) / (total - dup_game)
        return player1, player2, round(p1_win_rate, 4), p1_black_win, p1_white_win, draw, total, dup_game


def compute_elo(cur_elo, win):
    if win == 1:
        cur_elo += 1000
    elif win == 0:
        cur_elo -= 1000
    else:
        cur_elo -= max(min(1000, 400 * np.log10((1 - win) / win)), -1000)
    return round(cur_elo, 3)


def plot_elo_curve(fig_name, *args):
    _, ax = plt.subplots()
    ax.set_xlabel('nn steps')
    ax.set_ylabel('elo rating')
    for i, (its, elos) in enumerate(args):
        player_name = input(f'player{i+1} name: ')
        ax.plot([0] + its, [0] + elos, label=player_name)
    ax.legend()
    plt.savefig(fig_name)

def plot_win_rate_curve(fig_name, *args):

    _, ax = plt.subplots()
    ax.set_xlabel('nn steps')
    ax.set_ylabel('win rate')
    ax.spines['left'].set_position(('data', 0))
    for i, (its, win_rates) in enumerate(args):
        player_name = input(f'player{i+1} name: ')
        ax.plot([0] + its, [0] + win_rates, label=player_name)
    ax.legend()
    plt.savefig(fig_name)


def eval(dir, fout_name, player1_elo_file, game0_white, step_num, plot_elo, plot_win_rate):
    win_rates = []
    black_wins = []
    white_wins = []
    draws = []
    totals = []
    player1s = []
    player2s = []
    dup_games = []
    for subdir in os.listdir(dir):
        datfile = os.path.join(dir, subdir, f'{subdir}.txt')
        if os.path.isfile(datfile):
            player1, player2, win, black_win, white_win, draw, total, dup_game = getWinRate(
                datfile, game0_white)
            win_rates.append(win)
            draws.append(draw)
            black_wins.append(black_win)
            white_wins.append(white_win)
            totals.append(total)
            dup_games.append(dup_game)
            player1s.append(int(player1) // step_num)
            player2s.append(int(player2) // step_num)
    result = pd.DataFrame({'P1': player1s,
                           'P2': player2s,
                           'Black': black_wins,
                           'White': white_wins,
                           'Draw': draws,
                           'DUP': dup_games,
                           'Total': totals,
                           'WinRate': win_rates}, index=player1s).sort_values(['P1', 'P2'])
    if player1_elo_file:
        elo_df = pd.read_csv(player1_elo_file)
        black_elos = []
        white_elos = []
        for model, win in zip(result['P1'].iloc, result['WinRate'].iloc):
            try:
                if not game0_white:
                    black_elos.append(
                        elo_df[elo_df['P1'] == model]['P1 Elo'].to_list()[0])
                    white_elos.append(compute_elo(black_elos[-1], 1 - win))
                else:
                    white_elos.append(
                        elo_df[elo_df['P1'] == model]['P1 Elo'].to_list()[0])
                    black_elos.append(compute_elo(white_elos[-1], 1 - win))
            except BaseException:
                # black_elos.append(np.nan)
                # white_elos.append(np.nan)
                # print(f'No data for {model} in {player1_elo_file} !')
                if not game0_white:
                    black_elos.append(0)
                    white_elos.append(compute_elo(black_elos[-1], 1 - win))
                else:
                    white_elos.append(0)
                    black_elos.append(compute_elo(white_elos[-1], 1 - win))
        result.insert(8, 'P1 Elo', black_elos)
        result.insert(9, 'P2 Elo', white_elos)
        if plot_elo:
            plot_elo_curve(os.path.join(dir, f'{fout_name}.png'),
                           [result['P1'].to_list(), result['P1 Elo'].to_list()],
                           [result['P2'].to_list(), result['P2 Elo'].to_list()])
        if plot_win_rate:
            plot_win_rate_curve(os.path.join(dir, f'{fout_name}.png'),
                           [result['P1'].to_list(), result['WinRate'].to_list()])
    else:
        cur_elo = 0
        elos = []
        for win in result['WinRate'].iloc:
            cur_elo = compute_elo(cur_elo, win)
            elos.append(cur_elo)
        result.insert(8, 'P1 Elo', elos)
        if plot_elo:
            plot_elo_curve(os.path.join(dir, f'{fout_name}.png'),
                           [result['P1'].to_list(), result['P1 Elo'].to_list()])
        if plot_win_rate:
            plot_win_rate_curve(os.path.join(dir, f'{fout_name}.png'),
                           [result['P1'].to_list(), result['WinRate'].to_list()])
    result.to_csv(os.path.join(dir, f'{fout_name}.csv'), index=False)
    print(dir)
    print(result.to_string(index=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-in_dir', dest='dir', type=str,
                        help='dir to eval')
    parser.add_argument('-out_file', dest='fout_name', type=str, default='elo',
                        help='output flie')
    parser.add_argument('-elo', dest='player1_elo_file', type=str,
                        help='elo flie of player 1')
    parser.add_argument('--white', action='store_true', dest='game0_white',
                        help='print player2\'s stats')
    parser.add_argument('--step', dest='step_num', type=int, default=1,
                        help='training step')
    parser.add_argument('--plot', action='store_true', dest='plot_elo',
                        help='plot elo curve')
    parser.add_argument('--pw', action='store_true', dest='plot_win_rate',
                        help='plot win_rate curve')
    args = parser.parse_args()
    if args.dir:
        if os.path.isdir(args.dir):
            eval(args.dir, args.fout_name, args.player1_elo_file,
                 args.game0_white, args.step_num, args.plot_elo, args.plot_win_rate)
        else:
            print(f'\"{args.dir}\" does not exist!')
            exit(1)
    else:
        parser.print_help()
        exit(1)

