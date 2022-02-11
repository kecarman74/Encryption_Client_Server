#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define MAX_MSG_SIZE 90000

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(0);
}

//function to remove new line cahracters
char* remove_newline(char* string) {
    char buffer[MAX_MSG_SIZE];

    int i = 0;
    //scan string until null terminator found
    while (string[i] != '\0') {
        //if a new line character is found replace with a space in the new string
        if (string[i] == '\n') {

        }
        //otherwise write the char into the new string
        else {
            buffer[i] = string[i];
        }
        i++;
    }
    //add null terminator and copy to heap
    buffer[i] = '\0';
    char* parsed_str = malloc(strlen(buffer) + 1);
    strncpy(parsed_str, buffer, strlen(buffer) + 1);

    return parsed_str;
}

//function to get user input
char* get_input(char* filePath) {
    char buffer[MAX_MSG_SIZE];

    // Open the specified file for reading only
    FILE* msg_file = fopen(filePath, "r");

    //transfer input to string
    fgets(buffer, MAX_MSG_SIZE, msg_file);

    //close file
    fclose(msg_file);

    //return buffer as variable in heap
    char* input_str = malloc(strlen(buffer) + 1);
    strncpy(input_str, buffer, strlen(buffer) + 1);

    return input_str;
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname("localhost");
    if (hostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*)&address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char* argv[]) {
    int socketFD, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char *msg;
    char* key;
    char rec_msg[MAX_MSG_SIZE];
    char buffer[MAX_MSG_SIZE];

    // Check usage & args
    if (argc < 3) {
        fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
        exit(0);
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]));

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }
    // Get input message from user
    msg = get_input(argv[1]);

    // Get key file from user
    key = get_input(argv[2]);

    char identity[] = "enc_client\n";
    strcat(buffer, identity);
    strcat(buffer, msg);
    strcat(buffer, key);
    
    int msg_len_int = strlen(buffer);
    char msg_len_str[20];
    sprintf(msg_len_str, "%d", msg_len_int);

    // Send msg length to server
    charsWritten = send(socketFD, msg_len_str, strlen(msg_len_str), 0);
    if (charsWritten < 0) {
        error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(msg_len_str)) {
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }
    sleep(1);
    // Send buffer to server
    // Write to the server
    int chars_sent = 0;
    while (chars_sent < msg_len_int) {
        charsWritten = send(socketFD, buffer, strlen(buffer), 0);
        if (charsWritten < 0) {
            error("CLIENT: ERROR writing to socket");
        }
        if (charsWritten < strlen(buffer)) {
            printf("CLIENT: WARNING: Not all data written to socket!\n");
        }
        chars_sent += charsWritten;
    }

    // Get return message from server
    // Clear out the buffer again for reuse
    //memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end
    int enc_msg_len = strlen(msg) - 1;
    int chars_recd = 0;
    memset(buffer, '\0', MAX_MSG_SIZE);
    while (chars_recd < enc_msg_len) {
        charsRead = recv(socketFD, rec_msg, strlen(rec_msg) - 1, 0);
        if (charsRead < 0) {
            error("CLIENT: ERROR reading from socket");
        }
        
        strcat(buffer, rec_msg);
        memset(rec_msg, '\0', MAX_MSG_SIZE);
        chars_recd += charsRead;
        if (strncmp(buffer, "Error: not correct client", 25) == 0) {
            break;
        }
        else if (strncmp(buffer, "Error: key is too short", 23) == 0) {
            break;
        }
    }
    //remove newlines
    char* parsed_str = remove_newline(buffer);
    //print encrypted message
    printf("%s\n", parsed_str);

    // Close the socket
    close(socketFD);
    return 0;
}