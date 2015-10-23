#!/bin/bash

mpirun -np $1 -machinefile hosts ./calcPI $2
