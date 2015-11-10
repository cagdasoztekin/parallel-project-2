#ifndef UTILS_H
#define UTILS_H

struct node{
	int id;
	unsigned long int similarity;
	struct node* next;
};
char** str_split( char* str, char delim, int* numSplits );
struct node* readfile(char* filename, int* query, int dictionary_size, int* list_size);
int* readquery(char* queryfile, int dictionary_size);
void insertion_sort(unsigned long int *arr, int* ids, int length);

#endif