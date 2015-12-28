#!/bin/bash

numProc=1
while [ $numProc -lt 17 ]; do
    echo "Number of processors: $numProc"
    traps=1
    while [ $traps -lt 10000000001 ]; do
        ./calcPI2 $traps $numProc
        let traps=traps*10
    done
    let numProc=numProc*2
done
