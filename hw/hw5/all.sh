#!/bin/bash

numProc=4
while [ $numProc -lt 17 ]; do
    echo "Number of processors: $numProc"
    traps=100000
    while [ $traps -lt 1000000000 ]; do
        ./run.sh $numProc $traps
        let traps=traps*10
    done
    let numProc=numProc*2
done
