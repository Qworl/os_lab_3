#ifndef STRING_H_ 
#define STRING_H_
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
	int size;
	int capacity;
	char *buf;
} string;

void str_create(string *s);
void str_destroy(string *s);
int str_capacity(string *s);
int str_size(string *s);
bool str_is_empty(string *s);
bool str_grow_buf(string *s);
bool str_push(string *s, char new_char);
bool str_shrink_buf(string *s);
char str_pop(string *s);
char str_get(string *s, int i);
char* str_get_all(string *s);

#endif