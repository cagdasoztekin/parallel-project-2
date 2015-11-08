#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "utils.h"

void kreduce(int * leastk, int * myids, int * myvals, int k, int world_size, int my_rank){
	if(my_rank == 0){
		int i;
		int **leastks = (int**)malloc(world_size * sizeof(int*));
		for(i = 0; i < world_size; i++){
			leastks[i] = (int*)malloc(k * sizeof(int));
		}
		for(i = 1; i < world_size; i++){
			MPI_Send(myids+(k*i), k, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(myvals+(k*i), k, MPI_INT, i, 1, MPI_COMM_WORLD);
		}
		insertion_sort(myvals, myids, k);
		leastks[0] = insertion_sort(myvals, myids, k);
		for(i = 1; i < world_size; i++){
			MPI_Recv(leastks[i-1], k, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(leastks[])
		}
	}
	else{
		MPI_Recv(myids, k, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(myvals, k, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		insertion_sort(myvals, myids, k);
		MPI_Send(myids, k, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(myvals, k, MPI_INT, 0, 3, MPI_COMM_WORLD); 
	}
}

void insertion_sort(int *arr, int* ids, int length) {
int i, j ,tmp, tmp2;
	for (i = 1; i < length; i++) {
		j = i;
		while (j > 0 && arr[j - 1] > arr[j]) {
			tmp2 = ids[j];
			ids[j] = ids[j-1];
			ids[j-1] = tmp2;
			tmp = arr[j];
			arr[j] = arr[j - 1];
			arr[j - 1] = tmp;
			j--;
		}
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
		int	k_value				= atoi(argv[2]);
		if(my_rank == 0){
			int dictionary_size 	= atoi(argv[1]);
			// int	k_value				= atoi(argv[2]);
			char* document_path 	= argv[3];
			char* query_path		= argv[4];
			int list_size;
			char* query = readquery(query_path, dictionary_size);
			struct node* ptr = readfile(document_path, query, dictionary_size, &list_size);
			int i;
			int *leastk = (int*)malloc(k_value * sizeof(int));
			int *myvals = (int*)malloc(list_size * sizeof(int));
			int *myids = (int*)malloc(list_size * sizeof(int)); 

			while(ptr->next != NULL){
				// printf("ID: %d, sim: %d\n", ptr->id, ptr->similarity);
				myvals[i] = ptr->similarity;
				myids[i] = ptr->id;
				ptr = ptr->next;
				++i;
			}

			kreduce(leastk, myids, myvals, k_value, world_size, my_rank);

		}
		else{
			int *myvals = (int*)malloc(k_value * sizeof(int));
			int *miyds = (int*)malloc(k_value * sizeof(int));
			int *leastk = (int*)malloc(k_value * sizeof(int));
			kreduce(leastk, myids, myvals, k_value, world_size, my_rank);
		}
	}

	return 0;
}