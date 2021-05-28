/**********************************************************************************
 * Name        : minishell.c
 * Author      : Isabella Cruz & Julia Liszka
 * Date        : April 13, 2021
 * Description :
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 *********************************************************************************/

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <wait.h>

#define BRIGHTBLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"

volatile sig_atomic_t interrupted = 0;

void minishell();
void cd(char** input, int length);
int normalExit();

void catch_signal(int sig) {
	if(sig == SIGINT){
		printf("\n");
		interrupted = 1;
	}
}

void minishell() {

	char path[PATH_MAX];
	if (getcwd(path, PATH_MAX) == NULL) {
		fprintf(stderr, "%sError: Cannot get current working directory. %s.\n", DEFAULT, strerror(errno));
	}
	size_t path_length = 0;
	while (path[path_length] != '\0') {
		path_length++;
	}

	write(STDOUT_FILENO, DEFAULT, sizeof(DEFAULT));
	write(STDOUT_FILENO, "[", 1);

	write(STDOUT_FILENO, BRIGHTBLUE, sizeof(BRIGHTBLUE));
	write(STDOUT_FILENO, path, path_length);

	write(STDOUT_FILENO, DEFAULT, sizeof(DEFAULT));
	write(STDOUT_FILENO, "]$ ", 3);


	struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = catch_signal;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) < 0) {
    	fprintf(stderr, "%sError: Cannot register signal handler. %s.\n", DEFAULT, strerror(errno));
    }

	// gets user input
	size_t s = 4096;
	char user_input[s];
	user_input[0] = '\0';
	size_t read_ctr = 0;
	char ui;
	int read_result = 0;

	while((read_result = read(STDIN_FILENO, &ui, 1)) && read_ctr < s-1 ) {
		if (read_result == -1 && errno != EINTR) {
			fprintf(stderr, "%sError: Failed to read from stdin. %s.\n", DEFAULT, strerror(errno));
		}
		if(interrupted == 1){
			interrupted = 0;
			minishell();
			exit(EXIT_SUCCESS);
			break;
		}
		if (ui == '\n') {
			break;
		}
		user_input[read_ctr++] = ui;
	}

	user_input[read_ctr] = '\0';

	char *input; // used to take each input argument separated by spaces
	input = strtok(user_input, " "); // takes up to first space of input

	int ctr = 0;
	char** args;
	if ((args = (char **)malloc(1024 * sizeof(char *))) == NULL) {
		fprintf(stderr, "%sError: malloc() failed. %s.\n", DEFAULT, strerror(errno));
	}

	while (input != NULL) {
		args[ctr] = input;
		input = strtok(NULL, " ");
		ctr++;
	}
	args[ctr] = '\0';

	if (strcmp(args[0], "cd") == 0) {
		//call cd function
		cd(args, ctr);
		free(args);
		minishell();

	}
	else if (strcmp(args[0], "exit") == 0) {
		free(args);
		normalExit();
	}
	else {
		pid_t pid;
		if ((pid = fork()) < 0) {
			fprintf(stderr, "%sError: fork() failed. %s.\n", DEFAULT, strerror(errno));
		}
		else if (pid == 0) {

			char temp[ctr+5];
			strcpy(temp, "/bin/");
			strcat(temp, args[0]);

			if (execv(temp, args) < 0){
				fprintf(stderr, "%sError: exec() failed. %s.\n", DEFAULT, strerror(errno));
				//free(args);
				exit(EXIT_SUCCESS);
			}
		}

		if (wait(NULL) < 0 && errno != EINTR) {
			fprintf(stderr, "%sError: wait() failed. %s.\n", DEFAULT, strerror(errno));
		}
		wait(NULL);
		if(interrupted == 1){
			interrupted = 0;
	//		minishell();
	//		exit(EXIT_SUCCESS);
		}

		free(args);
		minishell();
	}
}

void cd(char **input, int length) {
	if(length > 2) {
		fprintf(stderr, "%sError: Too many arguments to cd.\n", DEFAULT);
	}
	else if (length == 1 || strcmp(input[1],"~") == 0) {
		// no directory or '~' --> go to user home directory
		struct passwd *pw;
		if ((pw = getpwuid(getuid())) == NULL) {
			fprintf(stderr, "%sError: Cannot get passwd entry. %s.\n", DEFAULT, strerror(errno));
		}
		else {
			// change dir to home dir
			const char *homedir = pw->pw_dir;
			if (chdir(homedir) == 1) {
				fprintf(stderr, "%sError: Cannot change directory to '%s'. %s.\n",
						DEFAULT, homedir, strerror(errno));
			}
		}
	}
	else {
		// change to a specified directory
		if (chdir(input[1]) == 1) {
			fprintf(stderr, "%sError: Cannot change directory to '%s'. %s.\n",
					DEFAULT, input[1], strerror(errno));
		}
	}
}

int normalExit() {
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
	if (argc > 1) {
		printf("Error: Too many arguments.\n");
		return EXIT_FAILURE;
	}
	minishell();
	return EXIT_SUCCESS;
}
