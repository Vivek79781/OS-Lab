while read -r LINE
do
if [ `echo $LINE | grep -c $2` -gt 0 ]
then
    echo `sed 's/[A-Z]/\L&/g'<<<$LINE` | sed 's/[[:lower:]][^[:alpha:]]*\([[:alpha:]]\|$\)/\u&/g'
else
    echo $LINE
fi
done < $1
