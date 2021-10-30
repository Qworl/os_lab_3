#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include "unistd.h"
#include <stdint.h>
#include <inttypes.h>
#include "string.h"
#include <math.h>

void read_filename(string* str, int fd){
	char cur = 'a';
	while(read(fd, &cur, sizeof(char)) > 0){
		if(cur == '\n'){
			break;
		}
		str_push(str, cur);
	}
}

typedef enum{
	reading_suc,
	reading_wrong_value,
	reading_eof,
	spare_eol,
	eof,
} read_rvl_stat;

read_rvl_stat reading_128_int(int fd, __uint128_t* cur){
	char c;
	*cur = 0;
	int k = read(fd, &c, sizeof(char));
	bool fnd_smth = false;
	int alr_read = 0;
	while(k > 0){
		if(c == '\n'){
			if(fnd_smth) return reading_eof;
			else return spare_eol;
		}
		if(c == ' '){
			if(fnd_smth) break;
			else return spare_eol;
		}
		if((c < 'a' || c > 'f') && (c < '0' || c > '9')){
			return reading_wrong_value;
		}
		fnd_smth = true;
		alr_read++;
		if(alr_read > 32){
			return reading_wrong_value;
		}
		if(c >= 'a' && c <= 'f')
			*cur = *cur * 16 + (c - 'a' + 10);
		if(c >= '0' && c <= '9')
			*cur = *cur * 16 + c - '0'; 
		k = read(fd, &c, sizeof(char));
	}
	if(k == 0){
		if (fnd_smth) return reading_eof;
		else return eof;
	}
	return reading_suc;
}

typedef struct{
	__uint128_t sum;
	bool res;
	int cnt;
	int fd;
} result_value;

pthread_mutex_t mutex;
bool end = false;

void* func(void* thread_data){
	result_value* res = (result_value*) thread_data;
	__uint128_t sum = 0;
	__uint128_t cur;
	while(true){
		pthread_mutex_lock(&mutex);
		if(end){
			res->res = true;
			break;
		}
		read_rvl_stat last_read = reading_128_int(res->fd, &cur);
		if(last_read == reading_wrong_value){
			res->res = false;
			end = true;
			break;
		} else if (last_read == eof){
			res->res = true;
			end = true;
			break;
		} else if (last_read == reading_eof){
			res->res = true;
			sum += cur;
			res->cnt++;
			end = true;
			break;
		} else if (last_read == spare_eol){
			res->res = true;
		} else {
			res->res = true;
			sum += cur;
			res->cnt++;
		}
		pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_unlock(&mutex);
	res->sum = sum;
	pthread_exit(0);
}

int main(int argc, char* argv[]){
	if(argc > 3){
		printf("Wrong argument number\n");
		printf("a.out <Memory size> <b for bytes/kb for kilobytes/mb for megabytes>\n");
		return 0;
	}
	long int N;
	if(argc == 3){
		long int sum_mem = atol(argv[1]);
		if(sum_mem <= 0){
			printf("Wrong thread number\n");
			return 0;
		}
		int status = -1;
		if(!strcmp(argv[2], "b")) status = 0;
		if(!strcmp(argv[2], "kb")) status = 1;
		if(!strcmp(argv[2], "mb")) status = 2;
		if(status == -1){
			printf("Wrong argument number\n");
			printf("a.out <Memory size> <b for bytes/kb for kilobytes/mb for megabytes>\n");
			return 0;
		}
		for(int i = 0; i < status; i++) sum_mem *= 1024;
		N = (sum_mem - 2652) / 8196;
		if(N <= 0){
			printf("%ld - Wrong thread number\n", N);
			return 0;
		}
	}
	if(argc == 1){
		N = 1;
	}
	if(argc == 2){
		long int sum_mem = atol(argv[1]);
		if(sum_mem <= 0){
			printf("Wrong thread number\n");
			return 0;
		}
		N = (sum_mem * 1024 - 2652) / 8196;
		if(N <= 0){
			printf("%ld - Wrong thread number\n", N);
			return 0;
		}
	}
	string file_name;
	str_create(&file_name);
	read_filename(&file_name, 0); // 0 = STDIN
	str_push(&file_name, '\0');
	int file = open(str_get_all(&file_name), O_RDONLY);
	if(file == -1){
		perror("File can't be opened");
		str_destroy(&file_name);
		return -5;
	}
	str_destroy(&file_name);
	pthread_mutex_init(&mutex, NULL);
	pthread_t threads[N];
	result_value threads_rvl[N];
	for(int i = 0; i < N; i++){
		threads_rvl[i].sum = 0;
		threads_rvl[i].res = false;
		threads_rvl[i].cnt = 0;
		threads_rvl[i].fd = file;
	}
	for(int i = 0; i < N; i++){
		pthread_create(&threads[i], NULL, func, &threads_rvl[i]);
	}
	for(int i = 0; i < N; i++){
		pthread_join(threads[i], NULL);
	}
	__uint128_t res_sum = 0;
	int res_cnt = 0;
	for(int i = 0; i < N; i++){
		if(!threads_rvl[i].res){
			printf("Wrong value in file!\n");
			pthread_mutex_destroy(&mutex);
			return 0;
		}
		res_sum += threads_rvl[i].sum;
		res_cnt += threads_rvl[i].cnt;
	}
	res_sum = res_sum / res_cnt;
	__uint128_t high = (res_sum >> 64);
	if(high){
		printf("%" PRIx64, (uint64_t)high);
	}
	printf("%llx\n", (unsigned long long)(res_sum & 0xFFFFFFFFFFFFFFFF));
	pthread_mutex_destroy(&mutex);
	return 0;
}
