#!/bin/bash

function usage {
	# echo "./fight.sh [BOARD_SIZE] [NUM_FIGHT] [TOTAL_GAMES] [FIGHT_NAME] [BLACK_PROGRAM] [WHITE_PROGRAM] [force/noforce]"
	# ./tools/fight_edax_softmax.sh 8 1 100 minizero_vs_edax_level 21 3000 140000 noforce
	# ./tools/fight_edax_max.sh 8 2 2 minizero_vs_edax_level 22 8000 650 noforce
	# ./tools/fight_edax_max.sh 8 2 2 minizero_vs_edax_level 22 16000 650 noforce
    # python3 ./tools/eval_for_edax.py -d fight_results_Par_Max8000/ -w
	echo "./fight.sh [BOARD_SIZE] [NUM_FIGHT] [TOTAL_GAMES] [FIGHT_NAME] [EDAX_LEVEL] [MINIZERO_SIMULATION] [START_ITER] [OPENINGS_DIR] [force/noforce]"
	exit
}

if [ $# -eq 9 ]; then
	BOARD_SIZE=$1
	NUM_FIGHT=$2
	TOTAL_GAMES=$3
	FIGHT_NAME="model_$7_$4_vs_$5"
	EDAX_LEVEL=$5
	MINIZERO_SIMULATION=$6
	# MODEL_ITERATION=$7
	# BLACK_PROGRAM=$5
	# WHITE_PROGRAM=$6
	START_ITER=$7
    OPENINGS_DIR=$8
    NUM_GPU=$(nvidia-smi -L | wc -l)
    # GPU_LIST=0123
    GPU_LIST=$(echo $NUM_GPU | awk '{for(i=0;i<$1;i++)printf i}')
    GPU_NUMBER=2
	if [[ "$9" == "force" ]]; then
		FORCE="-force"
	elif [[ "$9" == "noforce" ]]; then
		FORCE=""
	else
      		usage
	fi
	FOLDER="othello_8x8_az_20bx256_n256-d902cd-dirty"
	INTERVAL=15
else
  usage
fi

function run_twogtp(){
# BLACK_PROGRAM="./edax-reversi/bin/lEdax-x64 -l ${EDAX_LEVEL} -game-time 15:00.000"
BLACK_PROGRAM="build/othello/minizero_othello -conf_file othello_eval.cfg -conf_str \"nn_file_name=othello_8x8_az_20bx256_n256-d902cd-dirty/model/$2:actor_num_simulation=${MINIZERO_SIMULATION}\""
WHITE_PROGRAM="build/othello/minizero_othello -conf_file othello_eval.cfg -conf_str \"nn_file_name=othello_8x8_az_20bx256_n256-d902cd-dirty/model/$2:actor_num_simulation=${MINIZERO_SIMULATION}\""
# WHITE_PROGRAM="./edax-reversi/bin/lEdax-x64 -l ${EDAX_LEVEL} -game-time 15:00.000"

# WHITE_PROGRAM="Release/minizero -conf_file othello_play_az_test.cfg -conf_str \"nn_file_name=othello_8x8_az_20bx256_n256-d902cd-dirty/model/weight_iter_140000.pt\""
    # BLACK="./Release/minizero -conf_file $CONF_FILE1 -conf_str \"nn_file_name=$FOLDER1/model/$2\""
# Release/minizero -conf_file othello_play_az_test.cfg -conf_str nn_file_name=othello_8x8_az_20bx256_n256-d902cd-dirty/model/weight_iter_140000.pt

# =============================Do not change following argument=============================

# some argument
FIGHT_RESULT_DIR="fight_results_Par_no_forward_Max${MINIZERO_SIMULATION}_vs_edax_${EDAX_LEVEL}_t_15_openings"
FIGHT_ITER="edax_${EDAX_LEVEL}_vs_minizero_${2}"
if [ ! -d "${FIGHT_RESULT_DIR}" ];then
    mkdir ${FIGHT_RESULT_DIR}
fi
if [ ! -d "${FIGHT_RESULT_DIR}/${FIGHT_ITER}" ];then
    mkdir ${FIGHT_RESULT_DIR}/${FIGHT_ITER}
fi
permissions=$(stat -c %a ${FIGHT_RESULT_DIR}/${FIGHT_ITER})
# if [ ! -d "${FIGHT_RESULT_DIR}/${2}/${FIGHT_NAME}" ];then
#     mkdir ${FIGHT_RESULT_DIR}/${2}/${FIGHT_NAME}
# fi
# mkdir fight_results/${FIGHT_NAME}
chmod -R ${permissions} ${FIGHT_RESULT_DIR}/${FIGHT_ITER}
SGF_PREFIX_NAME=${FIGHT_RESULT_DIR}/${FIGHT_ITER}/${FIGHT_ITER}

# fight command
# gogui-twogtp documents: https://www.kayufu.com/gogui/reference-twogtp.html
echo "GPUID: $1, Current players: edax_${EDAX_LEVEL} vs. minizero_${2}, Game num $TOTAL_GAMES"
echo "CUDA_VISIBLE_DEVICES=$1 gogui-twogtp -black "${BLACK_PROGRAM}" -white "${WHITE_PROGRAM}" -auto ${FORCE} -games ${TOTAL_GAMES} -sgffile ${SGF_PREFIX_NAME} -size ${BOARD_SIZE} -threads ${NUM_FIGHT} -openings ${OPENINGS_DIR} -alternate -verbose"

CUDA_VISIBLE_DEVICES=$1 gogui-twogtp -black "${BLACK_PROGRAM}" -white "${WHITE_PROGRAM}" -auto ${FORCE} -games ${TOTAL_GAMES} -sgffile ${SGF_PREFIX_NAME} -size ${BOARD_SIZE} -threads ${NUM_FIGHT} -openings ${OPENINGS_DIR} -alternate -verbose
# CUDA_VISIBLE_DEVICES=$1 gogui-twogtp -black "$BLACK" -white "$WHITE" -games $GAMENUM -sgffile $SGFFILE -alternate -auto -size $BOARD_SIZE -komi 7 -threads 2

gogui-twogtp -analyze $SGF_PREFIX_NAME.dat -force
chmod -R ${permissions} ${FIGHT_RESULT_DIR}/${FIGHT_NAME}
# ==========================================================================================
}


# $2=start
function run_gpu(){
    CUR_NUM=1
    CNT=1
    S=0
    P1=""
    P2=""
    # counter=10002
    for file in $(ls -rt $FOLDER/model | grep .pt )
    do
        # counter=$(($counter-1))
        # if [ $counter -eq 0 ];then
        #     break
        # fi
		# echo "file=$file"
		# echo "$S=$S"
		# echo "$2=$2"
	
        if [ $S -lt $2 ];then
            S=$(($S+1))
            continue
        fi
		# echo "$S not -lt $2"
        if [ $S -eq $2 ];then
            P1=$file
            S=$(($S+1))
            continue
        fi
		# echo "$S not -eq $2"
        if [ $CNT -ne $INTERVAL ];then
            CNT=$(($CNT+1))
            continue
        fi
		# echo "$CNT not -ne $INTERVAL"
        CNT=1
        CUR_NUM=$(($CUR_NUM+1))
        P2=$file
		# echo "CUR_NUM=$CUR_NUM"
		# echo "P1=$P1"
		# echo "P2=$P2"
		# echo "1=$1"

        # P2=$file
		# $1=GPUID
		# $P1=find file
        run_twogtp $1 $P2
        P1=$P2
    done
    echo "GPUID $1 done!"
}
for (( i=0; i < ${#GPU_LIST} ; i = i+1 ))
do
    GPUID=${GPU_LIST:$i:1}
    run_gpu $GPUID $START_ITER &
    sleep 10
done
wait
echo "done!"