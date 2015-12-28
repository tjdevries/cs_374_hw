#!/bin/bash

threads=1
i=1
while [ $i -lt 10000000001 ]; do
    echo $i
    while [ $threads -lt 17 ]; do
        ./calcPI2 $i $threads
        let threads=threads*2
    done
    let i=i*10
    let threads=1
done

