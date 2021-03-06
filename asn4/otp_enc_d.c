/*********************************************************************
CS 344 Program 4
otp_enc_d.c, Encryption Server
Taylor Griffin
03/15/18
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

// SPECIFY encrypt (1), decrypt (0)
char id = '1';

// Process variables
int numConnections = 0;
int checkConnections = 1;
int connectionPID [5] = {-2,-2,-2,-2,-2};
int establishedConnectionFD = -1;
// Storage
char plainTextBuffer [80000];
char keyBuffer [80000];
char cipherBuffer [80000];
// i/o
char transmitBuffer [160000];
char recBuffer [160000];

// Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(1); }
// Checks for zombie processes
void checkDeadChildren();
// Finds next avaliable index in array
int findNextAvail(int*,int,int);

int main(int argc, char *argv[]) {
	// network things
	int listenSocketFD, portNumber, charsRead, charsSent, index;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;
	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress));
	portNumber = atoi(argv[1]);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0) error("ERROR opening socket");
	// Connect socket to port
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		error("ERROR on binding");
	}
	// Flip the socket on - it can now receive up to 5 connections
	listen(listenSocketFD, 5);
	// Continue to listen for communication
	while (1) {
		// Check for zombies
		checkDeadChildren();
		// Check if server is full
		if (numConnections < 5) {
			// Accept a connection, blocking if one is not available until one connects
			sizeOfClientInfo = sizeof(clientAddress);
			establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
			if (establishedConnectionFD < 0) error("ERROR on accept");
			else { numConnections++; }
			// Create child process
			pid_t pid = fork();
			// Quick parent process stuff
			if (pid != 0) {
				index = findNextAvail(connectionPID,5,-2);
				connectionPID[index] = pid;
			}
			// Child process breaks off
			if (pid == 0) {
				charsRead = 0;
				while (charsRead == 0) {
					charsRead = recv(establishedConnectionFD, recBuffer, sizeof(recBuffer), MSG_WAITALL);
					if (charsRead < 0) error("ERROR reading from socket");
				}
				//Report back error if wrong client
				if (recBuffer[0] == id) {
					memset(cipherBuffer, '\0', sizeof(plainTextBuffer));
					cipherBuffer[0] = '?';
					charsRead = send(establishedConnectionFD, cipherBuffer, sizeof(cipherBuffer), 0);
					if (charsRead < 0) error("ERROR writing to socket");
					exit(0);
				}
				// Parse data recieved
				int i;
				int break1;
				int break2;
				int plainTextIndex = 0;
				int keyIndex = 0;
				// Find division between message and key
				for (i=0;i<sizeof(recBuffer);i++) {
					if (recBuffer[i] == '\n')
					break;
				}
				// Store plain text in its own buffer
				break1 = i;
				for (i=2;i<break1;i++) {
					plainTextBuffer[plainTextIndex]=recBuffer[i];
					plainTextIndex++;
				}
				// Find eof
				for (i=break1+1;i<sizeof(recBuffer);i++) {
					if (recBuffer[i] == '\n')
					break;
				}
				break2 = i;
				// Store key in its own buffer
				for (i=break1+1;i<break2;i++) {
					keyBuffer[keyIndex]=recBuffer[i];
					keyIndex++;
				}
				// Do the encryption
				char temp;
				int keyInt;
				int cipherInt;
				int plainTextInt;
				memset(cipherBuffer, '\0', sizeof(plainTextBuffer));
				// Iterate through plaintext
				for (i=0;i<break1-1;i++) {
					// Spaces will break things
					if (keyBuffer[i] == ' ')
						keyBuffer[i] = '@';
					if (plainTextBuffer[i] == ' ')
						plainTextBuffer[i] = '@';
					// Convert chars to ints
					plainTextInt = (int) plainTextBuffer[i];
					keyInt = (int) keyBuffer[i];
					// Map to 0-26
					plainTextInt-=64;
					keyInt-=64;
					// Cipher formula
					cipherInt = ((keyInt + plainTextInt) % 27);
					// Map to A-Z
					cipherInt+=64;
					// Convert cipher value to char
					temp = (char) cipherInt;
					if (temp == '@')
						temp = ' ';
					// Store cipher char in buffer
					cipherBuffer[i] = temp;
				}
				// Add newline to end
				cipherBuffer[i-1] = '\n';
				// Transmit ciphertext
				charsSent = send(establishedConnectionFD, cipherBuffer, sizeof(cipherBuffer), 0);
				if (charsRead < 0) error("ERROR writing to socket");
				// Process is finished
				close(establishedConnectionFD);
				exit(0);
			}
			// Parent process resumes
			else {
				// nothing needs to go here
			}
		}
		// Server's full
		else
			printf("SERVER: sorry no more sockets avaliable....\n");
	}
	// it doesn't actually get here
	return 0;
}

void checkDeadChildren() {
	int i=0;
	int pid;
	int check;
	//Check for complete connections
	for (i=0;i<5;i++) {
		pid = connectionPID[i];
		pid_t chk = waitpid(pid,&check,WNOHANG);
		// Remove process if it's done
		if ((chk == connectionPID[i]) || (chk == 2) || (chk == 1)) {
			connectionPID[i] = -2;
			numConnections--;
		}
	}
}

int findNextAvail(int* array, int n, int def) {
	int i;
	int done = 0;
	// Iterate through looking for first default char
	for (i=0;i<n;i++) {
		if (array[i] == def)
			done = 1;
		if (done)
			return(i);
	}
	if (!done)
		return(-1);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                             