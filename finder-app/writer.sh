#!/bin/sh

if [ "$#" -ne  2 ];
then
	echo "wrong amount of parameter"
	exit 1
fi

echo $2 > $1

if [ $? -ne 0 ];
then
	exit 1
fi

