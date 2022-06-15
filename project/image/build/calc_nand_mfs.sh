#!/bin/bash

#$1 is partition fold
#$2 is PAGE SIZE
#$3 is ERASE_BLOCK_SIZE
#$4 is reserved partition size, unit is MB
#$5 is static partition size

#default value
UNIT=1024 #KB
SP=$(($3/1024)) #KB

LEB=$(($3-($2*2))) #KB
DEFAULT_PART_SIZE=$((4*${UNIT}))
PERCENT=100

if [ "$#" -lt 5 ]
    then
    #get partition fold size
    if [ -d $1 ]
        then
        FOLD_PATH=$1
        PART_SIZE=`find ${FOLD_PATH} -print0 | xargs -0 stat --format="%s" | awk '{sum+=$1} END {printf "%.0f\n",(sum/1024/1024)+0.5}'`
        PART_SIZE=$((${PART_SIZE}*${UNIT}))
    else
        PART_SIZE=0
    fi
    #if [ -e $1 ]
    #    then
    #    FOLD_PATH=$1
    #    SIZE=`cat $1 | grep "$5 " | awk '{print $4}'`
    #    ((PART_SIZE=${SIZE}))
    #else
    #    PART_SIZE=0
    #fi

    #add reserved partition size
    PART_SIZE=$((${PART_SIZE}+($4*${UNIT})))

    #check is less than default partition size
    if [ ${PART_SIZE} -lt ${DEFAULT_PART_SIZE} ]
        then
            PART_SIZE=${DEFAULT_PART_SIZE}
    fi
else
    PART_SIZE=$5/${UNIT}
fi

PAGE_SIZE=$(($2/${UNIT}))

#SL=$((${SP}-(2*${PAGE_SIZE})))
SL=$(($LEB/${UNIT}))

P=$((${PART_SIZE}/${SP}))

B1=$((${P}/${PERCENT}))
B2=`echo "${P}/${PERCENT}" | bc -l`
B3=`echo "$B1 < $B2" | bc`
if [ ${B3} -eq 1 ]
    then
    B=$((${B1}+1))
else
    B=${B1}
fi

O=$((${SP}-${SL}))

OVERHEAD_SIZE=$((((${B}+4)*${SP})+(${O}*(${P}-${B}-4))))

PEBS1=$((${OVERHEAD_SIZE}/${SP}))
PEBS2=`echo "${OVERHEAD_SIZE}/${SP}" | bc -l`
PEBS3=`echo "$PEBS1 < $PEBS2" | bc`
if [ ${PEBS3} -eq 1 ]
    then
    PEBS=$((${PEBS1}+1))
else
    PEBS=${PEBS1}
fi

PEBS=$((${P}-${PEBS}))

C=$((${PEBS}*${SP}/${SL}))
C=$((${C}+10))
if [ ${C} -lt 24 ]
	  then
		C=24
fi

#output
echo ${C}
