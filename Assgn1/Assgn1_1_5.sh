gcd()(c=$(bc<<<"$1%$2")
!((c))&&echo $2||gcd $2 $c)
lcm()(p=$(bc<<<"$1*$2")
q=$(gcd $1 $2)
echo $(bc<<<"$p/$q"))
nums=($(rev $1))
a=${nums[0]}
for item in "${nums[@]}"
do
a=$(lcm $a $item)
done
echo $a