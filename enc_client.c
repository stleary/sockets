#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
 
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
 

// Error function used for reporting issues
void error(const char *msg) {
 perror(msg);
 exit(0);
}
 
 
// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
                       int portNumber,
                       char* hostname){
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));
  
  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  
  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname);
  if (hostInfo == NULL) {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(1);
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}


// return 1 on sucess otherwise 0
// updates len
// don't forget to free bufferptr
int readFile(char *filename, char **bufferptr, int *len) {
	
  /* declare a file pointer */
  FILE    *infile;
  char    *buffer;
  long    numbytes;
  
  /* open an existing file for reading */
  infile = fopen(filename, "r");
  
  /* quit if the file does not exist */
  if(infile == NULL) {
      fprintf(stderr, "%s does not exist\n", filename);
      return 0;
  }
  
  /* Get the number of bytes */
  fseek(infile, 0L, SEEK_END);
  *len = ftell(infile);
  
  /* reset the file position indicator to 
  the beginning of the file */
  fseek(infile, 0L, SEEK_SET);	
  
  /* grab sufficient memory for the 
  buffer to hold the text */
  *bufferptr = (char*)calloc(*len, sizeof(char));	
  
  /* memory error */
  if(*bufferptr == NULL) {
      fprintf(stderr, "%s could not allocate memory\n", filename);
      return 0;
  }
  
  /* copy all the text into the buffer */
  fread(*bufferptr, sizeof(char), *len, infile);
  fclose(infile);
  return (1);
}
 
int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[256];

  // Check args
  if (argc != 4) {
    fprintf(stderr,"USAGE: enc_client plaintextFile keyFile serverPort\n");
    exit(1);
  }
  
  char *plaintextFile = argv[1];
  char *plaintextBuf = 0;
  int plaintextLen = 0;
  int result = readFile(plaintextFile, &plaintextBuf, &plaintextLen);
  if (result == 0) {
    exit(1);
  }
  // printf("len: %d\n", plaintextLen);
  // printf("file: %s\n", plaintextBuf);
  free(plaintextBuf);

  char *keyFile = argv[2];
  char *keyBuf = 0;
  int keyLen = 0;
  result = readFile(keyFile, &keyBuf, &keyLen);
  if (result == 0) {
    exit(1);
  }
  printf("len: %d\n", keyLen);
  printf("file: %s\n", keyBuf);
  free(keyBuf);

  int port = atoi(argv[3]);
  if (port == 0) {
    fprintf(stderr, "Invalid port: %s\n", argv[3]);
    exit(1);
  }


  return(0);

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }
  
    // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[2]), argv[1]);
  
  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }
  // Get input message from user
  printf("CLIENT: Enter text to send to the server, and then hit enter: ");
  // Clear out the buffer array
  memset(buffer, '\0', sizeof(buffer));
  // Get input from the user, trunc to buffer - 1 chars, leaving \0
  fgets(buffer, sizeof(buffer) - 1, stdin);
  // Remove the trailing \n that fgets adds
  buffer[strcspn(buffer, "\n")] = '\0';
  
  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(buffer)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }
  
  // Get return message from server
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
  
  // Close the socket
  close(socketFD);
  return 0;
}

