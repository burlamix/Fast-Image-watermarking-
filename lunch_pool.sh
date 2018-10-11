echo -------------------pool_pqversion--------------------------


echo $1
for ((j=1; j<=4; j++))
do
	for ((i=3; i<=128; i=i+3))
	do


	    a=$((i/3))

	    ./pool_pq data_254 mark.png output 1 $i msl
	    #echo $i

	    #echo ------ $a

		if [ $i -gt 60 ]
		then
			i=$i+6;
		fi


	done

	echo
done

