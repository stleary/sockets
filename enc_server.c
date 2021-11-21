#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/wait.h> 

#define ENCRYPT 1
#define DECRYPT 0

#define DIRECTION ENCRYPT
 
// Error function used for reporting issues
void error(const char *msg) {
 perror(msg);
 exit(1);
}
 
// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
                       int portNumber){
 // Clear out the address struct
 memset((char*) address, '\0', sizeof(*address));
 
 // The address should be network capable
 address->sin_family = AF_INET;
 // Store the port number
 address->sin_port = htons(portNumber);
 // Allow a client at any address to connect to this server
 address->sin_addr.s_addr = INADDR_ANY;
}

// read a file over the socket.
// return 1 on success, otherwise 0
int readFileSocket(int connectionSocket, char **bufferPtr, int *fileLen) {

  char buffer[256];
  // Get the file len
  memset(buffer, '\0', 256);
  // Read the client's message from the socket
  int charsRead = recv(connectionSocket, buffer, 5, 0);
  if (charsRead < 0){
    fprintf(stderr, "ERROR reading from socket");
    return (0);
  }
  *fileLen = atoi(buffer);
  if (*fileLen == 0) {
    fprintf(stderr, "invalid length: %s\n", buffer);
    return (0);
  }
  printf("SERVER: I received this from the client: \"%d\"\n", *fileLen);

  *bufferPtr = calloc(*fileLen, sizeof(char));
  char *ptr = *bufferPtr;
  int len = *fileLen;
  while (1) {
    if (len <= 256) {
      charsRead = recv(connectionSocket, ptr, len, 0);
      if (charsRead < 0){
        error("ERROR reading from socket");
      }       
      break;
    } else {
      charsRead = recv(connectionSocket, ptr, 256, 0);
      if (charsRead < 0){
        error("ERROR reading from socket");
      }       
      ptr += 256;
      len -= 256;
    }      
  }
  return (1);
}



// writes a file 256 bytes at a time
// return 1 if successful,  otherwise 0
int sendFileSocket(int socketFD, char *fileBuf, int fileLen) {


  // send len of file
  char buffer[256];
  sprintf(buffer, "%5d", fileLen);
  int charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  if (charsWritten < 0){
    fprintf(stderr, "CLIENT: ERROR writing to socket");
    return (0);
  }
  if (charsWritten < strlen(buffer)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  // send file
  int len = fileLen;
  char *ptr = fileBuf;
  while (1) {
    if (len <= 256) {
      memcpy(buffer, ptr, len);
      charsWritten = send(socketFD, buffer, len, 0);
      if (charsWritten < 0){
        fprintf(stderr, "CLIENT: ERROR writing to socket");
        return (0);
      }
      if (charsWritten < len){
        printf("CLIENT: WARNING: Not all data written to socket!\n");
      }
      break;
    } else {
      memcpy(buffer, ptr, 256);
      ptr += 256;
      len -= 256;
      charsWritten = send(socketFD, buffer, 256, 0);
      if (charsWritten < 0){
        fprintf(stderr, "CLIENT: ERROR writing to socket");
        return (0);
      }
      if (charsWritten < 256){
        printf("CLIENT: WARNING: Not all data written to socket!\n");
      }
    }
  }
  return(1);
}

// encyphers or decpyhers text
void cypher(char *textPtr, char *keyPtr, char*cypherPtr, int len, int direction) {

  for (int i = 0; i < len; ++i) {
    char c = 0;
    char cText = textPtr[i];
    if (cText == ' ') {
      cText = 26;
    } else {
      cText -= 'A';
    }
    char cCypher = keyPtr[i];
    if (cCypher == ' ') {
      cCypher = 26;
    } else {
      cCypher -= 'A';
    }

    if (direction == ENCRYPT) {
      c = (cText + cCypher) % 27;
    } else {
      // DECRYPT
      c = (cText - cCypher);
      if (c < 0) {
        c += 27;
      }
    }

    if (c == 26) {
      c = ' ';
    } else {
      c += 'A';
    }
    cypherPtr[i] = c;
  }
}


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
 setupAddressStruct(&serverAddress, atoi(argv[1]));
 
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

    printf("SERVER: Connected to client running at host %d port %d\n",
                          ntohs(clientAddress.sin_addr.s_addr),
                          ntohs(clientAddress.sin_port));
  
    // client name
    memset(buffer, '\0', 256);
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, 3, 0);
    if (charsRead < 0){
      error("ERROR reading from socket");
    }
    if (strcmp(buffer, "enc")) {
      fprintf(stderr, "Not a client, hanging up: %s\n", buffer);
      exit(0);
    }
    printf("SERVER: I received this from the client: \"%s\"\n", buffer);

    char *textFilePtr = 0;
    int textLen = 0;
    int result = readFileSocket(connectionSocket, &textFilePtr, &textLen);
    if (result == 0) {
      fprintf(stderr, "unable to read input file\n");
      exit(0);
    }
    printf("textFile: %s\n", textFilePtr);

    char *keyFilePtr = 0;
    int keyLen = 0;
    result = readFileSocket(connectionSocket, &keyFilePtr, &keyLen);
    if (result == 0) {
      fprintf(stderr, "unable to read input file\n");
      exit(0);
    }
    printf("keyFile: %s\n", keyFilePtr);


    // todo still need to perform a cypher operation
    char *cypherTextPtr = calloc(textLen, sizeof(char));
    cypher(textFilePtr, keyFilePtr, cypherTextPtr, textLen, DIRECTION);

    result = sendFileSocket(connectionSocket, cypherTextPtr, textLen);
    if (result == 0) {
      error("Unable to send cypher file");
    }

    // Close the connection socket for this client
    close(connectionSocket);
    printf("terminating child\n");
    exit(0);
  }
 }
 // Close the listening socket
 close(listenSocket);
 return 0;
}
 

