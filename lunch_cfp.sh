echo $1
for ((j=1; j<=3; j++))
do
	for ((i=1; i<=32; i=i+1))
	do
	    ./c_pipe_farm data_128 mark.png output 1 $i $1
	done
echo
done