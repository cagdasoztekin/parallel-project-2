#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "utils.h"

void kreduce(int * leastk, int * myids, int * myvals, int k, int world_size, int my_rank){
	if(my_rank == 0){
		int i;
		int j;
		// leastkvals and leastkids will hold data that will come from each processing element
		int **leastkvals = (int**)malloc(world_size * sizeof(int*));
		int **leastkids = (int**)malloc(world_size * sizeof(int*));

		// allocate memory space for each processor's own arrays
		for(i = 0; i < world_size; i++){
			leastkvals[i] = (int*)malloc(k * sizeof(int));
			leastkids[i] = (int*)malloc(k * sizeof(int));
		}

		// distribute parts of the whole array to the processing elements
		// the data is guaranteed to be of length k
		for(i = 1; i < world_size; i++){
			MPI_Send(myids+(k*i), k, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(myvals+(k*i), k, MPI_INT, i, 1, MPI_COMM_WORLD);
		}

		// the next 3 lines is the computation done in the root processor,
		// root processor sorts its own part of the array and puts the result in the 0-th index
		// of the 2-d arrays that holds the data that arrives from each processor
		insertion_sort(myvals, myids, k);
		leastkvals[0] = myvals;
		leastkids[0] = myids;

		// receive the results from each processor and put them in the 2-d arrays
		for(i = 1; i < world_size; i++){
			MPI_Recv(leastkids[i], k, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(leastkvals[i], k, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		// these arrays will be put the least values and the corresponding id's
		int* leastval = (int*)malloc(k * sizeof(int));
		int* leastid  = (int*)malloc(k * sizeof(int));

		// an int to track if there is a change to the leastval/leastid
		// meaning that if all the least elements possible have been added so far
		int change = 1;
		// count will go from 0 to k, to initially fill the leastval/leastid with the first integers
		// that are found in the results of each processing element
		int count = 0;

		// if there is no change to the resulting array, meaning that all the least values have been found
		// there is no need to continue looking for more, so change == 1 is a predicate to the loop as well
		// the loops may run world_size * k in the worst case, which is all the least k elements having been
		// sent to the last processor to be checked, and each element that comes from the last processor
		// is among the least k elements
		for(i = 0; i < world_size && change == 1; i++){
			// initialize change to 0 with each iteration of the outer loop
			change = 0;
			for(j = 0; j < k; j++){
				// the least elements of the received arrays from each processing element are checked in order
				// for example, the 0-th index of all the received arrays are checked, then the 1-st index
				// until the k-th index if need be
				if(count < k){
					// there are not yet k numbers in the resulting array so any number that comes will be added to
					// the count-th index of the final array
					leastid[count] = leastkids[i][j];
					leastval[count] = leastkvals[i][j];
					// increment count, so the next index to add an integer is determined
					count += 1;
					// now there's been a change
					change = 1;
					if(count == k){
						// now it is ready to be sorted so that we can put the max element in this array at the last index
						// to help ease with comparing with other integers later
						// insertion_sort swaps the id's according to the values as well
						insertion_sort(leastval, leastid, k);
					}
				}
				else{
					// if the maximum element of the resulting array is greater than the tested number from a received array
					// swap those numbers and swap the id's as well
					// insertion_sort swaps the id's according to the values as well
					if(leastval[k-1] > leastkvals[i][j]){
						leastval[k-1] = leastkvals[i][j];
						leastid[k-1] = leastkids[i][j];
						change = 1;
					}
					insertion_sort(leastval, leastid, k);
				}
			}
		}

		// at this point the the leastid array holds the id's of the documents with the least similarity values
		// so we can move the data to the leastk array, which is one of the parameters
		for(i = 0; i < k; i++){
			leastk[i] = leastid[i];
		}
		// now is the time to free the memory allocations
		free(leastid);
		free(leastval);
		for(i = 0; i < world_size; i++){
			free(leastkvals[i]);
			free(leastkids[i]);
		}
		free(leastkvals);
		free(leastkids);

	}
	else{
		// the slave processors receive the values and ids and return them in the sorted order according to the values
		MPI_Recv(myids, k, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(myvals, k, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		// printf("Hello there I'm %d and I received an array of integers of size %d\n", my_rank, k);
		insertion_sort(myvals, myids, k);
		MPI_Send(myids, k, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(myvals, k, MPI_INT, 0, 3, MPI_COMM_WORLD); 
	}
}



int main(int argc, char** argv){
	if(argc < 5){
		printf("Not enough arguments. Rerun with moar arguments please.\n");
	}
	else{
		// start mpi
		MPI_Init(&argc, &argv);
		int my_rank, world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        // k and leastk are common to all processing elements so initialize them for all
		int	k= atoi(argv[2]);
		int *leastk = (int*)malloc(k * sizeof(int));
		if(my_rank == 0){
			double serialstart, serialend, parallelend;
			serialstart = MPI_Wtime();
			int dictionary_size 	= atoi(argv[1]);
			char* document_path 	= argv[3];
			char* query_path		= argv[4];
			int list_size = 0;
			char* query = readquery(query_path, dictionary_size);
			// the linked list, holds the id's and the similarity values in seqeunce
			// and the list_size will hold the size of the list after readfile function returns
			struct node* ptr = readfile(document_path, query, dictionary_size, &list_size);
			struct node* root = ptr;
			int i;
			
			int *myvals = (int*)malloc(list_size * sizeof(int));
			int *myids = (int*)malloc(list_size * sizeof(int)); 
			// move the data from the linked list to arrays so that it can be distributed
			for(i = 0; i < list_size; i++){
				// printf("ID: %d, sim: %d\n", ptr->id, ptr->similarity);
				myvals[i] = ptr->similarity;
				myids[i] = ptr->id;
				ptr = ptr->next;
			}

			// put a barrier before calling kreduce, up to this point things were in serial
			MPI_Barrier(MPI_COMM_WORLD);
			serialend = MPI_Wtime();
			// call kreduce
			kreduce(leastk, myids, myvals, k, world_size, my_rank);
			// parallel part is over
			MPI_Barrier(MPI_COMM_WORLD);
			parallelend = MPI_Wtime();

			// the result is here
			for(i = 0; i < k; i++){
				printf("Least k %d is with id %d\n", i, leastk[i]);
			}
			printf("Serial Runtime: %f\n", serialend-serialstart);
			printf("Parallel Runtime: %f\n", parallelend-serialend);
			
			
			free(leastk);
			free(myids);
			free(myvals);
			// free the linked list
			freelist(root);

		}
		else{
			int *myvals = (int*)malloc(k * sizeof(int));
			int *myids = (int*)malloc(k * sizeof(int));
			// no memory allocation for leastk, slave processors will not be using that
			int *leastk;
			// put a barrier before calling kreduce 
			MPI_Barrier(MPI_COMM_WORLD);
			// call kreduce from the slaves
			kreduce(leastk, myids, myvals, k, world_size, my_rank);
			// put a barrier, end of the parallel part and then free the allocated memory space
			MPI_Barrier(MPI_COMM_WORLD);
			free(leastk);
			free(myids);
			free(myvals);

		}
	}

	MPI_Finalize();

	return 0;
}