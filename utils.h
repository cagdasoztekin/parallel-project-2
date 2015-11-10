#ifndef UTILS_H
#define UTILS_H

struct node{
	int id;
	int similarity;
	struct node* next;
};
void freelist(struct node* root);
char** str_split( char* str, char delim, int* numSplits );
struct node* readfile(char* filename, int* query, int dictionary_size, int* list_size);
int* readquery(char* queryfile, int dictionary_size);
int similarity(int* vals, int* query, int size);
void insertion_sort(int *arr, int* ids, int length);

#endif