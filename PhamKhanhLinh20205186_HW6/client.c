#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5500
#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    int client_sock;
    char buff[BUFF_SIZE + 1];
    struct sockaddr_in server_addr;
    int msg_len, bytes_sent, bytes_received;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError! Cannot connect to server! Client exits immediately! ");
        return 0;
    }

    // Step 4: Communicate with server

    // Send message
    while (1)
    {
        printf("\nEnter username and password to login:\n(Passwords and accounts are separated by space) ");
        memset(buff, '\0', (strlen(buff) + 1));
        fgets(buff, BUFF_SIZE, stdin);
        msg_len = strlen(buff);

        bytes_sent = send(client_sock, buff, msg_len, 0);
        if (bytes_sent < 0)
            perror("\nError: ");

        // Receive echo reply
        bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0)
            printf("Connection closed.\n");

        buff[bytes_received] = '\0';
        printf("Reply from server: %s\n", buff);
        if (strcmp(buff, "Account is block") == 0)
            break;
        if (strcmp(buff, "Successfully login") == 0)
        {
            printf("Enter any key for log out: \n");
            scanf("%s", buff);
            bytes_sent = send(client_sock, buff, msg_len, 0);
            bytes_received = recv(client_sock, buff, BUFF_SIZE, 0); //nhan thong tin log out tu server
            buff[bytes_received] = '\0';
            printf("%s", buff);
            break;
        }
    }

    // Step 4: Close socket
    close(client_sock);
    return 0;
}
