echo ----------------------fast flow--------------


echo $1
for ((j=1; j<=3; j++))
do
	for ((i=3; i<=254; i=i+3))
	do
	    #echo $i

	    a=$((i/3))
	    #echo ------ $a
	    ./ff_pipe_farm data_254 mark.png output 1 $a pof

		if [ $i -gt 50 ]
		then
			i=$i+6
		fi
		if [ $i -gt 100 ]
		then
			i=$i+6
		fi
	done

	echo
done
