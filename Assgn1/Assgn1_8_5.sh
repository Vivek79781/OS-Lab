if [ ! -f main.csv ]
then
	touch main.csv
	echo "Date (dd-mm-yyyy),Category,Amount,Name" >> main.csv
fi
header=$(head -n 1 main.csv)
args=($@)
echo "${args[-4]},${args[-3]},${args[-2]},${args[-1]}" >> main.csv
echo "Inserted ${args[-4]},${args[-3]},${args[-2]},${args[-1]} in main.csv"
tail -n +2 main.csv | awk -F, '
    BEGIN {PROCINFO["sorted_in"] = "@ind_str_asc"}
    NR==1 {print $0; next}
    {   
        split ($1, d, /[-]/); 
        time = mktime (d[3] " " d[2] " " d[1] " 00 00 00"); 
        sorted[time] = ((time in sorted) ? sorted[time] RS $0 : $0)
    } 
    END {for (key in sorted) print sorted[key]}
' > temp
echo "$header" > main.csv
cat temp >> main.csv
while getopts ":c:n:s:h" opt
do
	case $opt in 
	c)
	categoryTotal=$(awk -F, -v category="$OPTARG" '$2==category {sum+=$3} END{print sum}' main.csv)
	echo "Total Amount Spent On $OPTARG: $categoryTotal"
	;;
	n)
	nameTotal=$(awk -F, -v name="$OPTARG" '$4==name {sum+=$3} END{print sum}' main.csv)
	echo "Total Amount Spent By $OPTARG: $nameTotal"
	;;
	s)
	columnNumber=$(echo "$header" | awk -F "," '{ for(i=1;i<=NF;i++) if($i == "'$OPTARG'") { print i; exit}}')
	tail -n +2 main.csv | sort -t, -nk$columnNumber > temp
	echo "$header" > main.csv
	cat temp >> main.csv
	;;
	h)
	printf "\033[1mNAME\033[0m\n\tAssgn1_8_5.sh\n\033[1mSYNOPSIS\033[0m\n\t./Assgn1_8_5.sh [-c category] [-n name] [-s column] date category amount name\n\033[1mDESCRIPTION\033[0m\n\tThis script tracks expenses by inserting new records into a main.csv file and allows for querying expenses by category, name and sorting by column name.\n"
	exit
	;;
esac
done
