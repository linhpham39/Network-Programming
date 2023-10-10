#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 1024


/* Client 1 send message first, then turn to client 2 and vice versa */
int main(int argc, char *argv[])
{
    // Check command line arguments
    if (argc != 3)
    {
        printf("Usage: %s IPAddress PortNumber\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd, len;
    struct sockaddr_in servaddr;
    char buffer[1024];
    memset(&servaddr, 0, sizeof(servaddr));

    // Create socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error. Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address and port number
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2])); // Convert port number to network byte order
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) // Convert IP address from text to binary
    {
        printf("Invalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("Message input: ");
        fgets(buffer, sizeof(buffer), stdin); // Read input from user
        // Remove trailing newline character
        buffer[strcspn(buffer, "\n")] = 0;
        // Send message to server
        sendto(sockfd, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        if (strlen(buffer) == 0) // If user entered empty string, break out of loop
        {
            break;
        }
        len = sizeof(servaddr);
        // Receive result from server and display it
        int n = recvfrom(sockfd, (char *)buffer, BUF_SIZE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
        buffer[n] = '\0';
        int cnt = 0;
        if (strlen(buffer) == 0) // If server response is empty, increment counter
        {
            cnt++;
        }
        printf("Reply from server: \n%s\n", buffer);
        n = recvfrom(sockfd, (char *)buffer, BUF_SIZE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
        buffer[n] = '\0';
        if (strlen(buffer) == 0) // If server response is empty, increment counter
        {
            cnt++;
        }
        printf("%s\n", buffer);
        if (cnt == 2) // If both server responses were empty, break out of loop
        {
            break;
        }
        else
        {
            cnt == 0; // Reset counter
        }
    }

    // Close the socket
    close(sockfd);
    return 0;
}
