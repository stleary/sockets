

void error(const char *msg);
void setupAddressStructClient(struct sockaddr_in* address,
                       int portNumber,
                       char* hostname);
void setupAddressStructServer(struct sockaddr_in* address,
                       int portNumber);
int readFile(char *filename, char **bufferptr, int *len);
int readFileSocket(int connectionSocket, char **bufferPtr, int *fileLen);
int sendFileSocket(int socketFD, char *fileBuf, int fileLen);
void cypher(char *textPtr, char *keyPtr, char*cypherPtr, int len, int direction);
int sendWithRetries(int socketFD, char *buffer);
int readWithRetries(int socketFD, char *buffer, int len);

#define ENCRYPT 1
#define DECRYPT 0
