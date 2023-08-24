for i in `seq $1 $(($1+10))`
do 
kill -9 $i
done