mkdir $2
for letter in {a..z}
do
  touch "$2/$letter.txt"
  for file in $1/*
  do
  egrep -i "^[$letter].*$" $file>>"$2/$letter.txt"
  done
  done
  for file in $2/*.txt
  do
  sort $file -o  $file
  done