echo $1 
for ((j=1; j<=3; j++))
do
	for ((i=1; i<=128; i=i*2))
	do
	    ./ff_pipe_farm input mark.png output $i $1 $1
	done
done