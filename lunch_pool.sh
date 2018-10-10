echo -------------------pool_pqversion--------------------------

echo $1
for ((j=1; j<=3; j++))
do
	for ((i=3; i<=254; i=i+3))
	do
	    #./pool_pq data_254 mark.png output 1 $i sml
	    #echo $i

	    a=$((i/3))
	    #echo ------ $a

		if [ $i -gt 100 ]
		then
			i=$i+6;
		fi

		if [ $i -gt 45 ]
		then
			i=$i+6;
		fi


	done

	echo
done

