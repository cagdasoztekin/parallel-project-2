#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "utils.h"

// TAKEN FROM http://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
// I know the lines from documents.txt will only be processed in the root processor
// but I was still scared to use the non thread-safe function strtok()
char** str_split( char* str, char delim, int* numSplits ) {
    char** ret;
    int retLen;
    char* c;

    if (( str == NULL ) || ( delim == '\0' )) {
        ret = NULL;
        retLen = -1;
    }
    else {
        retLen = 0;
        c = str;
        do {
            if ( *c == delim ) {
                retLen++;
            }

            ++c;
        } while ( *c != '\0' );

        ret = (char**)malloc( ( retLen + 1 ) * sizeof( *ret ) );
        ret[retLen] = NULL;

        c = str;
        retLen = 1;
        ret[0] = str;

        do {
            if ( *c == delim ) {
                ret[retLen++] = &c[1];
                *c = '\0';
            }

            c++;
        } while ( *c != '\0' );
    }

    if ( numSplits != NULL ) {
        *numSplits = retLen;
    }

    return ret;
}

int* readquery(char* queryfile, int size){
	FILE *fp = fopen(queryfile, "r");
	int* len = 0;
	char* line = NULL;
	while ((getline(&line, &len, fp)) != -1){
	}
	fclose(fp);
	int num = 0;
	char** tokens = str_split(line, ' ', &num);
	int* vals = (int *)malloc(size * sizeof(int));
	int i;
	for(i = 0; i < size; i++){
		vals[i] = atoi(tokens[i]);
	}
	free(tokens);

	return vals;
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

int similarity(int* vals, int* query, int size){
	int sim = 0;
	int i;
	for(i = 0; i < size; i++){
		sim += pow(vals[i], query[i]);
	}
	return ((int)sim);
}

struct node* readfile(char* filename, int* query, int dictionary_size, int* list_size){
	FILE *fp = fopen(filename, "r");
	int* len = 0;
    char* line = NULL;
    // int count = 0;
    int num = 0;
    int i;
    (*list_size) = 0;
    printf("Size comes as %d\n", (*list_size));
    int cursim;
    int *vals = (int*)malloc(dictionary_size * sizeof(int));
    struct node * root = (struct node *)malloc(sizeof(struct node*));
    struct node * ptr = root;
	while ((getline(&line, &len, fp)) != -1){
		// sample input "2: 1 6 4 67"
		char** tokens = str_split(line, ' ', &num);
		// for(i = 0; i < num; i++){
			// printf("Token %d is %s\n", i, tokens[i]);
		// }
		ptr->id = atoi(tokens[0]);
		for(i = 0; i < num - 1; i++){
			vals[i] = atoi(tokens[i+1]);
		}
		cursim = similarity(vals, query, dictionary_size);
		ptr->similarity = cursim;

		ptr->next = (struct node*)malloc(sizeof(struct node*));
		ptr = ptr->next;

        (*list_size) += 1;
        // count++;
		free(tokens);

	}
	fclose(fp);
    // printf("count is %d\n", count);
    // list_size = count;
    // printf("size in readfile %d\n", (*list_size));
    // size = count;

	return root;
}

void kreduce(int * leastk, int * myids, int * myvals, int k, int world_size, int my_rank){
	if(my_rank == 0){
		int i;
		int j;
		int **leastkvals = (int**)malloc(world_size * sizeof(int*));
		int **leastkids = (int**)malloc(world_size * sizeof(int*));
		printf("K is %d\n", k);
		for(i = 0; i < k * world_size; i++){
			printf("i %d id %d val %d\n", i, myids[i], myvals[i]);
		}

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
		for(i = 0; i < world_size; i++){
			for(j = 0; j < k; j++){
				printf("i: %d, j: %d, id: %d, val: %d\n", i, j, leastkids[i][j], leastkvals[i][j]);
			}
		}

		int* leastval = (int*)malloc(k * sizeof(int));
		int* leastid  = (int*)malloc(k * sizeof(int));

		int change;
		int count = 0;
		for(i = 0; i < world_size && change == 1; i++){
			change = 0;
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
					}
					insertion_sort(leastval, leastid, k);
				}
			}
		}

		for(i = 0; i < k; i++){
			leastk[i] = leastid[i];
		}

	}
	else{
		// int* ids;
		// int* vals;
		MPI_Recv(myids, k, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(myvals, k, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Hello there I'm %d and I received an array of integers of size %d\n", my_rank, k);
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
			printf("List size is %d\n", list_size);
			for(i = 0; i < list_size; i++){
				printf("ID: %d, sim: %d\n", ptr->id, ptr->similarity);
				myvals[i] = ptr->similarity;
				myids[i] = ptr->id;
				ptr = ptr->next;
			}

			kreduce(leastk, myids, myvals, k, world_size, my_rank);

			for(i = 0; i < k; i++){
				printf("Least k %d is with id %d\n", i, leastk[i]);
			}

		}
		else{
			int *myvals = (int*)malloc(k * sizeof(int));
			int *myids = (int*)malloc(k * sizeof(int));
			// no memory allocation for leastk, slave processors will not be using that
			int *leastk;
			kreduce(leastk, myids, myvals, k, world_size, my_rank);

		}
	}

	MPI_Finalize();

	return 0;
}