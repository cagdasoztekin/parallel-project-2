#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "utils.h"

// struct node{
// 	// int id;
// 	int *vals;
// 	struct node* next;
// };

// TAKEN FROM http://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
char** str_split( char* str, char delim, int* numSplits )
{
    char** ret;
    int retLen;
    char* c;

    if ( ( str == NULL ) ||
        ( delim == '\0' ) )
    {
        ret = NULL;
        retLen = -1;
    }
    else
    {
        retLen = 0;
        c = str;
        do
        {
            if ( *c == delim )
            {
                retLen++;
            }

            c++;
        } while ( *c != '\0' );

        ret = malloc( ( retLen + 1 ) * sizeof( *ret ) );
        ret[retLen] = NULL;

        c = str;
        retLen = 1;
        ret[0] = str;

        do
        {
            if ( *c == delim )
            {
                ret[retLen++] = &c[1];
                *c = '\0';
            }

            c++;
        } while ( *c != '\0' );
    }

    if ( numSplits != NULL )
    {
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

int similarity(int* vals, int* query, int dictionary_size){
	int sim = 0;
	int i;
	for(i = 0; i < size; i++){
		sim += pow(vals[i], query[i]);
	}
	return ((int)sim);
}

struct node* readfile(char* filename, int* query, int dictionary_size, ){
	FILE *fp = fopen(filename, "r");
	int* len = 0;
    char* line = NULL;
    int count = 0;
    int num = 0;
    int i;
    int cursim;
    int *vals = (int*)malloc(size* sizeof(int));
    struct node * root = (struct node *)malloc(sizeof(struct node*));
    struct node * ptr = root;
	while ((getline(&line, &len, fp)) != -1){
		// sample input "2: 1 6 4 67"
		char** tokens = str_split(line, ' ', &num);
		// for(i = 0; i < num; i++){
			// printf("Token %d is %s\n", i, tokens[i]);
		// }
		ptr->id = atoi(tokens[0]);
		for(i = 0; i < size; i++){
			vals[i] = atoi(tokens[i+1]);
		}
		cursim = similarity(vals, query, size);
		ptr->similarity = cursim;

		ptr->next = (struct node*)malloc(sizeof(struct node*));
		ptr = ptr->next;
		free(tokens);

	}
	fclose(fp);

	return root;
}