#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_MSG_SIZE 180000

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

//function to decrypt message
char* dec_msg(char* key, char* msg) {
    char buffer[90000];
    int i;
    int key_int;
    int msg_int;
    int dec_char;

    for (i = 0; i < strlen(msg); i++) {

        //get the ascii value from the message char
        //convert it to 0-26
        key_int = (int)key[i] - 65;
        msg_int = (int)msg[i] - 65;

        //if we had a space, assign it 27
        if (key_int == -33) {
            key_int = 26;
        }
        else if ((key_int < 0) || (key_int > 25)) {
            printf("bad char\n");
            exit(1);
        }
        if (msg_int == -33) {
            msg_int = 26;
        }
        else if ((msg_int < 0) || (msg_int > 25)) {
            printf("bad char\n");
            exit(1);
        }
        //calc the encryption int
        dec_char = msg_int - key_int;
        if (dec_char < 0) {
            dec_char += 27;
        }

        //write the encrypted message to buffer
        //space case
        if (dec_char == 26) {
            buffer[i] = ' ';
        }
        //otherwise, add to A and append to buffer
        else {
            buffer[i] = 'A' + dec_char;
        }
    }

    char* enc_msg = malloc(strlen(buffer) + 1);
    strncpy(enc_msg, buffer, strlen(buffer) + 1);

    return enc_msg;
}

int main(int argc, char* argv[]) {
    int connectionSocket, charsRead;
    char msg_buffer[MAX_MSG_SIZE];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    // Check usage & args
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
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
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }

    // Start listening for connetions. Allow up to 10 connections to queue up
    listen(listenSocket, 10);

    // Accept a connection, blocking if one is not available until one connects
    while (1) {
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket,
            (struct sockaddr*)&clientAddress,
            &sizeOfClientInfo);
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }
        //create a new child for each request
        pid_t spawnPid = fork();
        switch (spawnPid) {
            case -1:
                perror("fork()\n");
                exit(EXIT_FAILURE);
                break;
            case 0:
                ;
                //child process
                //receive msg length
                close(listenSocket);
                char msg_len_str[20];
                int msg_len_int = 0;
                char buffer[MAX_MSG_SIZE];
                // Read the client's message from the socket
                charsRead = recv(connectionSocket, msg_len_str, 20, 0);
                if (charsRead < 0) {
                    error("ERROR reading from socket");
                }
                msg_len_int = atoi(msg_len_str);

                charsRead = 0;
                int chars_received = 0;

                memset(msg_buffer, '\0', MAX_MSG_SIZE);
                while (chars_received < msg_len_int) {
                    // Get the message from the client and display it
                    // Read the client's message from the socket
                    charsRead = recv(connectionSocket, msg_buffer, MAX_MSG_SIZE, 0);
                    if (charsRead < 0) {
                        error("ERROR reading from socket");
                    }
                    strcat(buffer, msg_buffer);
                    memset(msg_buffer, '\0', MAX_MSG_SIZE);
                    chars_received += charsRead;
                }

                //break apart id, key and message
                char* key;
                char* msg;
                char* id = strtok_r(buffer, "\n", &key);
                msg = strtok_r(NULL, "\n", &key);

                //check to see if it is the enc_client
                if (strcmp(id, "dec_client") != 0) {
                    send(connectionSocket, "Error: not correct client", 26, 0);
                    exit(2);
                    break;
                }
                //check if key is long enough
                else if (strlen(key) < strlen(msg)) {
                    send(connectionSocket, "Error: key is too short", 24, 0);
                    exit(2);
                }
                else {
                    //char* for encrypted message
                    char* decrypted_msg;

                    //call function to encrypt and assign to variable
                    decrypted_msg = dec_msg(key, msg);

                    int chars_sent = 0;
                    int enc_msg_len = strlen(decrypted_msg);

                    while (chars_sent < enc_msg_len) {
                        // Send encrypted message back to the client
                        charsRead = send(connectionSocket,
                            decrypted_msg, strlen(decrypted_msg), 0);
                        if (charsRead < 0) {
                            error("ERROR writing to socket");
                        }
                        chars_sent += charsRead;
                    }
                    exit(0);
                }
                
                break;
            default:
                ;
                //if this is the parent proceed with program and close connection socket and continue while loop
                close(connectionSocket);
        }
    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}
