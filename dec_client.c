#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <netdb.h>      // gethostbyname()
#include "utils.h" 

#define DIRECTION DECRYPT

/**
* encrypting Client code
* usage: enc_client plaintextFile keyFile encServerPort
* read and validate plaintext and key files
* Create a socket and connect to the server port specified in the command arugments.
* send enc identifer
* send plaintext length
* send plaintext
* send key length
* send key
* receive and print ciphertext
* terminate connection
* exit 1 on input error, 2 on wrong server, otherwise 0.
*/
 
char *plaintextBuf = 0;
char *keyBuf = 0;


int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[256];

  // Check args
  if (argc != 4) {
    if (DIRECTION == ENCRYPT) {
      fprintf(stderr,"USAGE: enc_client plaintextFile keyFile serverPort\n");
    } else {
      fprintf(stderr,"USAGE: dec_client encryptedFile keyFile serverPort\n");
    }
    exit(1);
  }

 
  int port = atoi(argv[3]);
  if (port == 0) {
    fprintf(stderr, "Invalid port: %s\n", argv[3]);
    exit(1);
  }

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }
  
    // Set up the server address struct
  setupAddressStructClient(&serverAddress, port, "localhost");


  char *plaintextFile = argv[1];
  int plaintextLen = 0;
  int result = readFile(plaintextFile, &plaintextBuf, &plaintextLen);
  if (result == 0) {
    exit(1);
  }
 

  char *keyFile = argv[2];
  int keyLen = 0;
  result = readFile(keyFile, &keyBuf, &keyLen);
  if (result == 0) {
    exit(1);
  }

  if (keyLen < plaintextLen) {
    error("Key is too short for text\n");
  }

  
  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  // identify self to server
  if (DIRECTION == ENCRYPT) {
    strcpy(buffer, "enc");
  } else {
    strcpy(buffer, "enc");
  }
  if (sendWithRetries(socketFD, buffer) == 0) {
    error("Failed to send encrypt/decrypt ID string");
  }
  
  result = sendFileSocket(socketFD, plaintextBuf, plaintextLen);
  if (result == 0) {
    error("Unable to send plaintext file");
  }

  result = sendFileSocket(socketFD, keyBuf, keyLen);
  if (result == 0) {
    error("Unable to send key file");
  }

  
  char *responseFilePtr = 0;
  int responseTextLen = 0;
  result = readFileSocket(socketFD, &responseFilePtr, &responseTextLen);
  if (result == 0) {
    fprintf(stderr, "unable to read input file\n");
    exit(0);
  }
  printf("%s\n", responseFilePtr);
  free(responseFilePtr);
  
  // Close the socket
  close(socketFD);

  free(plaintextBuf);
  free(keyBuf);

  return 0;
}

