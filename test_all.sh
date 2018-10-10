echo ++++++++++++++++++++++++++++++++++pool_pq with different priority 1-100  +++++++++++++++++++++++++++++++++

echo -------------------------lms--------------------------------
for ((j=1; j<=5; j++))
do
	for ((i=1; i<=80; i=i+1))
	do
	    ./pool_pq data_512 mark.png output 1 $i lms
	done
done

echo
echo -------------------------sml-------------------------------
for ((j=1; j<=5; j++))
do
	for ((i=1; i<=80; i=i+1))
	do
	    ./pool_pq data_512 mark.png output 1 $i sml
	done
done


echo
echo --------------------msl------------------------------
for ((j=1; j<=5; j++))
do
	for ((i=1; i<=80; i=i+1))
	do
	    ./pool_pq data_512 mark.png output 1 $i msl
	done
done

echo
echo pool_pq 1-128
echo ----------------------msl--------------------------
for ((j=1; j<=5; j++))
do
	for ((i=1; i<=128; i=i*2))
	do
	    ./pool_pq data_512 mark.png output 1 $i msl
	done
echo
done


echo 
echo

echo -----------++++++++++++++++--------c++ version--------------+++++++++++++++++++------------

echo pof
for ((j=1; j<=5; j++))
do
	for ((i=1; i<=128; i=i*2))
	do
	    ./c_pipe_farm data_512 mark.png output 1 $i pof
	done
echo
done

echo fop
for ((j=1; j<=5; j++))
do
	for ((i=1; i<=128; i=i*2))
	do
	    ./c_pipe_farm data_512 mark.png output 1 $i fop
	done
echo
done

echo 
echo

echo ----------++++++++++++++++++++++------------fast flow------+++++++++++++++++++++++++++++++--------

echo pof
for ((j=1; j<=5; j++))
do
	for ((i=1; i<=128; i=i*2))
	do
	    ./ff_pipe_farm data_512 mark.png output 1 $i pof
	done
done


echo pof
for ((j=1; j<=5; j++))
do
	for ((i=1; i<=128; i=i*2))
	do
	    ./ff_pipe_farm data_512 mark.png output 1 $i pof
	done
done