#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define BUF_SIZE 1024

/*
Splits a string into two parts: one containing all alphabetic characters,
 and the other containing all digits. If the input string contains any
non-alphanumeric characters, both output strings are set to "Error".
*/
void split_string(const char *input, char *alphabet, char *digit) {
    size_t i, j = 0, k = 0;
    for (i = 0; input[i] != '\0'; i++) {
        if (isalpha(input[i])) {
            alphabet[j++] = input[i];
        } else if (isdigit(input[i])) {
            digit[k++] = input[i];
        } else {
            strcpy(alphabet, "Error");
            strcpy(digit, "Error");
            alphabet[j] = '\0';
            digit[k] = '\0';
            return;
        }
    }
    alphabet[j] = '\0';
    digit[k] = '\0';
}


int main(int argc, char *argv[])
{
    int port = atoi(argv[1]);
    int sockfd, len;
    struct sockaddr_in servaddr, cliaddr1, cliaddr2;
    char buffer[BUF_SIZE];
    char alphabet[BUF_SIZE], digit[BUF_SIZE];
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr1, 0, sizeof(cliaddr1));
    memset(&cliaddr2, 0, sizeof(cliaddr2));

    // Create socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error. Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address and port number
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        len = sizeof(cliaddr1);
        // Receive message from first client
        int n = recvfrom(sockfd, (char *)buffer, BUF_SIZE, MSG_WAITALL, (struct sockaddr *)&cliaddr1, &len);
        buffer[n] = '\0';
        // Split string and send result to second client
        split_string(buffer, alphabet, digit);
        printf("Received message from client 1:\nAlphabet: %s\nDigit: %s\n", alphabet, digit);
        sendto(sockfd, (const char *)alphabet, strlen(alphabet), MSG_CONFIRM, (const struct sockaddr *)&cliaddr2, len);
        sendto(sockfd, (const char *)digit, strlen(digit), MSG_CONFIRM, (const struct sockaddr *)&cliaddr2, len);
        if (strlen(buffer) == 0)
        {
            printf("Received empty string, Exit.\n");
            break;
        }
        len = sizeof(cliaddr2);
        // Receive message from second client
        n = recvfrom(sockfd, (char *)buffer, BUF_SIZE, MSG_WAITALL, (struct sockaddr *)&cliaddr2, &len);
        buffer[n] = '\0';

        // Split string and send result to first client
        split_string(buffer, alphabet, digit);
        printf("Received message from client 2: Alphabet:%s\nDigit: %s\n", alphabet, digit);
        sendto(sockfd, (const char *)alphabet, strlen(alphabet), MSG_CONFIRM, (const struct sockaddr *)&cliaddr1, len);
        sendto(sockfd, (const char *)digit, strlen(digit), MSG_CONFIRM, (const struct sockaddr *)&cliaddr1, len);
        if (strlen(buffer) == 0)
        {
            printf("Received empty string, Exit\n");
            break;
        }
    }
    // Close the socket
    close(sockfd);

    return 0;
}
