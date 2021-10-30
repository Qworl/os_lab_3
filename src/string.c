#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "string.h"

void str_create(string *s){
	s->buf = NULL;
	s->size = 0;
	s->capacity = 0;
}

void str_destroy(string *s){
	s->size = 0;
	free(s->buf);
	s->capacity = 0;
	s->buf = NULL;
}

int str_capacity(string *s){
	return s->capacity;
}

int str_size(string *s){
	return s->size;
}

bool str_is_empty(string *s){
	return s->size == 0;
}

bool str_grow_buf(string *s){
	int tmp = s->capacity * 3 / 2;
	if(!tmp){
		tmp = 10;
	}
	char *newd = realloc(s->buf, sizeof(char) * tmp);
	if(newd != NULL) {
		s->buf = newd;
		s->capacity = tmp;
		return true;
	}
	return false;
}

bool str_push(string *s, char new_char){
	if(str_size(s) == str_capacity(s)){
		if(!str_grow_buf(s))
			return false;
	}
	s->buf[str_size(s)] = new_char;
	s->size++;
	return true;
}

bool str_shrink_buf(string *s){
	int tmp = s->capacity * 4 / 9;
	if(tmp < str_size(s)){
		return true;
	}
	char *newd = realloc(s->buf, sizeof(char) * tmp);
	if(newd != NULL) {
		s->buf = newd;
		s->capacity = tmp;
		return true;
	}
	return false;
}

char str_pop(string *s){
	char tmp = s->buf[str_size(s) - 1];
	str_shrink_buf(s);
	s->size--;
	return tmp;
}

char str_get(string *s, int i){
	return s->buf[i];
}

char* str_get_all(string *s){
	return s->buf;
}