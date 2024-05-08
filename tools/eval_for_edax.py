import numpy as np
import pandas as pd
import os
import argparse
import matplotlib.pyplot as plt


def getPlayerName(lines, black=True):
    if black:
        command = 'BlackCommand'
        counter_command = 'WhiteCommand'
    else:
        command = 'WhiteCommand'
        counter_command = 'BlackCommand'
    # print(lines.split(counter_command)[1].split('\n')[0])
    # print(lines.split(counter_command)[1].split('\n')[0].split(" ")[3])
    # print(len(lines.split(counter_command)[1].split('\n')[0].split(" ")[3])==2)
    
    try:
        # print("test")
        # print(lines.split(command)[1].split('\n')[0].split('weight_iter_')[1].split('.')[0])
        # print(lines.split(command)[1].split('\n')[0].split('weight_iter_')[1].split('.')[0])
        return lines.split(command)[1].split('\n')[0].split('weight_iter_')[1].split('.')[0]
    except:
        try:
            if int(len(lines.split(command)[1].split('\n')[0].split(" ")[3])) == 2 or int(len(lines.split(command)[1].split('\n')[0].split(" ")[3])) == 1:
                # print("insei")
                return lines.split(command)[1].split('\n')[0].split(" ")[3]
            else:
                # print("oisuf")
                return '0'
        except:
            # print("fsdf")
            return '0'

        # print(lines.split(counter_command)[1])


def getWinRate(file, game0_white,skip):
    with open(file, 'r') as f:
        lines = f.read()
        if game0_white:
            player2 = getPlayerName(lines, True)
            player1 = getPlayerName(lines, False)
        else:
            player1 = getPlayerName(lines, True)
            player2 = getPlayerName(lines, False)
        lines = lines.split('ERR_MSG\n')[1].split('\n')[:-1]
        total = len(lines)
        p1_black_win = 0
        p1_white_win = 0
        draw = 0
        dup_game = 0
        counted_game=0
        if game0_white:
            white_games = 0
            resign_win='W+'
        else:
            white_games = 1
            resign_win='B+'
        for line in lines:
            game_info = line.split('\t')
            # 0 GAME, 1	RES_B, 2 RES_W, 3 RES_R, 4 ALT, 5 DUP
            if game_info[5] != '-':
                dup_game += 1
                if skip == True:
                    continue
            if int(game_info[4]) == white_games:
                if (game_info[1] == '-1.000000' or game_info[2] == '-1.000000' or resign_win in game_info[1] or resign_win in game_info[2]):
                    p1_white_win += 1
                elif game_info[1] == '0.000000'or game_info[2] == '0.000000' or game_info[1] == '0' or game_info[2] == '0':
                    draw += 1
            elif (game_info[1] == '1.000000' or game_info[2] == '1.000000' or resign_win in game_info[1] or resign_win in game_info[2]):
                p1_black_win += 1
            elif game_info[1] == '0.000000' or game_info[2] == '0.000000' or game_info[1] == '0' or game_info[2] == '0':
                draw += 1
            counted_game+=1
        # print("player1",player1)
        # print("player2",player2)
        # print("p1_black_win",p1_black_win)
        # print("p1_white_win",p1_white_win)
        # print("draw",draw)
        # print("counted_game",counted_game)
        p1_win_rate = (p1_black_win + p1_white_win + draw / 2) / total
        p1_win_rate_counted = (p1_black_win + p1_white_win + draw / 2) / counted_game
        # print("p1_win_rate",p1_win_rate)
        # print("p1_win_rate_counted",p1_win_rate_counted)
        # print("")
        return player1, player2, round(p1_win_rate_counted, 4), p1_black_win, p1_white_win, draw, counted_game, dup_game


def compute_elo(cur_elo, win):
    if win == 1:
        cur_elo += 1000
    elif win == 0:
        cur_elo -= 1000
    else:
        cur_elo -= max(min(1000, 400*np.log((1-win)/win)), -1000)
    return round(cur_elo, 3)


def plot_elo_curve(fig_name, *args):
    _, ax=plt.subplots()
    ax.set_xlabel('nn steps')
    ax.set_ylabel('elo rating')
    for i, (its, elos) in enumerate(args):
        player_name = input(f'player{i+1} name: ')
        ax.plot([0]+its, [0]+elos, label=player_name)
    ax.legend()
    plt.savefig(fig_name)

def plot_win_rate_curve(fig_name, *args):
    _, ax = plt.subplots()
    ax.set_xlabel('nn steps')
    ax.set_ylabel('win rate')
    # ax.spines['left'].set_position(('data', 600000))
    for i, (its, win_rates) in enumerate(args):
        player_name = input(f'player{i+1} name: ')
        # for i in range(len(win_rates)):
        #     win_rates[i] = 1 - win_rates[i]
        for j in range(len(its)):
            its[j] -= 600000
        ax.plot(its, win_rates, label=player_name)
    ax.legend()
    plt.savefig(fig_name)


def eval(dir, fout_name, player1_elo_file, game0_white, step_num, plot_elo, plot_win_rate, skip):
    win_rates = []
    black_wins = []
    white_wins = []
    draws = []
    totals = []
    player1s = []
    player2s = []
    dup_games = []
    for subdir in os.listdir(dir):
        datfile = os.path.join(dir, subdir, f'{subdir}.dat')
        if os.path.isfile(datfile):
            player1, player2, win, black_win, white_win, draw, total, dup_game = getWinRate(
                datfile, game0_white,skip)
            win_rates.append(win)
            draws.append(draw)
            black_wins.append(black_win)
            white_wins.append(white_win)
            totals.append(total)
            dup_games.append(dup_game)
            # print("player1",player1)
            # print("player2",player2)
            player1s.append(int(player1)//step_num)
            player2s.append(int(player2)//step_num)
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
                    white_elos.append(compute_elo(black_elos[-1], 1-win))
                else:
                    white_elos.append(
                        elo_df[elo_df['P1'] == model]['P1 Elo'].to_list()[0])
                    black_elos.append(compute_elo(white_elos[-1], 1-win))
            except:
                black_elos.append(np.nan)
                white_elos.append(np.nan)
                print(f'No data for {model} in {player1_elo_file} !')
        result.insert(8, 'P1 Elo', black_elos)
        result.insert(9, 'P2 Elo', white_elos)
        if plot_elo:
            plot_elo_curve(os.path.join(dir, f'{fout_name}.png'),
                           [result['P1'].to_list(), result['P1 Elo'].to_list()],
                           [result['P2'].to_list(), result['P2 Elo'].to_list()])
        if plot_win_rate:
            plot_win_rate_curve(os.path.join(dir, f'{fout_name}.png'),
                           [result['P2'].to_list(), result['WinRate'].to_list()])
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
                           [result['P2'].to_list(), result['WinRate'].to_list()])
    if skip == True:
        fout_name=fout_name+'_skip'
    result.to_csv(os.path.join(dir, f'{fout_name}.csv'), index=False)
    print(result.to_string(index=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--dir', dest='dir', type=str,
                        help='dir')
    parser.add_argument('-o', '--out', dest='fout_name', type=str, default='elo',
                        help='output flie')
    parser.add_argument('-e', '--elo', dest='player1_elo_file', type=str,
                        help='elo flie of player 1')
    parser.add_argument('-w', '--white', action='store_true', dest='game0_white',
                        help='output flie')
    parser.add_argument('--step', dest='step_num', type=int, default=1,
                        help='training step')
    parser.add_argument('--skip', dest='skip', type=bool, default=False,
                        help='skip DUP or not')
    parser.add_argument('--plot', action='store_true', dest='plot_elo',
                        help='plot elo curve')
    parser.add_argument('--pw', action='store_true', dest='plot_win_rate',
                        help='plot win_rate curve')
    args = parser.parse_args()
    if args.dir and os.path.isdir(args.dir):
        eval(args.dir, args.fout_name, args.player1_elo_file,
             args.game0_white, args.step_num, args.plot_elo, args.plot_win_rate, args.skip)
    else:
        print('No such directory!')