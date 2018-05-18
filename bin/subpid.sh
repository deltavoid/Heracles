#!/bin/bash

input=$1 
pids=`ps -ef | awk '{if('$input'==$3){print $2}}'`  
for pid in ${pids}  
do  
  echo ${pid}  
done 