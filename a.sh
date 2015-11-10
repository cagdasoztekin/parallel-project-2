#!/bin/bash
mpicc -o doc main.c utils.h utils.c -lm
mpirun -np 2 ./doc 4 5 documents.txt query.txt
