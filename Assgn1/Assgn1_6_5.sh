
prime=()
prime[0]=1
prime[1]=1
for ((i=2;i<=1000000;i++))
do
	prime[i]=$i
done
for ((i=2;i<=1000000;i++))
do
	if [ ${prime[i]} -eq $i ]
	then
		for ((j=i;i*j<=1000000;j++))
		do
			z=$((i*j))
			if [[ ${prime[z]} -eq $z ]]
			then
			prime[i*j]=$i
			fi
		done
		#echo "$i"
	fi
done

nums=($(cat $1))
#echo "${nums[1]}"
for num in "${nums[@]}"
do
	#echo "$num"
	if [ -z "$num" ]
	then
		continue
	fi
	while [[ $num -gt 1 ]]
	do
		printf "${prime[num]} "
		 z=${prime[$num]}
		 num=$((num/z))
	done
	printf "\n"
done > output.txt
