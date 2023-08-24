echo "$(cat $1|awk '{print($2)}'|sort|uniq -c|sort -k1,1nr -k2,2|awk '{$1=$1} 1')"
printf "\n$(cat $1|awk '{print($1)}'|sort|uniq -d)\n"
echo "$(cat $1|awk '{print($1)}'|sort|uniq -u|wc -l)"
