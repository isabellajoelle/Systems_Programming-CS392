/*******************************************************************************
 * Name        : pfind.c
 * Author      : Isabella Cruz and Julia Lizska
 * Date        : 3/16/21
 * Description : A program that finds files with a specified set of permissions
 * Pledge      : "I pledge my honor that I have abided by the Stevens Honor System."
 ******************************************************************************/
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

char* USAGE = "Usage: ./pfind -d <directory> -p <permissions string> [-h]";

int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR,
               S_IRGRP, S_IWGRP, S_IXGRP,
               S_IROTH, S_IWOTH, S_IXOTH};

int permission_string(struct stat *statbuf) {
	char* return_string = (char *)malloc(11 * sizeof(char *));
	int permission_valid;
	return_string[0] = '\0';
	for(int i = 0; i < 10; i += 3){
		permission_valid = statbuf->st_mode & perms[i-1];
		if(permission_valid){
			return_string[i] = '1';
		}else{
			return_string[i] = '0';
		}
		permission_valid = statbuf->st_mode & perms[i];
		if(permission_valid){
			return_string[i+1] = '1';
		}else{
			return_string[i+1] = '0';
		}
		permission_valid = statbuf->st_mode & perms[i+1];
		if(permission_valid){
			return_string[i+2] = '1';
		}else{
			return_string[i+2] = '0';
		}
	}
	return_string[10] = '\0';
	int ans = atoi(return_string);
	free(return_string);
	return ans;
}

int pfind(char *directory, int pstring){

	char path[PATH_MAX];
	char* real = realpath(directory, path);

	if (real == NULL) {
		fprintf(stderr, "Error: Cannot get full path of file '%s'. %s.\n",
				directory, strerror(errno));
		return EXIT_SUCCESS;
	}

	DIR *dir;
	if ((dir = opendir(path)) == NULL) {
		fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n",
				path, strerror(errno));
		return EXIT_SUCCESS;
	}

	struct dirent *entry;
	struct stat sb;
	char full_filename[PATH_MAX+1];
	size_t pathlen = 0;

	full_filename[0] = '\0';
	if(strcmp(path, "/")){
		strncpy(full_filename, path, PATH_MAX);
	}
	pathlen = strlen(full_filename)+1;
	full_filename[pathlen-1] = '/';
	full_filename[pathlen] = '\0';

	while((entry = readdir(dir)) != NULL){
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
			continue;
		}
		strncpy(full_filename + pathlen, entry->d_name, PATH_MAX - pathlen);
		if(lstat(full_filename, &sb) < 0){
			fprintf(stderr, "Error: Cannot stat file '%s'. %s\n",
					full_filename, strerror(errno));
			continue;
		}
		if(!(permission_string(&sb) ^ pstring) || (permission_string(&sb) == 0 && pstring == 0)){
			printf("%s\n", full_filename);
		}
		if(S_ISDIR(sb.st_mode)){
			pfind(full_filename, pstring);
		}
	}
	closedir(dir);
	return EXIT_SUCCESS;
}

int main(int argc, char ** argv){
	if(argc == 1){
		printf("%s\n", USAGE);
		return EXIT_FAILURE;
	}

	opterr = 0;
	int opt, flag_ctr = 0;
	int flags[2]= {0, 0};
	char* dir_ = NULL;
	char* per = NULL;

	while((opt = getopt(argc, argv, "d:p:h")) != -1){
			switch(opt)
			{
			case 'd':
				if(strcmp(optarg, "-p") == 0){
					printf("Error: Required argument -d <directory> not found.\n");
					return EXIT_FAILURE;
				}else{
					dir_ = optarg;
					flags[flag_ctr] = 1;
					break;
				}
			case 'p':
				per = optarg;
				flags[flag_ctr] = 2;
				break;
			case 'h':
				printf("%s\n", USAGE);
				return EXIT_SUCCESS;
			case '?':
				if(optopt == 'd' || optopt == 'p'){
					break;
				}else{
					//if an unknown flag is given, state it and print a Usage Error
					printf("Error: Unknown option '-%c' received.\n", optopt);
					return EXIT_FAILURE;
				}
			}
			flag_ctr++;
	}

	if(*(flags) != 1){
		printf("Error: Required argument -d <directory> not found.\n");
		return EXIT_FAILURE;
	}
	if(strcmp(argv[1], "-d") != 0){
		printf("Error: Required argument -d <directory> not found.\n");
		return EXIT_FAILURE;
	}
	if(*(flags+1) != 2){
		printf("Error: Required argument -p <permissions string> not found.\n");
		return EXIT_FAILURE;
	}
	if(strcmp(argv[3], "-p") != 0){
		printf("Error: Required argument -p <permissions string> not found.\n");
		return EXIT_FAILURE;
	}

	DIR *dir;
	if((dir = opendir(dir_)) == NULL){
		if(errno == 2){
			fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", dir_, strerror(errno));
		}else{
			char path[PATH_MAX];
			char* real = realpath(dir_, path);
			fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n", real, strerror(errno));
		}
		closedir(dir);
		return EXIT_FAILURE;
	}

	char *pstring = per;

	if(strlen(pstring) != 9){
		printf("Error: Permissions string '%s' is invalid.\n", pstring);
		closedir(dir);
		return EXIT_FAILURE;
	}

	char match = 'r';
	for(int i = 0; i < 9; i++){
		if(i%3 == 0){
			match = 'r';
		}else if(i%3 == 1){
			match = 'w';
		}else{
			match = 'x';
		}
		if(*(pstring + i) != match && *(pstring + i) != '-'){
			printf("Error: Permissions string '%s' is invalid.\n", pstring);
			closedir(dir);
			return EXIT_FAILURE;
		}else{
			if(*(pstring + i) == '-'){
				*(pstring + i) = '0';
			}else{
				*(pstring + i) = '1';
			}
		}
	}

	pfind(dir_, atoi(pstring));
	closedir(dir);

	return EXIT_SUCCESS;
}
