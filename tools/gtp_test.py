#! /usr/bin/env python3
import os
import argparse
import sys
N = 1024
pipe_1to2 = os.pipe()
pipe_2to1 = os.pipe()
pipe_1to3 = os.pipe()
pipe_3to1 = os.pipe()
# def split_sgf(dir_path):
#     for path in os.scandir(dir_path):
#         if path.is_file():
#             if "-" in str(path):
#                 print("path: ",str(path))
#                 f = open(path, "r")

#                 print(f.read())

def get_game_number(dir_path):
    game_number = 0 # played games number
    for path in os.scandir(dir_path):
        if path.is_file():
            # print("path: ",str(path))
            if "-" in str(path):
                game_number += 1
    return game_number
# twogtp
def child1(args):
    os.close(pipe_1to2[0])
    os.close(pipe_2to1[1])
    os.close(pipe_1to3[0])
    os.close(pipe_3to1[1])
    start = True
    counter=0
    games=[]
    path_one=[]
    done_list=[]
    trajectory= [[] for i in range(int(args.actor_num))]
    # duplicate_trajectory=[[] for i in range(int(args.games))]
    game_number = 0 # played games number
    # filename=args.sgffile+"-"+str(len(games))+".txt"


    dir_path=os.path.dirname(os.path.dirname(args.sgffile+'/'))
    game_number=get_game_number(dir_path)
    # dir_path = args.sgffile[:len(args.sgffile)-6]
    # print('dir_path:', dir_path)

    print('file game_number:', game_number)
    while True:
        if start:
            # buffer = os.read(0, N)
            # num actor
            # clearboard
            # actor 2
            # genmove b b
            # play b xx b PASS
            # genmove w w
            # play w xx w PASS
            # genmove b -
            # play b xx - -
            # genmove w -
            # play w xx - -

            print(" start !! game_number:",game_number, file=sys.stderr)
            # buffer = b"actor "+str(args.actor_num+"\n").encode()
            buffer = f"actor {args.actor_num}\n".encode()
            # f"play w ".encode()
            os.write(1, buffer)
            os.write(pipe_1to2[1], buffer)
            os.write(pipe_1to3[1], buffer)
            buffer = os.read(pipe_2to1[0], N)
            os.write(1, buffer)
            buffer = os.read(pipe_3to1[0], N)
            os.write(1, buffer)
            start=False

        if counter%2==0:
            # buffer = b"genmove b b\n"
            buffer = f"genmove"
            for i in range(int(args.actor_num)):
                if i in done_list:
                    buffer+=" -"
                else:
                    buffer+=" b"
            buffer+="\n"
            buffer = buffer.encode()
        else:
            # buffer = b"genmove w w\n"
            buffer = f"genmove"
            for i in range(int(args.actor_num)):
                if i in done_list:
                    buffer+=" -"
                else:
                    buffer+=" w"
                # buffer+=" w"
            buffer+="\n"
            buffer = buffer.encode()
        # print(buffer, file=sys.stderr)
        
        if buffer and start:
            start = False
        # print(f"{buffer[:len(buffer)-1]}")

        # print(f"1 buffer: {buffer}")
        if counter%2==0:
            os.write(pipe_1to2[1], buffer)
        else:
            os.write(pipe_1to3[1], buffer)

        # os.write(pipe_1to2[1], buffer)
        # print(f"2 buffer: {buffer}")
        # os.write(pipe_1to3[1], b"play " + buffer)
        if counter%2==0:
            buffer = os.read(pipe_2to1[0], N)
        else:
            buffer = os.read(pipe_3to1[0], N)
        # buffer = os.read(pipe_2to1[0], N)
        # print(f"{buffer[:len(buffer)-3]}")
        if not buffer:
            break
        # print(f"3 buffer: {buffer}")
        os.write(1, buffer)
        # print(f"4 buffer: {buffer}")
        tmp=[]
        for item in str(buffer).split(' '):
            # print("buffer.split(' '): "+item, file=sys.stderr)
            item.replace("\n", "")
            item.replace("\'", "")
            # print("item",item, file=sys.stderr)
            # if item[1]=="PASS":
            #     item[1]="PASS"
            tmp.append(item)
            # print("tmp",tmp, file=sys.stderr)
        # print(tmp, file=sys.stderr)
        if counter%2==0:
            # buffer=b"play b " + str(tmp[1]).encode()+b" b "+str(tmp[2]+"\n").encode()
            buffer=f"play"
            for i in range(int(args.actor_num)):
                if i in done_list:
                    buffer+=" - -"
                else:
                    buffer+=" b"
                    buffer+=f" {str(tmp[i+1])}"
                trajectory[i].append(["b",tmp[i+1]])
            buffer+="\n"
            buffer = buffer.encode()

            # buffer = b"game: "+str(len(games)+game_number).encode()
            # path_one.append(["b",tmp[1]])
            # store b buffer[2:]
        else:
            # buffer=b"play w " + str(tmp[1]).encode()+b" w "+str(tmp[2]+"\n").encode()
            buffer=f"play"
            for i in range(int(args.actor_num)):
                if i in done_list:
                    buffer+=" - -"
                else:
                    buffer+=" w"
                    buffer+=f" {str(tmp[i+1])}"
                # buffer+=" w"
                # buffer+=f" {str(tmp[i+1])}"
                trajectory[i].append(["w",tmp[i+1]])
            buffer+="\n"
            buffer = buffer.encode()
            path_one.append(["w",tmp[1]])
        # print(buffer, file=sys.stderr)
        
            # store w buffer[2:]
        # print(trajectory)
        if counter%2==0:
            os.write(pipe_1to3[1], buffer)
        else:
            os.write(pipe_1to2[1], buffer)

        # os.write(pipe_1to3[1], buffer)
        # print(buffer)
        # print(path_one)
        # for i in range(int(args.actor_num)):
        #     print(f"{i}'s trajectory\n",trajectory[i])
        if counter%2==0:
            # print("send buffer",buffer, file=sys.stderr)
            buffer = os.read(pipe_3to1[0], N)
        else:
            # print("send buffer",buffer, file=sys.stderr)
            buffer = os.read(pipe_2to1[0], N)
        # print("read buffer",buffer, file=sys.stderr)
        # buffer = os.read(pipe_3to1[0], N)
        # print(f"{buffer[:len(buffer)-3]}")
        if not buffer:
            break
        # print(f"6 buffer: {buffer}")
        os.write(1, buffer)
        # print(f"7 buffer: {buffer}")
        counter=counter+1
        for i in range(int(args.actor_num)):
            
            if ((trajectory[i][len(trajectory[i])-1][0]=="b" and trajectory[i][len(trajectory[i])-1][1]=="PASS" and
            trajectory[i][len(trajectory[i])-2][0]=="w" and trajectory[i][len(trajectory[i])-2][1]=="PASS") or 
            (trajectory[i][len(trajectory[i])-1][0]=="w" and trajectory[i][len(trajectory[i])-1][1]=="PASS" and
            trajectory[i][len(trajectory[i])-2][0]=="b" and trajectory[i][len(trajectory[i])-2][1]=="PASS")  ):
                print(f"game {i} is done!!!!")
                done_list.append(i)

        all_done_flag=0
        for i in range(int(args.actor_num)):
            if i in done_list:
                all_done_flag+=1
        if all_done_flag==int(args.actor_num):
            print("alldone!!!")
            buffer = f"final_score\n".encode()
            os.write(pipe_1to2[1], buffer)
            os.write(pipe_1to3[1], buffer)
            buffer = os.read(pipe_3to1[0], N)
            final_score=str(buffer)
            final_score_tmp=[]
            for item in final_score.split(' '):
                # print("item",item, file=sys.stderr)
                final_score_tmp.append(item)
                # print("tmp",final_score_tmp, file=sys.stderr)
        # for item in str(buffer).split(' '):
        #     # print("buffer.split(' '): "+item, file=sys.stderr)
        #     item.replace("\n", "")
        #     item.replace("\'", "")
        #     print("item",item, file=sys.stderr)
        #     # if item[1]=="PASS":
        #     #     item[1]="PASS"
        #     tmp.append(item)
        #     print("tmp",tmp, file=sys.stderr)

            os.write(1, buffer)
            buffer = os.read(pipe_2to1[0], N)
            os.write(1, buffer)
            game_number=get_game_number(dir_path)+1
            # split_sgf(dir_path)
            for i in range(int(args.actor_num)):
                buffer = f"game: {game_number+i}".encode()
                # buffer = b"game: "+str(len(games)+game_number).encode()
                os.write(1, buffer)

                # try to write int(args.actor_num) txt file at the same time
                mystring=";"
                mystring="game_number:"+str(game_number+i)+"\n"
                mystring=mystring+final_score_tmp[i+1]+"\n"

                print("game_number:",game_number+i, file=sys.stderr)
                for item in trajectory[i]:
                    if item[1]=='-':
                        break
                    # print(item)
                        # print(item[0])
                        # print(item[1])
                    mystring=mystring+item[0]+"["+item[1]+"];"
                    # print(item[1][0])
                    # print(item[1][1])
                    # if item[1]=="PASS":
                    #     pos=int(args.size)*int(args.size)
                    # else:
                    #     pos=int(float(ord(item[1][0])-ord('A')))+(int(item[1][1])-1)*int(args.size)
                        #x value:int(float(ord(item[1][0])-ord('A')))
                        #y value:(int(item[1][1])-1)
                    # print(pos)
                    # duplicate_trajectory[game_number+i-1].append([item[0],pos])
                    #store in duplicate_trajectory start from 0
                    #duplicate store the exact pos value
                    # duplicate_trajectory[game_number+i-1].append([item[0],item[1]])
                                    # trajectory[i].append(["b",tmp[i+1]])
                    if len(mystring)%5==0:
                        mystring=mystring+"\n"
                # print("games saved:",games, file=sys.stderr)
                # print("len(games):",len(games), file=sys.stderr)
                # print("game_number:",game_number+i, file=sys.stderr)
                # print("i:",i, file=sys.stderr)
                    # to do :
                    # the output filename is not correct

                # move-> to check
                duplicate_index=game_number+i-1
                # move=duplicate_trajectory[duplicate_index]#要檢查的
                # moveNumber=len(duplicate_trajectory[duplicate_index])
                # TODO:重複場次判斷
                # print("moveNumber: ",moveNumber)
                # print("duplicate_index: ",game_number+i)# start from game 1
                # for item in range(0,duplicate_index):#從前面的盤面對照
                #     numbergame=item
                #     print(duplicate_trajectory[numbergame])#get prvious game
                #     move_min=min(moveNumber,len(duplicate_trajectory[numbergame]))
                #     print("move_min :",move_min)
                #     for position in range(0,move_min-1):
                #         print("--------------------------")
                #         print("duplicate_trajectory game:",numbergame+1)#從前面的盤面對照 第幾場
                #         print("trajectory game:",numbergame-game_number+1)#從前面的盤面對照 存在trajectory的第幾盤
                #         print("value in trajectory:",trajectory[numbergame-game_number+1][position])
                #         print("value in duplicate_trajectory:",duplicate_trajectory[numbergame][position])
                #         print("position: ",position)#第幾步
                #         print("duplicate_index: ",game_number+i)
                #         print("value in move :",move[position])
                #         print("move game:",duplicate_index-numbergame)
                #         print("trajectory in move :",trajectory[duplicate_index-numbergame][position])
                filename=args.sgffile+"-"+str(game_number+i)+".txt"
                # print("filename:",filename, file=sys.stderr)
                f = open(filename, "w")
                # print("''.join(trajectory[i]):",mystring, file=sys.stderr)
                f.write(mystring)
                f.close()
                #open and read the file after the overwriting:
                f = open(filename, "r")
                # print(f.read())

                
                
                # try to add args.sgffile+".txt" to record all game result like .dat
                # if game_number==1 and i==0:
                #     mystring_2="Black:\t"+args.black+"\n"
                #     mystring_2+="White:\t"+args.white+"\n"
                # else:
                #     mystring_2=""
                mystring_2=""
                filename_result=args.sgffile+".txt"
                print("filename_result:",filename_result, file=sys.stderr)
                f = open(filename_result, "a")
                mystring_2+="game_number:"+str(game_number+i)
                mystring_2+=":\t"+final_score_tmp[i+1]+"\n"
                # # print("''.join(trajectory[i]):",mystring, file=sys.stderr)
                f.write(mystring_2)
                f.close()

                #open and read the file after the overwriting:
                f = open(filename, "r")
                print(f.read())


            buffer = f"clear_board\n".encode()
            os.write(pipe_1to2[1], buffer)
            os.write(pipe_1to3[1], buffer)
            buffer = os.read(pipe_3to1[0], N)
            os.write(1, buffer)
            buffer = os.read(pipe_2to1[0], N)
            os.write(1, buffer)
            # print(duplicate_trajectory)

            path_one=[]
            trajectory= [[] for i in range(int(args.actor_num))]
            done_list=[]
            start=True
            #alldone
        # if counter==70:
        #     break
        print("game_number",game_number, file=sys.stderr)
        print("args.games",args.games, file=sys.stderr)
        game_number=get_game_number(dir_path)+1
        if game_number>=int(args.games):
            print("---same---")
            quit()
#minizero black
def child2(args):
    os.close(pipe_1to2[1])
    os.close(pipe_2to1[0])
    os.dup2(pipe_1to2[0], 0)
    os.close(pipe_1to2[0])
    os.dup2(pipe_2to1[1], 1)
    os.close(pipe_2to1[1])
    os.close(pipe_1to3[0])
    os.close(pipe_3to1[0])
    os.close(pipe_1to3[1])
    os.close(pipe_3to1[1])

    args_=[]
    for item in args.black.split(' '):
        # print("args.black.split(' '): "+item, file=sys.stderr)
        args_.append(item)
    os.execvp(args_[0], args_)

#minizero white
def child3(args):
    os.close(pipe_1to3[1])
    os.close(pipe_3to1[0])
    os.dup2(pipe_1to3[0], 0)
    os.close(pipe_1to3[0])
    os.dup2(pipe_3to1[1], 1)
    os.close(pipe_3to1[1])
    os.close(pipe_1to2[0])
    os.close(pipe_2to1[0])
    os.close(pipe_1to2[1])
    os.close(pipe_2to1[1])
    args_=[]
    for item in args.white.split(' '):
        # print("args.white.split(' '): "+item, file=sys.stderr)
        args_.append(item)
    os.execvp(args_[0], args_)
def main():
    ## arguments ##
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-black', default='')
    parser.add_argument('-white', default='')
    parser.add_argument('-games', default='0')
    parser.add_argument('-sgffile', default='')
    parser.add_argument('-actor_num', default='1')
    parser.add_argument('-size', default='1')
    args = parser.parse_args()
    mystring_2="Black:\t"+args.black+"\n"+"White:\t"+args.white+"\n"
    filename_result=args.sgffile+".txt"
    f = open(filename_result, "a")
    f.write(mystring_2)
    f.close()
    print("args black: "+args.black)
    print("args white: "+args.white)
    print("args games: "+args.games)
    print("args size: "+args.size)
    pid1 = os.fork()
    if pid1 == 0:
        child1(args)
        exit(0)

    pid2 = os.fork()
    if pid2 == 0:
        child2(args)
        exit(0)

    pid3 = os.fork()
    if pid3 == 0:
        child3(args)
        exit(0)

    os.close(pipe_1to2[1])
    os.close(pipe_2to1[0])
    os.close(pipe_1to3[1])
    os.close(pipe_3to1[0])

    os.waitpid(pid1, 0)
    os.waitpid(pid2, 0)
    os.waitpid(pid3, 0)

if __name__ == '__main__':
    main()
