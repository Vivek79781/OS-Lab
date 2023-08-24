for file in $(find $1 -name "*.jsonl");do
temp=${file%.*};{
    echo "$3,$4,$5";jq -r "[.$3,.$4,.$5]|@csv" $file;}>"$2/${temp##*/}.csv";done