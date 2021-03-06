/*********************************************************************
CS 344 Program 4
otp_dec_d.c, Decryption Server
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
int id = '0';

// Process variables
int numConnections = 0;
int checkConnections = 1;
int connectionPID [5] = {-2,-2,-2,-2,-2};
int establishedConnectionFD = -1;
// Storage
char plainTextBuffer [80000];
char keyBuffer [80000];
char cipherTextBuffer [80000];
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
					memset(plainTextBuffer, '\0', sizeof(cipherTextBuffer));
					plainTextBuffer[0] = '?';
					charsRead = send(establishedConnectionFD, plainTextBuffer, sizeof(plainTextBuffer), 0);
					if (charsRead < 0) error("ERROR writing to socket");
					exit(0);
				}
				// Parse data recieved
				int i;
				int break1;
				int break2;
				int cipherTextIndex = 0;
				int keyIndex = 0;
				// Find division between ciphertext and key
				for (i=0;i<sizeof(recBuffer);i++) {
					if (recBuffer[i] == '\n')
					break;
				}
				// Store ciphertext in its own buffer
				break1 = i;
				for (i=2;i<break1;i++) {
					cipherTextBuffer[cipherTextIndex]=recBuffer[i];
					cipherTextIndex++;
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
				// Do the decryption
				char temp;
				int keyInt;
				int cipherTextInt;
				int plainTextInt;
				memset(plainTextBuffer, '\0', sizeof(cipherTextBuffer));
				// Iterate through ciphertext
				for (i=0;i<break1-1;i++) {
					// Spaces will break things...
					if (cipherTextBuffer[i] == ' ')
						cipherTextBuffer[i] = '@';
					if (keyBuffer[i] == ' ')
						keyBuffer[i] = '@';
					// Convert chars to ints
					cipherTextInt = (int) cipherTextBuffer[i];
					keyInt = (int) keyBuffer[i];
					// Remove mapping from encryption
					cipherTextInt-=64;
					keyInt-=64;
					// Cipher formula
					plainTextInt = ((cipherTextInt - keyInt) % 27);
					if (plainTextInt < 0)
						plainTextInt+=27;
					// Map A-Z
					plainTextInt+=64;
					// Convert cipher value to char
					temp = (char) plainTextInt;
					if (temp == '@')
						temp = ' ';
					// Store plaintext char in buffer
					plainTextBuffer[i] = temp;
				}
				// Add newline to end
				plainTextBuffer[i-1] = '\n';
				// Transmit plainText
				charsSent = send(establishedConnectionFD, plainTextBuffer, sizeof(plainTextBuffer), 0);
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              