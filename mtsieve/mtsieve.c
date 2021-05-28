/**********************************************************************************
 * Name        : mtsieve.c
 * Author      : Isabella Cruz & Julia Lizska
 * Date        : April 23, 2021
 * Description : Implement the segmented sieve of eratosthenes to find prime
 *               numbers having two or more digits that are ‘3’
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 *********************************************************************************/

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>

int total_count = 0;
pthread_mutex_t lock;

char *USAGE = "Usage: ./mtsieve -s <starting value> -e <ending value> -t <num threads>\n";

typedef struct arg_struct {
 int start;
 int end;
} thread_args;

bool isNumber(char* number){
	for(int i = 0; i < strlen(number); i++){
		if(!isdigit(number[i])){
			return false;
		}
	}
	return true;
}

bool twoThrees(int num) {
	char* n = (char *)malloc(11*sizeof(char));
	sprintf(n, "%i", num);
	int ctr=0;
	for (int i=0; i<strlen(n); i++) {
		if (*(n+i) == '3') {
			ctr++;
		}
		if (ctr >= 2) {
			free(n);
			return true;
		}
	}
	free(n);
	return false;
}

void *seg_sieve(void *ptr) {
	thread_args arg = *(thread_args *)ptr;

	int sqrt_e = sqrt(arg.end);
	int *temp_primes = (int *)malloc(sqrt_e * sizeof(int));

	for(int i = 2; i < sqrt_e; i++){
		temp_primes[i] = i;
	}

	for(int i = 2; i <= sqrt_e; i++){
		if(temp_primes[i] != 0){
			int j = i*i;
			while(j < sqrt_e){
				temp_primes[j] = 0;
				j += i;
			}
		}
	}

	int prime_count = 0;
	int *low_primes = (int *)malloc(sqrt_e * sizeof(int));

	for(int i = 2; i < sqrt_e; i++){
		if(temp_primes[i] != 0){
			low_primes[prime_count] = temp_primes[i];
			prime_count++;
		}
	}

	int hp_limit = arg.end-arg.start+1;

	bool *high_primes = (bool *)malloc(hp_limit*sizeof(bool));
	for(int i = 0; i < hp_limit; i++){
		high_primes[i] = true;
	}

	for(int p = 0; p < prime_count; p++){
		int i = ceil((double)arg.start/low_primes[p]) * low_primes[p] - arg.start;
		if(arg.start <= low_primes[p]){
			i += low_primes[p];
		}
		for(int k = i; k < hp_limit; k += low_primes[p]){
			high_primes[k] = false;
		}
	}

	int retval;

	for(int i = 0; i < hp_limit; i++){
		if(high_primes[i]){
			if(twoThrees(i+arg.start)){
				if((retval = pthread_mutex_lock(&lock)) != 0){
					fprintf(stderr, "Warning: Cannot lock mutex. %s.\n", strerror(retval));
				}
				total_count++;
				if((retval = pthread_mutex_unlock(&lock)) != 0){
					fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n", strerror(retval));
				}
			}
		}
	}

	free(temp_primes);
	free(low_primes);
	free(high_primes);
	pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
	if (argc == 1) {
		printf("%s", USAGE);
		return EXIT_FAILURE;
	}

	int opt;
 	opterr = 0;
 	long s_input, e_input, t_input;
 	int flags[3] = {0,0,0};

	while ((opt = getopt(argc, argv, "s:e:t:")) != -1) {
		switch (opt) {
			case 's':
				if(!isNumber(optarg)) {
					fprintf(stderr, "Error: Invalid input '%s' received for parameter '-s'.\n", optarg);
					return EXIT_FAILURE;
				}
				s_input = strtol(optarg, NULL, 10);
				if (s_input >= INT_MAX || s_input <= INT_MIN) {
					fprintf(stderr, "Error: Integer overflow for parameter '-s'.\n");
					return EXIT_FAILURE;
				}
				flags[0] = 1;
				break;
			case 'e':
				if (!isNumber(optarg)) {
					fprintf(stderr, "Error: Invalid input '%s' received for parameter '-e'.\n", optarg);
					return EXIT_FAILURE;
				}
				e_input = strtol(optarg, NULL, 10);
				if (e_input >= INT_MAX || e_input <= INT_MIN) {
					fprintf(stderr, "Error: Integer overflow for parameter '-e'.\n");
					return EXIT_FAILURE;
				}
				flags[1] = 1;
				break;
			case 't':
				if (!isNumber(optarg)) {
					fprintf(stderr, "Error: Invalid input '%s' received for parameter '-t'.\n", optarg);
					return EXIT_FAILURE;
				}
				t_input = strtol(optarg, NULL, 10);
				if (t_input >= INT_MAX || t_input <= INT_MIN) {
					fprintf(stderr, "Error: Integer overflow for parameter '-t'.\n");
					return EXIT_FAILURE;
				}
				flags[2] = 1;
				break;
			case '?':
				if (optopt == 'e' || optopt == 's' || optopt == 't') {
				 	fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt);
				}
				else if (isprint(optopt)) {
				 	fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
				}
				else {
				 	fprintf(stderr, "Error: Unknown option character '\\x%x'.\n", optopt);
				}
				return EXIT_FAILURE;
		}
	}

	if (optind < argc){
		fprintf(stderr, "Error: Non-option argument '%s' supplied.\n", argv[optind]);
		return EXIT_FAILURE;
	}

	// starting value
	if(flags[0] == 0){
		fprintf(stderr, "Error: Required argument <starting value> is missing.\n");
		return EXIT_FAILURE;
	}
	if((int)s_input < 2){
		fprintf(stderr, "Error: Starting value must be >= 2.\n");
		return EXIT_FAILURE;
	}

	// ending value
	if(flags[1] == 0){
		fprintf(stderr, "Error: Required argument <ending value> is missing.\n");
		return EXIT_FAILURE;
	}
	if((int)e_input < 2){
		fprintf(stderr, "Error: Ending value must be >= 2.\n");
		return EXIT_FAILURE;
	}

	// compare end and start
	if((int)e_input < s_input){
		fprintf(stderr, "Error: Ending value must be >= starting value.\n");
		return EXIT_FAILURE;
	}

	// number of threads
	if(flags[2] == 0){
		fprintf(stderr, "Error: Required argument <num threads> is missing.\n");
		return EXIT_FAILURE;
	}
	if((int)t_input < 1){
		fprintf(stderr, "Error: Number of threads cannot be less than 1.\n");
		return EXIT_FAILURE;
	}
	if((int)t_input > 2*get_nprocs()) {
		fprintf(stderr, "Error: Number of threads cannot exceed twice the number of processors(%d).\n", get_nprocs());
		return EXIT_FAILURE;
	}

	printf("Finding all prime numbers between %d and %d.\n", (int)s_input, (int)e_input);

	if(t_input > (int)(e_input-s_input)){
		t_input = (int)(e_input-s_input);
	}

	int num_seg = (int)t_input;
	if (num_seg == 0){
		num_seg = 1;
	}

	int num_per_seg = (int)(e_input - s_input) / num_seg;

	thread_args seg[num_seg];
	seg[0].start = s_input;
	seg[0].end = s_input + num_per_seg;

	if(num_seg == 1){
		printf("%i segment:\n", num_seg);
	}else{
		printf("%i segments:\n", num_seg);
	}

	printf("   [%i, %i]\n", seg[0].start, seg[0].end);

	int rem = (int)(e_input - s_input) % num_seg;

	for (int i=1; i<num_seg; i++) {
		if (seg[i-1].end == e_input){
			seg[i].start = e_input;
		}
		else {
			seg[i].start = seg[i-1].end+1;
		}

		if (seg[i].start + num_per_seg > e_input) {
			seg[i].end = e_input;
		}
		else {
			if(rem != 0){
				seg[i].end = seg[i-1].end + num_per_seg + 1;
				rem--;
			}
			else{
				seg[i].end = seg[i-1].end + num_per_seg;
			}
		}
		printf("   [%i, %i]\n", seg[i].start, seg[i].end);

	}


	int retval;
    if ((retval = pthread_mutex_init(&lock, NULL)) != 0) {
        fprintf(stderr, "Error: Cannot create mutex. %s.\n", strerror(retval));
        return EXIT_FAILURE;
    }

	pthread_t *threads = (pthread_t *)malloc(num_seg*sizeof(pthread_t));

	for (int i=0; i<num_seg; i++) {
		if ((retval = pthread_create(&threads[i], NULL, seg_sieve, &seg[i])) != 0) {
		    fprintf(stderr, "Error: Cannot create thread %d. %s.\n", i + 1, strerror(retval));
		    break;
		}
		if ((retval = pthread_mutex_lock(&lock)) != 0) {
		    fprintf(stderr, "Warning: Cannot lock mutex. %s.\n", strerror(retval));
		}
		if ((retval = pthread_mutex_unlock(&lock)) != 0) {
		    fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n", strerror(retval));
		}
	}

	for (int i = 0; i < num_seg; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
		    fprintf(stderr, "Warning: Thread %d did not join properly.\n", i + 1);
		}
	}

	if ((retval = pthread_mutex_destroy(&lock)) != 0) {
        fprintf(stderr, "Error: Cannot destroy mutex. %s.\n", strerror(retval));
    }

    free(threads);
    printf("Total primes between %i and %i with two or more '3' digits: %i\n", (int)s_input, (int)e_input, total_count);

	return EXIT_SUCCESS;
}
