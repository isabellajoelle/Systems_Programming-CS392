/**********************************************************************************
 * Name        : spfind.c
 * Author      : Isabella Cruz & Julia Lizska
 * Date        : March 31, 2021
 * Description : Sorted output of pfind (with total matches counted) using pipes
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 *********************************************************************************/

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char *USAGE = "Usage: ./spfind -d <directory> -p <permissions string> [-h]";

bool starts_with(const char *str, const char *prefix) {
    if (strlen(prefix) > strlen(str)) { // if prefix can't be bigger than what we compare it to
      return false;
    }

    for (int i=0; i < strlen(prefix); i++) {
      if (str[i] != prefix[i]) { // check if prefix matches
        return false;
      }
    }
    return true;
}

int main(int argc, char *argv[]) {

	if (argc == 1) { // no arguments given
		printf("%s\n", USAGE);
		return EXIT_SUCCESS;
	}

	//create pipes
	int pfind_to_sort[2], sort_to_parent[2];
	if (pipe(pfind_to_sort) < 0) {
		fprintf(stderr, "Error: Cannot create pipe pfind_to_sort. %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if (pipe(sort_to_parent) < 0) {
		fprintf(stderr, "Error: Cannot create pipe sort_to_parent. %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}


    pid_t pid_pfind;
	pid_t pid_sort;

	if ( (pid_pfind = fork()) < 0) {
		fprintf(stderr, "Error: Failed to fork. %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
    else if (pid_pfind == 0) {
        // in pfind

        // Close all unrelated file descriptors.
        close(sort_to_parent[0]);
        close(sort_to_parent[1]);
        close(pfind_to_sort[0]);

        if (dup2(pfind_to_sort[1], STDOUT_FILENO) < 0) {
			fprintf(stderr, "Error: Failed to dup2 pfind_to_sort[1].\n");
			close(pfind_to_sort[0]);
			return EXIT_FAILURE;
		}

		// find path to pfind
        char path[PATH_MAX];
		realpath("pfind", path);

		// execute pfind
        if (execv(path, argv) < 0) {
        	fprintf(stderr, "Error: pfind failed.\n");
        	exit(EXIT_FAILURE);
        }
    }

    if ((pid_sort = fork()) == 0) {
        // in sort

		// Close all unrelated file descriptors.
        close(pfind_to_sort[1]);
        close(sort_to_parent[0]);

        if (dup2(pfind_to_sort[0], STDIN_FILENO) < 0) {
			fprintf(stderr, "Error: Failed to dup2 pfind_to_sort[0].\n");
			close(pfind_to_sort[0]);
			close(sort_to_parent[1]);
			return EXIT_FAILURE;
		}

        if (dup2(sort_to_parent[1], STDOUT_FILENO) < 0) {
			fprintf(stderr, "Error: Failed to dup2 sort_to_parent[1].\n");
			close(pfind_to_sort[0]);
			close(sort_to_parent[1]);
			return EXIT_FAILURE;
		}

		// execute sort from output of pfind
        if (execlp("sort", "sort", "-f", NULL) < 0) {
        	fprintf(stderr, "Error: sort failed.\n");
        	exit(EXIT_FAILURE);
        }
    }

	// Close all unrelated file descriptors.
    close(sort_to_parent[1]);
    close(pfind_to_sort[0]);
    close(pfind_to_sort[1]);

    if (dup2(sort_to_parent[0], STDIN_FILENO) < 0) {
		fprintf(stderr, "Error: Failed to dup2 sort_to_parent[0].\n");
		close(sort_to_parent[0]);
		return EXIT_FAILURE;
	}


    char buffer[128];
    int matches = 0;
	while(1){// loop until break
		ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer)-1);
		if (count < 0) { // read fails
			close(sort_to_parent[0]);
			perror("read()");
			exit(EXIT_FAILURE);
		}

		buffer[count] = '\0';
		if (count == 0) { // done reading
			break;
		}
		else { // reading

			write(STDOUT_FILENO, buffer, count); // write output

			if (starts_with(buffer, USAGE)) { // exit if usage is in output
        		close(sort_to_parent[0]);
        		exit(EXIT_SUCCESS);
        	}

			for (int i=0; i < count; i++) { // check each character to count each newline
				if (buffer[i] == '\n') {
					matches++;
				}
			}
		}
	}


    //wait for pfind
    int status;
    pid_t wait_pfind = waitpid(pid_pfind, &status, 0);
    if (wait_pfind == -1) { // waitpid failed
        perror("waitpid()");
        exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status)) { // pfind exited
        if (WEXITSTATUS(status) == 1) { // pfind exited with error status
        	close(sort_to_parent[0]);
        	exit(EXIT_FAILURE);
        }
    }

    // wait for sort
    wait(NULL);

    close(sort_to_parent[0]);

    printf("Total matches: %i\n", matches);

    return EXIT_SUCCESS;
}
