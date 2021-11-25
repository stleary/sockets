#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include "utils.h"



// Error function used for reporting issues and exit 1
void error(const char *msg) {
 perror(msg);
 exit(1);
}
 
// Set up the address struct
void setupAddressStructClient(struct sockaddr_in* address,
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

// Set up the address struct for the server socket
void setupAddressStructServer(struct sockaddr_in* address,
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

  // strip newline, if found
  if ((*bufferptr)[*len - 1] == '\n') {
    (*bufferptr)[*len - 1] = 0;
    *len = *len - 1;
  } else {
    fprintf(stderr, "Expected %s to end with a newline\n", filename);
    return 0;
  }

  // validate input

  for (int i = 0; i < *len; ++i) {
    if ((*bufferptr)[i] != ' ' && ((*bufferptr)[i] < 'A' || (*bufferptr)[i] > 'Z')) {
      fprintf(stderr, "Invalid char found in %s: %c\n", filename, (*bufferptr)[i]);
      return 0;
    }
  }
  return (1);
}



// read a file over the socket.
// return 1 on success, otherwise 0
int readFileSocket(int connectionSocket, char **bufferPtr, int *fileLen) {

  char buffer[257];
  // Get the file len
  memset(buffer, '\0', 257);
  // Read the client's message from the socket
  if (readWithRetries(connectionSocket, buffer, 5) == 0) {
    fprintf(stderr, "ERROR reading from socket");
    return (0);
  }
  *fileLen = atoi(buffer);
  if (*fileLen == 0) {
    fprintf(stderr, "invalid length: %s\n", buffer);
    return (0);
  }
  // printf("SERVER: I received this from the client: \"%d\"\n", *fileLen);

  *bufferPtr = calloc(*fileLen, sizeof(char));
  char *ptr = *bufferPtr;
  int len = *fileLen;
  while (1) {
    if (len <= 256) {
      if (readWithRetries(connectionSocket, ptr, len) == 0) {
        fprintf(stderr, "ERROR reading from socket");
        return 0;
      }       
      break;
    } else {
      if (readWithRetries(connectionSocket, ptr, 256) == 0) {
        fprintf(stderr, "ERROR reading from socket");
      }       
      ptr += 256;
      len -= 256;
      if (len == 0) {
        break;
      }
    }      
  }
  return (1);
}



// writes a file 256 bytes at a time
// return 1 if successful,  otherwise 0
int sendFileSocket(int socketFD, char *fileBuf, int fileLen) {


  // send len of file
  char buffer[257];
  memset(buffer, 0, 257);
  sprintf(buffer, "%5d", fileLen);
  if (sendWithRetries(socketFD, buffer) == 0) {
    fprintf(stderr, "Error writing filesize\n");
    return 0;
  }

  // send file
  int len = fileLen;
  char *ptr = fileBuf;
  while (1) {
    if (len <= 256) {
      memset(buffer, 0, 256);
      memcpy(buffer, ptr, len);
      if (sendWithRetries(socketFD, buffer) == 0) {
        fprintf(stderr, "CLIENT: ERROR writing to socket");
        return (0);
      }
      break;
    } else {
      memset(buffer, 0, 256);
      memcpy(buffer, ptr, 256);
      if (sendWithRetries(socketFD, buffer) == 0) {
        fprintf(stderr, "CLIENT: ERROR writing to socket");
        return (0);
      }
      ptr += 256;
      len -= 256;
      if (len == 0) {
        break;
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

// returns bytes read or 0 for failure
int readWithRetries(int socketFD, char *buffer, int len) {
  int tries = 0;
  int maxTries = 3;
  int charsToRead = len;
  int nextOffset = 0;

  while (tries < maxTries) {
    int charsRead = recv(socketFD, &buffer[nextOffset], charsToRead, 0);
    if (charsRead < 0){
      // nothing worked
      ++tries;
    } else if (charsRead < charsToRead){
      // partial write, advance the indices
      charsToRead -= charsRead;
      nextOffset += charsRead;
      ++tries;
    } else {
      // everything was written
      return len;
    }
  }
  // must have exhausted our tries
  return 0;
}


// returns bytes written or 0 for failure
int sendWithRetries(int socketFD, char *buffer) {
  int tries = 0;
  int maxTries = 3;
  int charsToWrite = strlen(buffer);
  int nextOffset = 0;

  while (tries < maxTries) {
    int charsWritten = send(socketFD, &buffer[nextOffset], charsToWrite, 0);
    if (charsWritten < 0){
      // nothing worked
      ++tries;
    } else if (charsWritten < charsToWrite){
      // partial write, advance the indices
      charsToWrite -= charsWritten;
      nextOffset += charsWritten;
      ++tries;
    } else {
      // everything was written
      return strlen(buffer);
    }
  }
  // must have exhausted our tries
  return 0;
}
