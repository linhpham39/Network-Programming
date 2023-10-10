#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define BUFF_SIZE 100000

void print_menu()
{
    printf("MENU\n");
    printf("-----------------------------------\n");
    printf("1. Gửi xâu bất kỳ\n");
    printf("2. Gửi nội dung một file\n");
    printf("Enter your choice (1 or 2): ");
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./client IPAddress PortNumber\n");
        exit(EXIT_FAILURE);
    }

    int SERVER_PORT = atoi(argv[2]);
    char *SERVER_ADDR = argv[1];

    int client_sock;
    char buff[BUFF_SIZE];
    struct sockaddr_in server_addr; /* server's address information */
    int msg_len, bytes_sent, bytes_received;

    // Create the socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // set the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError! Can not connect to server! Client exit immediately!\n");
        exit(EXIT_FAILURE);
    }

    // handle communication with server
    int choice;
    while(1)
    {
        print_menu();
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
        {
            char connect_request[BUFF_SIZE] = "Send string";
            send(client_sock, connect_request, strlen(connect_request), 0);
            // Send message
            printf("Insert string to send: ");
            fgets(buff, sizeof(buff), stdin);
            buff[strcspn(buff, "\n")] = 0;
            msg_len = strlen(buff);
            send(client_sock, buff, msg_len, 0);
            break;
        }
        case 2:
        {
            char request[BUFF_SIZE] = "Send file";
            send(client_sock, request, strlen(request), 0);
            printf("\nEnter file name: ");
            fgets(buff, sizeof(buff), stdin);
            buff[strcspn(buff, "\r\n")] = 0; // Remove newline character
            FILE *file = fopen(buff, "r");
            if (file == NULL)
            {
                printf("\nError! Could not open file!\n");
                exit(EXIT_FAILURE);
            }

            // Read the file content
            fseek(file, 0L, SEEK_END);
            long file_size = ftell(file);
            rewind(file);
            char file_content[file_size + 1];
            memset(file_content, 0, file_size + 1);
            fread(file_content, sizeof(char), file_size, file);
            fclose(file);

            // Send the file content
            send(client_sock, file_content, file_size, 0);
            break;
        }
        default:
        {
            printf("\nGoodbye. Connection close\n");
            char end_mes[5] = "bye";
            send(client_sock, end_mes, strlen(end_mes), 0);
            close(client_sock);
            return 0;
        }
        }
    }

    // Close socket
    close(client_sock);
    return 0;
}