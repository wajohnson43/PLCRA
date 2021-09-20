#!/bin/bash

#3648:		top
#164:		piControl I/O
#1:		systemd
#2:		kthread
#1548:		cron

for pid in 1 2 1549 2029 
do
	echo "pid: $pid"
	for j in {1..10}
	do
		echo "j: $j"
		for i in {1..100}
		do
			#sudo ./dump/dvm $pid
			sudo ./dump/dvm $pid &>> times/${pid}_${j}.raw
			yes | rm ${pid}_c.dump
			echo "i: $i"
		done
	done

	#grep 'real' exp2.raw >> exp2.dat
	#yes | rm exp2.raw
done
