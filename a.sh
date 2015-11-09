#!/bin/bash
mpicc -o doc main.c utils.h utils.c -lm
mpirun -np 2 ./doc 2 2 documents.txt query.txt
