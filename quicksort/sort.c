/*******************************************************************************
 * Name        : sort.c
 * Author      : Isabella Cruz and Julia Lizska
 * Date        : 3/2/21
 * Description : Uses quicksort to sort a file of either ints, doubles, or
 *               strings.
 * Pledge      : "I pledge my honor that I have abided by the Stevens Honor System."
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "quicksort.h"

#define MAX_STRLEN     64 // Not including '\0'
#define MAX_ELEMENTS 1024

char* USAGE = "Usage: ./sort [-i|-d] filename\n"\
"   -i: Specifies the file contains ints.\n"\
"   -d: Specifies the file contains doubles.\n"\
"   filename: The file to sort.\n"\
"   No flags defaults to sorting strings.";

typedef enum {
	STRING,
	INT,
	DOUBLE
} elem_t;

/**
 * Basic structure of sort.c:
 *
 * Parses args with getopt.
 * Opens input file for reading.
 * Allocates space in a char** for at least MAX_ELEMENTS strings to be stored,
 * where MAX_ELEMENTS is 1024.
 * Reads in the file
 * - For each line, allocates space in each index of the char** to store the
 *   line.
 * Closes the file, after reading in all the lines.
 * Calls quicksort based on type (int, double, string) supplied on the command
 * line.
 * Frees all data.
 * Ensures there are no memory leaks with valgrind. 
 */
int main(int argc, char **argv) {
	//if no arguments are given; return EXIT_FAILURE
	if(argc == 1){
		printf("%s\n", USAGE);
		return EXIT_FAILURE;
	}

	//suppresses errors
	opterr = 0;

	int opt, flag = 0, num_flags = 0;
	//obtains the given flag(s)
	while((opt = getopt(argc, argv, "id")) != -1){
		switch(opt)
		{
		case '\0':
			//0 signifies the type is string
			flag = 0;
			num_flags++;
			break;
		case 'i':
			//1 signifies the type is int
			flag = 1;
			num_flags++;
			break;
		case 'd':
			//2 signifies the type is double
			flag = 2;
			num_flags++;
			break;
		default:
			//if an unknown flag is given, state it and print a Usage Error
			printf("Error: Unknown option '-%c' received.\n", optopt);
			printf("%s\n", USAGE);
			return EXIT_FAILURE;
		}
	}

	//if too many flags are given, return an EXIT_FAILURE
	if(num_flags > 1){
		printf("Error: Too many flags specified.\n");
		return EXIT_FAILURE;
	}

	char** input = (char **)malloc(MAX_ELEMENTS * sizeof(char *));
	char buffer[MAX_STRLEN + 2];
	int line_count = 0;

	//if files were given
	if(optind < argc){
		int file_count = 1;
		while(optind < argc){
			//if more than 1 file was given, return an EXIT_FAILURE
			if(file_count > 1){
				printf("Error: Too many files specified.\n");
				//frees the memory of the char** array
				for (int i = 0; i < line_count; i++) {
					free(input[i]);
				}
				free(input);
				return EXIT_FAILURE;
			}
			//if the file given exists, copy each value in the char** array input
			if(access(argv[optind], F_OK) == 0){
				FILE *file = fopen(argv[optind], "r");
				while(fgets(buffer, MAX_STRLEN+2, file)){
					char* eoln = strchr(buffer, '\n');
					if(eoln == NULL){
						buffer[MAX_STRLEN] = '\0';
					}else{
						*eoln = '\0';
					}
					input[line_count] = strdup(buffer);
					line_count++;
				}
				file_count++;
				optind++;
				fclose(file);
			}
			//if the file given gives an error, return an EXIT_FAILURE
			else{
				printf("Error: Cannot open '%s'. %s\n", argv[optind], strerror(errno));
				//frees the memory of the char** array
				for (int i = 0; i < line_count; i++) {
					free(input[i]);
				}
				free(input);
				return EXIT_FAILURE;
			}
		}
	}
	//if no input file is given, return an EXIT_FAILURE
	else{
		printf("Error: No input file specified.\n");
		//frees the memory of the char** array
		for (int i = 0; i < line_count; i++) {
			free(input[i]);
		}
		free(input);
		return EXIT_FAILURE;
	}

	// if the flag equals 0, sort strings
	if(flag == 0){
		quicksort(input, line_count, sizeof(char *), str_cmp);
		for(int i = 0; i < line_count; i++){
			printf("%s\n", input[i]);
		}
		//if the flag equals 1, sort ints
	}else if(flag == 1){
		// initialize an array of ints
		int* input_int = (int *)malloc(MAX_ELEMENTS * sizeof(int));
		//copy the integer values of the elements in input to the new array
		for(int i = 0; i < line_count; i++){
			input_int[i] = atoi(input[i]);
		}
		quicksort(input_int, line_count, sizeof(int), int_cmp);
		for(int i = 0; i < line_count; i++){
			printf("%i\n", input_int[i]);
		}

		free(input_int);
		//if the flag equals 2, sorti doubles
	}else{
		// initailize an array of doubles
		double* input_double = (double *)malloc(MAX_ELEMENTS * sizeof(double));
		//copy the double/float values of the elements in input to the new array
		for(int i = 0; i < line_count; i++){
			input_double[i] = atof(input[i]);

		}
		quicksort(input_double, line_count, sizeof(double), dbl_cmp);
		for(int i = 0; i < line_count; i++){
			printf("%f\n", input_double[i]);
		}
		free(input_double);
	}

	//frees the memory of the char** array
	for (int i = 0; i < line_count; i++) {
		free(input[i]);
	}
	free(input);

	return EXIT_SUCCESS;
}
