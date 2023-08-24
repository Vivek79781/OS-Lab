while read username 
do 
if egrep -vif"fruits.txt" $1 | egrep -vi "^[A-Z]*$" | egrep "^[a-zA-Z][A-Za-z0-9]{4,19}$" | egrep -qw $username
then echo "YES"
else echo "NO"
fi
done < $1 > validation_result.txt