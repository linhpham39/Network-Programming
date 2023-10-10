#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define max 50
#define BUF_SIZE 1024


int main(int argc, char *argv[])
{
    int isLogin = 0; // check status of client login or not

    int sock;
    char message[BUF_SIZE];
    int str_len;
    socklen_t addr_size;
    struct sockaddr_in server_addr;

    // check valid command line arguments
    if (argc != 3)
    {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // create a UDP socket
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    // initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // send connection request to server
    char connect_request[BUF_SIZE] = "Request Connection";
    //send 1
    sendto(sock, connect_request, strlen(connect_request), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // receive message from server
    //receive 2
    addr_size = sizeof(server_addr);
    str_len = recvfrom(sock, message, BUF_SIZE, 0,
                       (struct sockaddr *)&server_addr, &addr_size);
    message[str_len] = '\0';
    printf("Received Message: %s\n", message);

    if (strcmp(message, "Client2 connected") == 0) // set role of client is 2 and just receive message from client 1 through server
    {
        while (1)
        {
            str_len = recvfrom(sock, message, BUF_SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);
            message[str_len] = '\0';
            
            // case receive message from server and display them            
            printf("%s\n", message);
            str_len = recvfrom(sock, message, BUF_SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);
            message[str_len] = '\0';
            printf("%s\n", message);
            
        }
    }
    else // role of client is 1 and just send the message to the server
    {
        while (1)
        {
            if (isLogin == 0)
            {
                char password[max];
                printf("Enter a Username: ");
                fgets(message, sizeof(message), stdin);
                printf("Enter a password: ");
                fgets(password, sizeof(password), stdin);
                strcat(message, password);
                //send name, pwd
                sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
                memset(message, 0, BUF_SIZE);
                //receive message of login
                str_len = recvfrom(sock, message, BUF_SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);
                printf("%s\n",message);
                if (strcmp(message, "Successfully login") == 0)
                {
                    isLogin = 1;
                    while (1)
                    {
                        printf("Choose option:\n0. Change password.\n1.Send message\n");
                        char option[BUF_SIZE];
                        scanf("%s", option);
                        //send 0-change pwd, 1- send message
                        sendto(sock, option, strlen(option), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
                        if(strcmp(option, "1") == 0){
                            //send message
                            char new_message[BUF_SIZE];
                            printf("Enter a new message or \"bye\" to sign out: ");
                            scanf("%s", new_message);
                            fflush(stdin);
                            //send message
                            sendto(sock, new_message, strlen(new_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
                            if (strcmp(new_message, "bye") == 0)
                            {   
                                str_len = recvfrom(sock, message, BUF_SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);
                                message[str_len] = '\0';
                                isLogin = 0;
                                break;
                            }
                        }
                        //if client choose 0- change pwd
                        else{
                            char new_message[BUF_SIZE];
                            printf("Enter new password: ");
                            scanf("%s", new_message);
                            sendto(sock, new_message, strlen(new_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
                            str_len = recvfrom(sock, message, BUF_SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);
                            message[str_len] = '\0';
                            printf("%s\n", message);
                        }
                    }
                }
            }
        }
    }
    close(sock);
    return 0;
}