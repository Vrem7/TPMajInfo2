#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>  // for getaddrinfo
#include <sys/socket.h>

// Constants
#define MAX_MESSAGE_LENGTH 100
#define USAGE_MESSAGE "Usage: ./command server file\ncommand can only be gettftp or puttftp\n"
#define UNKNOWN_COMMAND_MESSAGE "Unknown command\n"
#define DOWNLOAD_MESSAGE "Downloading file '%s' from server '%s'\n"
#define UPLOAD_MESSAGE "Uploading file '%s' to server '%s'\n"

// Function to display a message
void display(char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

// Function to handle downloading a file from the server
void downloadFile(const char *server, const char *file) {
    // Get the address information for the server
    struct addrinfo hints, *serverInfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server, "tftp", &hints, &serverInfo) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // Create a socket
    int sockfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Use sockfd to communicate with the server...

    freeaddrinfo(serverInfo);

    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), DOWNLOAD_MESSAGE, file, server);
    display(message);
}

// Function to handle uploading a file to the server
void uploadFile(const char *server, const char *file) {
    // Get the address information for the server
    struct addrinfo hints, *serverInfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server, "tftp", &hints, &serverInfo) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    //Reserved socket
    int sockfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Use sockfd to communicate with the server...

    freeaddrinfo(serverInfo);

    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), UPLOAD_MESSAGE, file, server);
    display(message);
}

int main(int argc, char *argv[]) {
    // Check the number of command-line arguments
    if (argc != 3) {
        display(USAGE_MESSAGE);
        exit(EXIT_FAILURE);
    }

    const char *command = argv[0];
    const char *server = argv[1];
    const char *file = argv[2];

    if (strncmp(command, "./gettftp", MAX_MESSAGE_LENGTH) == 0) {
        downloadFile(server, file);
    } else if (strncmp(command, "./puttftp", MAX_MESSAGE_LENGTH) == 0) {
        uploadFile(server, file);
    } else {
        display(UNKNOWN_COMMAND_MESSAGE);
        exit(EXIT_FAILURE);
    }

    return 0;
}
