#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "utils.h"

// struct node{
// 	// int id;
// 	int *vals;
// 	struct node* next;
// };

void freelist(struct node* root){
    while(root != NULL){
        struct node * tmp = root;
        root = root->next;
        free(tmp);
    }

    free(root);
}

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

void insertion_sort(unsigned long int *arr, int* ids, int length) {
    int i, j ,tmp2;
    unsigned long tmp;
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

unsigned long int similarity(int* vals, int* query, int size){
    unsigned long int sim = 0;
    int i;
    for(i = 0; i < size; i++){
        sim += pow(vals[i], query[i]);
    }
    return ((unsigned long int)sim);
}

struct node* readfile(char* filename, int* query, int dictionary_size, int* list_size){
    FILE *fp = fopen(filename, "r");
    int* len = 0;
    char* line = NULL;
    // int count = 0;
    int num = 0;
    int i;
    (*list_size) = 0;
    // printf("Size comes as %d\n", (*list_size));
    unsigned long int cursim;
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