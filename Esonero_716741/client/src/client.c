/*
 ============================================================================
 Name        : client.c
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

// terminates use of the Winsock
void clearwinsock();

// handles errors
void errorhandler(char *errorMessage);

int main(void) {
	operation op;
	char operator;
	char space; // check space caracter
	int first_number;
	int second_number;
	char buf[BUFFER_SIZE];

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		errorhandler("WSAStartup() failed!");
		return 0;
	}
#endif

	int c_socket; // socket descriptor for the client
	// client socket creation
	// SOCKET socket(int af, int type, int protocol); return socket_id or -1
	// af: PF_INET, PF_UNIX
	// type: SOCK_STREAM (bytestream protocol for TCP), SOCK_DGRAM (datagram protocol for UDP)
	// protocol: IPPROTO_TCP (TCP protocol), IPPROTO_UDP (UDP protocol)
	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket < 0) {
		errorhandler("socket creation failed!");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	// set connection settings
	struct sockaddr_in sad; // server address structure
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(ADDRESS);
	sad.sin_port = htons(PROTO_PORT);

	// connection with server
	if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("connect() failed!");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	// receive message from server
	memset(buf, '\0', BUFFER_SIZE);
	printf("Waiting for connection: ");
	if ((recv(c_socket, buf, BUFFER_SIZE - 1, 0)) <= 0) {
		errorhandler("recv() failed or connection closed prematurely");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}
	printf("%s\n", buf);


	while (1) {
		// input of string operation
		printf("Enter the math operator and numbers in this format: operator[space]first_number[space]second_number (Es:+ 26 4)\n");
		printf("Enter the '=' character to terminate the process\n");
		operator = '\0';
		space = '\0';
		first_number = 0;
		second_number = 0;
		memset(&op, 0, sizeof(op));
		memset(buf, '\0', BUFFER_SIZE);
		// input of a whitespaced string until [enter]
		while (fgets(buf, BUFFER_SIZE, stdin)){
			// sscanf reads formatted input from a string (buf) and returns the number of variables filled
			//  %1[-+*/=] Before consumes all available consecutive whitespace characters from the input
			//				and one of the specified characters must be entered
			// %c fills the space variable
			// %d fills the first_number variable
			// %d fills the second_number variable
			int count_input = sscanf(buf, " %1[-+*/=]%c%d %d", &operator, &space, &first_number, &second_number);
			// count_input must be equals to 2 because when '=' is entered and [enter] is pressed,
			// space variable is filled with '\n' character
			if (operator == '=' && count_input == 2) {
				closesocket(c_socket);
				clearwinsock();
				return 0;
			}
			// terminate string input when format is valid
			else if (operator != '=' && space == ' ' && count_input == 4) {
				break;
			} else {
				printf("Invalid format!\n");
			}
		}
		op.operator = operator;
		op.first_number = htonl(first_number);
		op.second_number = htonl(second_number);

		// send data of operation
		if (send(c_socket, (void*) &op, sizeof(op), 0) != sizeof(operation)) {
			errorhandler("send() failed or connection closed prematurely!");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}

		// receive the result of operation
		memset(buf, '\0', BUFFER_SIZE);
		if ((recv(c_socket, buf, BUFFER_SIZE - 1, 0)) <= 0) {
			errorhandler("recv() failed or connection closed prematurely!");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}
		printf("Result: ");
		printf("%s\n", buf);
	}

	closesocket(c_socket);
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
