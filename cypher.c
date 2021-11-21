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

  char *plain = "THE QUICK BROWN FOX";
  char *key =   "OOLAEHJCQIAXMIHOKZV";
  char *cbuf = calloc(256, sizeof(char));
  char *dbuf = calloc(256, sizeof(char));

  cypher(plain, key, cbuf, strlen(plain), ENCRYPT);
  printf("  plain: %s\n", plain);
  printf("    key: %s\n", key);
  printf("encrypt: %s\n", cbuf);
  cypher(cbuf, key, dbuf, strlen(plain), DECRYPT);
  printf("restore: %s\n", dbuf);




}
 

