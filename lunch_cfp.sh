echo -------------------c++ version--------------------------


echo $1
for ((j=1; j<=4; j++))
do
	for ((i=3; i<=128; i=i+3))
	do
	    #echo $i

	    a=$((i/3))

	    ./c_pipe_farm data_254 mark.png output 1 pof $a

		if [ $i -gt 60 ]
		then
			i=$i+6
		fi

	done

	echo
done
