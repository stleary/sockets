#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {

    int i, n;
    time_t t;

    /* Intializes random number generator */
    srand((unsigned) time(&t));

    if (argc < 2) {
        fprintf(stderr, "Usage: keygen n, where n is length of key to generate greater than 0\n");
        exit(2);
    }
    n = atoi(argv[1]);
    if (n == 0) {
        fprintf(stderr, "key length must be a number > 0\n");
        exit(2);
    }

    /* Print one time pad */
    for( i = 0 ; i < n ; i++ ) {
        int r =  (rand() % 27);
        if (r == 26) {
            printf(" ");
        } else {
            printf("%c", 'A' + r);
        }
    }
    printf("\n");

    return(0);
}
