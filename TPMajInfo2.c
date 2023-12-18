#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Constants
#define TFTP_PORT "1069"
#define MAX_MESSAGE_LENGTH 100
#define MAX_DATA_SIZE 512
#define RRQ_OPCODE 1
#define DATA_OPCODE 3
#define ACK_OPCODE 4
#define ERROR_OPCODE 5
#define USAGE_MESSAGE "Usage: %s server file\nCommand can only be gettftp or puttftp\n"
#define UNKNOWN_COMMAND_MESSAGE "Unknown command\n"
#define DOWNLOAD_MESSAGE "Downloading file '%s' from server '%s'\n"
#define UPLOAD_MESSAGE "Uploading file '%s' to server '%s'\n"
#define MODE "octet"

// Function to display a message
void display(const char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

// Function to construct a RRQ packet
int constructRRQ(char *buffer, const char *filename) {
    char *p = buffer;
    *(short *)p = htons(RRQ_OPCODE);
    p += 2;
    strcpy(p, filename);
    p += strlen(filename) + 1;
    strcpy(p, MODE);
    p += strlen(MODE) + 1;
    *p = '\0';
    return p - buffer + 1;
}

// Function to handle downloading a file from the server
void downloadFile(const char *server, const char *file) {
    // Get the address information for the server
    struct addrinfo hints, *serverInfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server, TFTP_PORT, &hints, &serverInfo) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // Create a socket
    int sockfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Construct RRQ packet
    char buffer[MAX_DATA_SIZE];
    int packetSize = constructRRQ(buffer, file);

    // Use sockfd to send the RRQ packet to the server...
    if (sendto(sockfd, buffer, packetSize, 0, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    // Open the file for writing in binary mode
    FILE *filePtr = fopen(file, "wb");
    if (filePtr == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // Inside the loop where you process data packets
    int blockNumber = 1;
    while (1) {
        // Receive the data packet
        socklen_t server_len = sizeof(struct sockaddr);
        int receivedBytes = recvfrom(sockfd, buffer, MAX_DATA_SIZE + 4, 0, serverInfo->ai_addr, &server_len);
        if (receivedBytes == -1) {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }

        short opcode = ntohs(*(short *)buffer);

        if (opcode == ERROR_OPCODE) {
            fprintf(stderr, "Received error packet with code %d: %s\n", ntohs(*(short *)(buffer + 2)), buffer + 4);
            exit(EXIT_FAILURE);
        } else if (opcode == DATA_OPCODE) {
            // Write data to the file
            fwrite(buffer + 4, 1, receivedBytes - 4, filePtr);

            // Send ACK
            *(short *)buffer = htons(ACK_OPCODE);
            *(short *)(buffer + 2) = htons(blockNumber);
            if (sendto(sockfd, buffer, 4, 0, serverInfo->ai_addr, server_len) == -1) {
                perror("sendto");
                exit(EXIT_FAILURE);
            }
            blockNumber++;
        }

        // Check if it's the last data packet
        if (receivedBytes < MAX_DATA_SIZE + 4) {
            break;
        }
    }

    // Close the file
    fclose(filePtr);

    freeaddrinfo(serverInfo);

    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), DOWNLOAD_MESSAGE, file, server);
    display(message);

    // Close the socket
    close(sockfd);
}

// Function to handle uploading a file to the server
void uploadFile(const char *server, const char *file) {
    // Get the address information for the server
    struct addrinfo hints, *serverInfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server, TFTP_PORT, &hints, &serverInfo) != 0) {
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
    snprintf(message, sizeof(message), UPLOAD_MESSAGE, file, server);
    display(message);
}

int main(int argc, char *argv[]) {
    // Check the number of command-line arguments
    if (argc != 3) {
        fprintf(stderr, USAGE_MESSAGE, argv[0]);
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
