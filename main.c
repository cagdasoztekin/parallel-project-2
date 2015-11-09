#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "utils.h"



void kreduce(int * leastk, int * myids, int * myvals, int k, int world_size, int my_rank){
	if(my_rank == 0){
		int i;
		int j;
		int **leastkvals = (int**)malloc(world_size * sizeof(int*));
		int **leastkids = (int**)malloc(world_size * sizeof(int*));
		// printf("K is %d\n", k);
		// for(i = 0; i < k * world_size; i++){
			// printf("i %d id %d val %d\n", i, myids[i], myvals[i]);
		// }

		for(i = 0; i < world_size; i++){
			leastkvals[i] = (int*)malloc(k * sizeof(int));
			leastkids[i] = (int*)malloc(k * sizeof(int));
		}
		for(i = 1; i < world_size; i++){
			MPI_Send(myids+(k*i), k, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(myvals+(k*i), k, MPI_INT, i, 1, MPI_COMM_WORLD);
		}
		insertion_sort(myvals, myids, k);
		leastkvals[0] = myvals;
		leastkids[0] = myids;
		for(i = 1; i < world_size; i++){
			MPI_Recv(leastkids[i], k, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(leastkvals[i], k, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		// for(i = 0; i < world_size; i++){
			// for(j = 0; j < k; j++){
				// printf("i: %d, j: %d, id: %d, val: %d\n", i, j, leastkids[i][j], leastkvals[i][j]);
			// }
		// }

		int* leastval = (int*)malloc(k * sizeof(int));
		int* leastid  = (int*)malloc(k * sizeof(int));

		int change = 1;
		int count = 0;
		// printf("K is %d\n", k);
		for(i = 0; i < world_size && change == 1; i++){
			change = 0;
			// printf("i is %d\n", i);
			for(j = 0; j < k; j++){
				if(count < k){
					leastid[count] = leastkids[i][j];
					leastval[count] = leastkvals[i][j];
					count += 1;
					change = 1;
				}
				else{
					
					if(leastval[k-1] > leastkvals[i][j]){
						leastval[k-1] = leastkvals[i][j];
						leastid[k-1] = leastkids[i][j];
						change = 1;
					}
					insertion_sort(leastval, leastid, k);
				}
				// int a;
				// for(a = 0; a < count; a++){
					// printf("Element %d is %d iteration i %d j %d\n", a, leastid[a], i, j);
				// }
			}
		}
		// printf("Count is %d\n", count);

		for(i = 0; i < k; i++){
			// printf("leastid %d is %d\n", i, leastid[i]);
			leastk[i] = leastid[i];
		}
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
		// int* ids;
		// int* vals;
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
		MPI_Init(&argc, &argv);
		int my_rank, world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
		int	k= atoi(argv[2]);
		int *leastk = (int*)malloc(k * sizeof(int));
		if(my_rank == 0){
			int dictionary_size 	= atoi(argv[1]);
			// int	k				= atoi(argv[2]);
			char* document_path 	= argv[3];
			char* query_path		= argv[4];
			int list_size = 0;
			char* query = readquery(query_path, dictionary_size);
			struct node* ptr = readfile(document_path, query, dictionary_size, &list_size);
			int i;
			
			int *myvals = (int*)malloc(list_size * sizeof(int));
			int *myids = (int*)malloc(list_size * sizeof(int)); 
			// printf("List size is %d\n", list_size);
			for(i = 0; i < list_size; i++){
				// printf("ID: %d, sim: %d\n", ptr->id, ptr->similarity);
				myvals[i] = ptr->similarity;
				myids[i] = ptr->id;
				ptr = ptr->next;
			}

			kreduce(leastk, myids, myvals, k, world_size, my_rank);

			for(i = 0; i < k; i++){
				printf("Least k %d is with id %d\n", i, leastk[i]);
			}
			MPI_Barrier(MPI_COMM_WORLD);
			free(leastk);
			// free(myids);
			// free(myvals);
			// freelist(ptr);

		}
		else{
			int *myvals = (int*)malloc(k * sizeof(int));
			int *myids = (int*)malloc(k * sizeof(int));
			// no memory allocation for leastk, slave processors will not be using that
			int *leastk;
			kreduce(leastk, myids, myvals, k, world_size, my_rank);
			MPI_Barrier(MPI_COMM_WORLD);
			free(myids);
			free(myvals);

		}
	}

	MPI_Finalize();

	return 0;
}