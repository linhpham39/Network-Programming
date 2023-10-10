#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#define BACKLOG 2 /* Number of allowed connections */
#define BUFF_SIZE 100000

void split_string(char *input, char *letters, char *digits);


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./server PortNumber\n");
        return 0;
    }
    int PORT = atoi(argv[1]);
    int listen_sock, conn_sock; /* file descriptors */
    char recv_data[BUFF_SIZE];
    int bytes_sent, bytes_received;
    // server's address information 
    struct sockaddr_in server;
    //client's address information 
    struct sockaddr_in client; 
    int sin_size;

    // Create a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { 
        perror("\nError: ");
        return 0;
    }

    // Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);             
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    { 
        perror("\nError: ");
        return 0;
    }

    //  Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    { 
        perror("\nError: ");
        return 0;
    }

    // Handle communication with client
    while (1)
    {
        // receive request connection from client
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");
        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); 

        // handle command from client
        while (1)
        {
            bytes_received = recv(conn_sock, recv_data, BUFF_SIZE, 0);

            if (bytes_received <= 0)
            {
                exit(EXIT_FAILURE);
            }

            recv_data[bytes_received] = '\0';
            printf("Received request: %s\n", recv_data);
            if (strcmp(recv_data, "Send string") == 0)
            {
                char str[BUFF_SIZE];
                int n = recv(conn_sock, str, BUFF_SIZE - 1, 0);
                str[n] = '\0';
                char letters[BUFF_SIZE], digits[BUFF_SIZE];
                split_string(str, letters, digits);
                printf("Received string:\n%s\n%s\n", letters, digits);
            }
            else if (strcmp(recv_data, "Send file") == 0)
            {
                char file_content[BUFF_SIZE];
                bytes_received = recv(conn_sock, file_content, BUFF_SIZE - 1, 0);
                if (bytes_received >= BUFF_SIZE)
                {
                    printf("Error: Size of file is larger than size of buffer.\n");
                    break;
                }
                if (bytes_received <= 0)
                {
                    printf("Connection closed!\n");
                    return 0;
                }
                file_content[bytes_received] = '\0';
                printf("File content:\n%s\n", file_content);
            }
            else if (strcmp(recv_data, "bye") == 0)
            {
                close(listen_sock);
                return 0;
            }
        }
    }
    return 0;
}

void split_string(char *input, char *letters, char *digits)
{
    int i, j, k;
    j = k = 0;
    for (i = 0; input[i] != '\0'; i++)
    {
        if (isalpha(input[i]))
        {
            letters[j++] = input[i];
        }
        else if (isdigit(input[i]))
        {
            digits[k++] = input[i];
        }
        else
        {
            strcpy(letters, "Error");
            strcpy(digits, "Error");
            return;
        }
    }
    letters[j] = '\0';
    digits[k] = '\0';

}