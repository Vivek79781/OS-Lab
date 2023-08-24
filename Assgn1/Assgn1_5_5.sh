#!/bin/bash


# Assign the input directory path to a variable
dir_path=$1

# Iterate through all python files in the directory and its subdirectories
find "$dir_path" -name "*.py" -print0 | 
while IFS= read -r -d $'\0' file
do
    echo "File: $file"
    t=0
    lineno=1
    count=0
    while read -ra line; 
    do
        # wordcount=0
        flag=0
        flaghash=0
        for word in "${line[@]}";
        do
            y="${word//[^\"]}"
            z=${#y}
            # wordcount=$((wordcount+1))
            count=$(( $count + $z ))
            if [[ $t -eq 1 ]]; then
                printf "%s " "$word"
                flag=1
            fi
            
            if [[ "$word" =~ ^\"\"\" && $t -eq 0  && $((count%2)) -eq 1 ]]
            then
                t=1
                if [[ $flag -eq 0 ]]
                then
                    printf "Line No. $lineno "
                fi
                flag=1
                printf "%s " "$word"
                # else
                #     t=0
                # fi
            elif [[ "$word" =~ \"\"\"$ && $t -eq 1 ]]
            then
                t=0
            fi
            if [[ flaghash -eq 1 ]]
            then
                
                printf "%s " "$word"
            fi
            if [[ "$word" =~ \# ]]
            then
                if [[ $((count%2)) == 0 ]]
                then 
                    flaghash=1;
                    if [[ $flag -eq 0 ]]
                    then
                        flag=1;
                        printf "Line No. $lineno "
                    fi
                    printf "%s " "$word"
                fi
            fi
        done;
        
        lineno=$((lineno+1))      
        if [[ $flag -eq 1 ]]
        then
            printf "\n"
        fi
    done < $file
    
done


# gawk '
#     while read -ra line; 
# do
#     for word in "${line[@]}";
#     do
#         echo "$word";
#     done;
# done < test.txtBEGIN {
#         multiline_comment = 0;
#         line_num = 1;
#     }
#     {
        
        
#         double_quote_count = 0;
#         split($0, fields, "\"");
#         for (i in fields) {
#             if (fields[i] == "\"") {
#                 double_quote_count++;
#             }
#         }
#         if (match($0, /\"\"\"/)) {
#             if (multiline_comment == 0) {
#                 print "Line " line_num ": Multi-line Comment Started";
#                 multiline_comment = 1;
#             } else {
#                 print $0
#                 print "Line " line_num ": Multi-line Comment Ended";
#                 multiline_comment = 0;
#             }
#         }
#         if (match($0, /\"\"\"/)) {
#             if (multiline_comment == 0) {
#                 print "Line " line_num ": Multi-line Comment Started";
#                 multiline_comment = 1;
#             } else {
#                 print $0
#                 print "Line " line_num ": Multi-line Comment Ended";
#                 multiline_comment = 0;
#             }
#         }

#         if (multiline_comment == 1) {
#                 print line_num $0
#                 multiline_comment = 1;
#             }

#         line_num++;

#     }
#     ' $file