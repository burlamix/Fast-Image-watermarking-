echo $1
for ((j=1; j<=2; j++))
do
	for ((i=1; i<=128; i=i*2))
	do
	    ./pool_pq data_128 mark.png output 1 $i $1
	done
echo
done


