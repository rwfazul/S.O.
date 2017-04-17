#/bin/bash

START=1
END=30

ShowTime () {
	time_finish=0
	file=cadeiaProcessos

	for (( c = $START; c<=$END; c++ )) 
	do
		./$file $1 >> out.txt
		tempo=`sed -n '$p' out.txt`
		time_finish=`echo $tempo + $time_finish | bc`
	done

	echo -e "\n"

	echo "total exe: " $END
	echo "total time: " $time_finish

	media=$(echo $time_finish/$END | bc -l)
	echo "media: " $media
	echo -e "\n"
}

gcc -o cadeiaProcessos cadeiaProcessos.c
ShowTime $1

