#!/bin/sh

if [ "$#" -ne  2 ];
then
	echo "wrong amount of parameter"
	exit 1
fi

if [ ! -d "$1" ];
then
	echo "no directory"
	exit 1
fi

my_count_x=$( ls $1 | wc -l)
my_count_y=$( grep -r $2 $1 | wc -l)

echo The number of files are $my_count_x and the number of matching lines are $my_count_y.
