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

int login = 0;

void menu()
{
    printf("\nMenu:\n");
    printf("1. Log in\n");
    printf("2. Log out\n");
    printf("Enter your choice: ");
}

void handleLogin(int client_sock)
{
    if(send(client_sock, "1", strlen("1"), 0) < 0){
        perror("Error: ");
        exit(1);
    }
    char username[BUFF_SIZE];
    char password[BUFF_SIZE];
    printf("\nEnter username login:\n");
    scanf("%s", username);
    getchar();
    send(client_sock, username, strlen(username), 0);

    printf("Enter password: ");
    scanf("%s", password);
    getchar();
    send(client_sock, password, strlen(password), 0);

    char buff[BUFF_SIZE];
    memset(buff, 0, sizeof(buff));
    int bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
    if(bytes_received <= 0){
        perror("Error: ");
        exit(1);
    }

    buff[bytes_received] = '\0';
    printf("Reply from server: %s\n", buff);
    if (strcmp(buff, "Successfully login") == 0)
    {
        login = 1;
    }else{
        login = 0;
    }
}

void handleLogout(int client_sock){
    if(send(client_sock, "2", strlen("2"), 0) < 0){
        perror("Error: ");
        exit(1);
    }
    char buff[BUFF_SIZE];
    memset(buff, 0, sizeof(buff));
    int bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
    if(bytes_received <= 0){
        perror("error: ");
        exit(1);
    }
    buff[bytes_received] = '\0';
    printf("Server response: %s\n", buff);
    login = 0;
}

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
    int choice;
    while (1)
    {
        menu();
        scanf("%d", &choice);
        getchar();
        switch (choice)
        {
        case 1:
            if (login)
            {
                printf("You are already logged in. Please log out first.\n");
            }
            else
            {
                handleLogin(client_sock);
            }
            break;
        case 2:

            if (!login)
            {
                printf("Please log out first.\n");
            }
            else
            {
                login = 0;
                handleLogout(client_sock);
            }
            break;
        default:
            printf("Invalid choice. Try again\n");
            break;
        }
    }

        // Step 4: Close socket
        close(client_sock);
        return 0;
}