#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

//max size of key
#define MAX_KEY_SIZE 70003

char* keygen(int num_letters) {
    char buffer[MAX_KEY_SIZE];
    int rando;

    //create key
    for (int i = 0; i < num_letters; i++) {

        //generate random number
        rando = rand() % 27;

        //space case
        if (rando == 26) {
            buffer[i] = ' ';
        }
        //otherwise, add to A and append to buffer
        else {
            buffer[i] = 'A' + rando;
        }
    }
    //add the newline
    buffer[num_letters] = '\n';

    char* key = malloc(strlen(buffer) + 1);
    strncpy(key, buffer, strlen(buffer) + 1);

    return key;
}

int main(int argc, char* argv[]) {

    int num_letters;

    //make sure a number was entered
    if (argc < 2)
    {
        printf("You must provide the number of letters for the key as the second arg\n");
        return EXIT_FAILURE;
    }
    else {
        num_letters = strtol(argv[1], NULL, 10); ;
    }

    //Seed Srand
    srand(time(0));
    
    //call keygen function
    char* key = keygen(num_letters);
    printf("%s", key);
    return EXIT_SUCCESS;
}