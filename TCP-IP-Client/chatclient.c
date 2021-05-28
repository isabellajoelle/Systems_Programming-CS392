/**********************************************************************************
 * Name        : chatclient.c
 * Author      : Julia Liszka & Isabella Cruz
 * Date        : May 7, 2021
 * Description : A chat client that is able to communicate with a server.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 *********************************************************************************/

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"

int client_socket = -1;
char username[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];

int handle_stdin() {
	//check if message is too long
	if(get_string(outbuf, MAX_MSG_LEN) == TOO_LONG){
		fprintf(stderr, "Sorry, limit your message to %d characters.\n", MAX_NAME_LEN);
	}

	//send message to server
	if (send(client_socket, outbuf, strlen(outbuf), 0) < 0) {
		fprintf(stderr, "Error: Failed to send message to server. %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if(strcmp(outbuf, "bye") == 0){
		printf("Goodbye.\n");
		return 1;
	}

	return 0;
}

int handle_client_socket() {

	int bytes_recvd = recv(client_socket, inbuf, BUFLEN, 0);
	if (bytes_recvd < 0) {
		fprintf(stderr,"Error: Failed to receive incoming message. %s.\n", strerror(errno));
		return 1;
	}else if (bytes_recvd == 0) {
		fprintf(stderr, "\nConnection to server has been lost.\n");
		return 1;
	}else{
		inbuf[bytes_recvd] = '\0';

		if(strcmp(inbuf, "bye") == 0){
			printf("\nServer initiated shutdown.\n");
			return 1;
		}else{
			printf("\n%s\n", inbuf);
			fflush(stdout);
			return 0;
		}
	}

}

int main(int argc, char *argv[]) {
	// Usage
	if (argc == 1) {
		fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int retval = EXIT_SUCCESS;

	// IP address
	struct sockaddr_in addr;
	char* ip_input = argv[1];
	if (inet_pton(AF_INET, ip_input, &addr.sin_addr) <= 0) {
		fprintf(stderr, "Error: Invalid IP address '%s'.\n", ip_input);
		return EXIT_FAILURE;
	}

	addr.sin_family = AF_INET;

	// Port
	int port = 0;
	char* port_input = argv[2];
	if (!parse_int(port_input, &port, "port number")) {
		return EXIT_FAILURE;
	}
	if (port > 65535 || port < 1024) {
		fprintf(stderr, "Error: Port must be in range [1024, 65535].\n");
		return EXIT_FAILURE;
	}

	addr.sin_port = htons(port);

	// Username
	printf("Enter your username: ");
	while (scanf("%s", username) == EOF || strlen(username) > MAX_NAME_LEN) {
		fprintf(stderr, "Sorry, limit your username to %d characters.\n", MAX_NAME_LEN);
		printf("Enter your username: ");
	}
	printf("Hello, %s. Let's try to connect to the server.\n", username);

	// Establishing connection

	// create a TCP socket
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}

	// connect to server
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if (connect(client_socket, (struct sockaddr *)&addr, addrlen) < 0) {
		fprintf(stderr, "Error: Failed to connect to server. %s.\n", strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}

	// receive welcome message
	int bytes_recvd;
	if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN - 1, 0)) < 0) {

        fprintf(stderr,"Error: Failed to receive message from server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }
    else if (bytes_recvd == 0) {
    	fprintf(stderr, "All connections are busy. Try again later.\n");
    	retval = EXIT_FAILURE;
        goto EXIT;
    }
    inbuf[bytes_recvd] = '\0';
    printf("\n%s\n\n", inbuf);

    // send the username to the server
    if (send(client_socket, username, strlen(username), 0) < 0) {
        fprintf(stderr, "Error: Failed to send username to server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    int stdin;
    int hcs;
    fd_set active_fd_set;

    while(true){
    	FD_ZERO(&active_fd_set);
    	FD_SET(client_socket, &active_fd_set);
    	FD_SET(STDIN_FILENO, &active_fd_set);

    	printf("[%s]: ", username);
    	fflush(stdout);

    	if(select(FD_SETSIZE, &active_fd_set, NULL, NULL, NULL) < 0){
    		fprintf(stderr, "Error: Select Failed. %s.\n", strerror(errno));
    		retval = EXIT_FAILURE;
    		goto EXIT;
    	}

    	if(FD_ISSET(STDIN_FILENO, &active_fd_set)){
    		stdin = handle_stdin();
    		if(stdin == 1){
    			retval = EXIT_SUCCESS;
    			goto EXIT;
    		}
    	}


    	if(FD_ISSET(client_socket, &active_fd_set)){
    		hcs = handle_client_socket();
    		if(hcs == 1){
    			retval = EXIT_SUCCESS;
    			goto EXIT;
    		}
    	}

    }

EXIT:
    if (fcntl(client_socket, F_GETFD) >= 0) {
        close(client_socket);
    }
    return retval;
}
