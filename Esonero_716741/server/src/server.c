/*
 ============================================================================
 Name        : server.c
 Author      : Michele Pio Ardo'
 ============================================================================
 */

#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"
#define QLEN 5 // size of request queue

// terminates use of the Winsock
void clearwinsock();

// handles errors
void errorhandler(char *errorMessage);

// returns the sum of two numbers
int add(int first_number, int second_number);

// returns the subtraction of two numbers
int sub(int first_number, int second_number);

// returns the product of two numbers
int mult(int first_number, int second_number);

// returns the division of two numbers
double division(int first_number, int second_number);

// handles client connection
int handleclientconnection(int client_socket);

int main(void) {

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int my_socket; // socket descriptor for the server
	// server socket creation
	// SOCKET socket(int af, int type, int protocol); return socket_id or -1
	// af: PF_INET, PF_UNIX
	// type: SOCK_STREAM (bytestream protocol for TCP), SOCK_DGRAM (datagram protocol for UDP)
	// protocol: IPPROTO_TCP (TCP protocol), IPPROTO_UDP (UDP protocol)
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket < 0) {
		errorhandler("socket creation failed!");
		clearwinsock();
		return -1;
	}

	// set connection settings
	struct sockaddr_in sad; // server address structure
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(ADDRESS);
	sad.sin_port = htons(PROTO_PORT);

	// binding the address to the socket
	if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("bind() failed!");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	// set socket to listen
	if (listen(my_socket, QLEN) < 0) {
		errorhandler("listen() failed!");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	struct sockaddr_in cad; // client address structure
	int client_socket; // socket descriptor for the client
	int client_len; // length client address


	while (1) {
		// accept new connection
		printf("Waiting for a client to connect...\n");
		client_len = sizeof(cad);
		if ((client_socket = accept(my_socket, (struct sockaddr*) &cad, &client_len)) < 0) {
			errorhandler("accept() failed!");
			closesocket(my_socket);
			clearwinsock();
			return -1;
		}

		// send message to client
		printf("Connection established with %s:%d\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));
		char* input_string = "Connection established";
		int string_len = strlen(input_string);
		if (send(client_socket, input_string, string_len, 0) != string_len) {
			errorhandler("send() sent a different number of bytes than expected!");
			closesocket(client_socket);
		}

		while (1) {
			if (handleclientconnection(client_socket) == -1)
				break;
		}
	}

	// close server socket
	closesocket(my_socket);
	clearwinsock();
	return 0;
}

// terminates use of the Winsock
void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

// print error message
void errorhandler(char *errorMessage) {
	printf("%s\n", errorMessage);
}

// returns the sum of two numbers
int add(int first_number, int second_number) {
	return first_number + second_number;
}

// returns the subtraction of two numbers
int sub(int first_number, int second_number) {
	return first_number - second_number;
}

// returns the product of two numbers
int mult(int first_number, int second_number) {
	return first_number * second_number;
}

// returns the division of two numbers
double division(int first_number, int second_number) {
	return (double) first_number / second_number;
}

// handles client connection
int handleclientconnection(int client_socket) {
	operation op;

	// receive operation from the client
	if ((recv(client_socket, (void*) &op, sizeof(operation), 0)) <= 0) {
		errorhandler("recv() failed or connection closed prematurely!");
		closesocket(client_socket);
		return -1;
	}

	op.first_number = ntohl(op.first_number);
	op.second_number = ntohl(op.second_number);

	// performs the desired operation
	double risultato;
	switch (op.operator) {
		case '+':
			risultato = add(op.first_number, op.second_number);
			break;
		case '-':
			risultato = sub(op.first_number, op.second_number);
			break;
		case '*':
			risultato = mult(op.first_number, op.second_number);
			break;
		case '/':
			risultato = division(op.first_number, op.second_number);
			break;
		default:
			break;
	}
	char buf[BUFFER_SIZE];
	sprintf(buf, "%g", risultato);

	// sends the result of the operation performed to the client
	if (send(client_socket, buf, strlen(buf), 0) != strlen(buf)) {
		errorhandler("send() failed or connection closed prematurely!");
		closesocket(client_socket);
		return -1;
	}

	return 0;
}
