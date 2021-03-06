/*********************************************************************
CS 344 Program 4
otp_enc.c, Encryption Client
Taylor Griffin
03/15/18
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// SPECIFY encrypt (0), decrypt (1)
char id = '0';

// Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[]) {
	// Network things
	int socketFD, portNumber, charsWritten, charsRead, charsSent;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer [256];
	char connectionFailed [100];
	// Storage
	char plainTextBuffer [80000];
	char keyBuffer [80000];
	char cipherTextBuffer [80000];
	// i/o
	char transmitBuffer [160000];
	char recBuffer [80000];
	// Store plaintext in buffer
	ssize_t plainTextRead;
	int plainTextFD = open(argv[1], O_RDONLY);
	memset(plainTextBuffer,'\0',sizeof(plainTextBuffer));
	lseek(plainTextFD,0,SEEK_SET);
	plainTextRead = read(plainTextFD, plainTextBuffer, sizeof(plainTextBuffer));
	//Store key in buffer
	ssize_t keyRead;
	int keyFD = open(argv[2], O_RDONLY);
	memset(keyBuffer,'\0',sizeof(keyBuffer));
	lseek(keyFD,0,SEEK_SET);
	keyRead = read(keyFD, keyBuffer, sizeof(keyBuffer));
	// Check that they're the same length
	if (plainTextRead > keyRead) {
		char keyTooShort [100];
		sprintf(keyTooShort,"Error: key ‘%s’ is too short",argv[2]);
		fprintf(stderr,"%s\n",keyTooShort);
		exit(1);
	}
	// Check plaintext
	int i;
	int testChar;
	for (i=0;i<strlen(plainTextBuffer);i++) {
		testChar = (int) plainTextBuffer[i];
		// Check for bad characters
		if (((testChar < 65) && ((testChar != 32) && (testChar!=10))) || (testChar > 90)) {
			fprintf(stderr,"otp_enc error: input contains bad characters\n");
			exit(1);
		}
	}
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress));
	portNumber = atoi(argv[3]);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverHostInfo = gethostbyname("localhost");
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
		error("CLIENT: ERROR connecting");
	// Insert identifier
	transmitBuffer[0] = id;
	transmitBuffer[1] = '.';
	// Insert plaintext
	for (i=2;i<plainTextRead+2;i++) {
		transmitBuffer[i] = plainTextBuffer[i-2];
	}
	// Insert key
	for (i=plainTextRead+2;i<plainTextRead+2+keyRead;i++) {
		transmitBuffer[i] = keyBuffer[i-(plainTextRead+2)];
	}
	// Prepare to do i/o
	charsSent = 0;
	charsRead = 0;
	memset(recBuffer, '\0', 80000);
	// Send message, key, and identifier to server
	charsSent = send(socketFD, transmitBuffer, sizeof(transmitBuffer), 0);
	if (charsSent < 0) error("CLIENT: Error reading from socket");
	charsRead = recv(socketFD, recBuffer, plainTextRead, MSG_WAITALL);
	if (charsRead < 0) error("ERROR reading from socket");
	// Check if it's a BAD connnection
	if (recBuffer[0] == '?') {
		fprintf(stderr,"Error: could not contact otp_enc_d on port %d\n",portNumber);
		exit(2);
	}
	// print ciphertext
	printf("%s",recBuffer);
	// Close the socket
	close(socketFD);
	exit(0);
}
                                            