#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/wait.h> 
#include "utils.h"

#define DIRECTION DECRYPT


int main(int argc, char *argv[]){
 int connectionSocket, charsRead;
 char buffer[256];
 struct sockaddr_in serverAddress, clientAddress;
 socklen_t sizeOfClientInfo = sizeof(clientAddress);
 
 // Check usage & args
 if (argc < 2) {
   fprintf(stderr,"USAGE: %s port\n", argv[0]);
   exit(1);
 }
  // Create the socket that will listen for connections
 int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
 if (listenSocket < 0) {
   error("ERROR opening socket");
 }
 
 // Set up the address struct for the server socket
 setupAddressStructServer(&serverAddress, atoi(argv[1]));
 
 // Associate the socket to the port
 if (bind(listenSocket,
         (struct sockaddr *)&serverAddress,
         sizeof(serverAddress)) < 0){
   error("ERROR on binding");
 }
 
 // Start listening for connetions. Allow up to 5 connections to queue up
 listen(listenSocket, 5);
  // Accept a connection, blocking if one is not available until one connects
 while(1){
   // Accept the connection request which creates a connection socket
   connectionSocket = accept(listenSocket,
               (struct sockaddr *)&clientAddress,
               &sizeOfClientInfo);
   if (connectionSocket < 0){
     error("ERROR on accept");
     continue;
   }
 
  pid_t parent = getpid();
  pid_t pid = fork();

  if (pid == -1)
  {
      // error, failed to fork()
      error("failed to fork");
  } 
  else if (pid > 0)
  {
      // int status;
      // waitpid(pid, &status, 0);
  }
  else 
  {

    // printf("SERVER: Connected to client running at host %d port %d\n",
    //                       ntohs(clientAddress.sin_addr.s_addr),
    //                       ntohs(clientAddress.sin_port));
  
    // client name
    memset(buffer, '\0', 256);
    // Read the client's message from the socket
    if (readWithRetries(connectionSocket, buffer, 3) == 0) {
      error("ERROR reading from socket");
    }
    if (strcmp(buffer, "enc")) {
      fprintf(stderr, "Not a client, hanging up: %s\n", buffer);
      exit(0);
    }
    // printf("SERVER: I received this from the client: \"%s\"\n", buffer);

    char *textFilePtr = 0;
    int textLen = 0;
    int result = readFileSocket(connectionSocket, &textFilePtr, &textLen);
    if (result == 0) {
      fprintf(stderr, "unable to read input file\n");
      exit(0);
    }
    // printf("textFile: %s\n", textFilePtr);

    char *keyFilePtr = 0;
    int keyLen = 0;
    result = readFileSocket(connectionSocket, &keyFilePtr, &keyLen);
    if (result == 0) {
      fprintf(stderr, "unable to read input file\n");
      exit(0);
    }
    // printf("keyFile: %s\n", keyFilePtr);


    // todo still need to perform a cypher operation
    char *cypherTextPtr = calloc(textLen, sizeof(char));
    cypher(textFilePtr, keyFilePtr, cypherTextPtr, textLen, DIRECTION);

    result = sendFileSocket(connectionSocket, cypherTextPtr, textLen);
    if (result == 0) {
      error("Unable to send cypher file");
    }

    // Close the connection socket for this client
    close(connectionSocket);
    // printf("terminating child\n");
    exit(0);
  }
 }
 // Close the listening socket
 close(listenSocket);
 return 0;
}
 

