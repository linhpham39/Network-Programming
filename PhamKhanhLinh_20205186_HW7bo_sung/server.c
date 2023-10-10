#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/select.h>

#define PORT 5500 /* Port that will be opened */
#define BACKLOG 5 /* Number of allowed connections */
#define BUFF_SIZE 1024

int numLoginErr = 0;

typedef struct Account
{
    char username[30];
    char password[40];
    int status;
    int login; // check whether account has been logined or not
} Account;

// linked list include data and the pointer points to next node
typedef struct node
{
    Account data;
    struct node *next;
} node;

node *root;
// crate the first element of the link list
node *createHead(Account x)
{
    node *root = (node *)malloc(sizeof(node));
    root->next = NULL;
    root->data = x;
    return root;
}

// insert node at the end of the linked list
node *addNode(node *endlist, Account x)
{
    node *temp = createHead(x);
    endlist->next = temp;
    return temp;
}

// find the data in linked list by username
node *Find(char username[])
{
    node *p = root;
    while (p != NULL)
    {
        if (strcmp(p->data.username, username) == 0)
        {
            return p;
        }
        p = p->next;
    }
    return p;
}
void printList(node *root, FILE *fout)
{
    node *p = root;
    while (p != NULL)
    {
        fprintf(fout, "%s %s %d\n", p->data.username, p->data.password, p->data.status);
        p = p->next;
    }
}

// check whether username exists then check account is acctive or not
// check number of consecutive incorrect password attempts >= 3 then account is block
char *signIn(char username[], char password[])
{
    node *p = Find(username);
    char *message = malloc(sizeof(char) * 100);
    if (p == NULL)
    {
        strcpy(message, "Cannot find account"); 
        return message;
    }
    else if (p->data.status == 0)
    {
        strcpy(message, "Account is blocked");
        return message;
    }
    else
    {
        if (strcmp(p->data.password, password) == 0)
        {
            if (p->data.login == 1)
            {
                strcpy(message, "Can not login. Account has been login somewhere");
                return message;
            }
            else
            {
                // nếu đăng nhập thành công thì gửi về Client đăng nhập thành công
                strcpy(message, "Successfully login");
                p->data.login = 1;
                numLoginErr = 0;
                return message;
            }
        }
        else
        {
            strcpy(message, "Password is incorrect");
            // increase the number of incorrect password
            // if number of incorrect password is greater than 3 then account is block
            numLoginErr++;
            if (numLoginErr >= 3)
            {
                p->data.status = 0;
                strcpy(message, "Account is block");
            }
            return message;
        }
    }
}

void handleLogin(int sockfd)
{
    char username[BUFF_SIZE];
    char password[BUFF_SIZE];
    ssize_t recv_username = recv(sockfd, username, sizeof(username) - 1, 0);
    username[recv_username] = '\0';

    // receive password
    ssize_t recv_password = recv(sockfd, password, sizeof(password) - 1, 0);
    password[recv_password] = '\0';

    char *message = signIn(username, password);
    printf("Sent: %s\n", message);
    int bytes_sent = send(sockfd, message, strlen(message), 0);
   if (strcmp(message, "Account is block") == 0)
    {
        FILE *fout = fopen("account.txt", "w+");
        printList(root, fout);
        fclose(fout);
    }
}

void handleLogout(int sockfd)
{
    char messageSignOut[] = "Sign out\n";
    int bytes_sent = send(sockfd, messageSignOut, strlen(messageSignOut), 0); /* gui lai thong tin dang xuat tu client */
    if (bytes_sent < 0)
    {
        perror("\nError: ");
    }
}

void handleChat(int sockfd){
    char message[BUFF_SIZE];
    ssize_t recv_message = recv(sockfd, message, sizeof(message) - 1, 0);
    if(recv_message <= 0){
        perror("Error");
        return;
    }
    message[recv_message] = '\0';
    printf("Receive message from client %d with content:%s\n", sockfd, message);
    /* if(send(sockfd, message, sizeof(message), 0) < 0){
        perror("Error");
        return;
    } */
}

void handleSendImage(int sockfd){
    char message[BUFF_SIZE];
    ssize_t recv_message = recv(sockfd, message, sizeof(message) - 1, 0);
    if(recv_message <= 0){
        perror("Error");
        return;
    }
    message[recv_message] = '\0';
    FILE *file = fopen("img.jpg", "rb");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        ssize_t bytes_sent = send(sockfd, "Failed to open the file.", strlen("Failed to open the file."), 0); 
        if(bytes_sent < 0){
            perror("Error");
        }
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    unsigned char *buffer = (unsigned char *)malloc(file_size);
    size_t bytes_read = fread(buffer, 1, file_size, file);
    // Assuming a grayscale image, print the pixel values
    for (int i = 0; i < file_size; i++) {
        printf("%u ", buffer[i]);
    }
    ssize_t bytes_sent = send(sockfd, "Send successfully", strlen("Send successfully"), 0); 
    printf("%s\n", "Send successfully");
    fclose(file);
    free(buffer);
}

int main(int argc, char *argv[])
{
    // read the file from account.txt file
    FILE *fin = fopen("account.txt", "r");
    // read file. If error then terminate the program
    if (fin == NULL)
    {
        printf("Cannot open file %s", "account.txt\n");
        return 0;
    }

    // check whether the port number is provided
    if (argc != 2)
    {
        printf("Syntax error: %s <port>\n", argv[0]);
        exit(0);
    }

    Account account;
    fscanf(fin, "%s %s %d", account.username, account.password, &account.status);
    root = createHead(account);
    node *p = root;
    while (fscanf(fin, "%s %s %d", account.username, account.password, &account.status) != EOF)
    {
        account.login = 0;
        p = addNode(p, account);
    }
    fclose(fin);

    int listen_sock, conn_sock; // file descriptors
    char recv_data[BUFF_SIZE];
    int bytes_sent, byte_received;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int sin_size;

    // set of socket descriptor for select statement
    // maximum file descriptor number
    fd_set readfds, active_fds;
    int max_fd;
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    // step 1: construct a tcp socket to listen connection request
    if (listen_sock < 0)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }

    // step 2: bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1])); // htons: convert port number to network byte order
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    { /* calls bind() */
        perror("\nError: ");
        return 0;
    }

    // step 3: Listen on socket with backlog size=BACKLOG
    if (listen(listen_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError: ");
        return 0;
    }

    // Initialize the set of active sockets
    FD_ZERO(&active_fds);
    FD_SET(listen_sock, &active_fds);
    max_fd = listen_sock;

    printf("Server started.\n");
    printf("Listening on port %s\n", argv[1]);

    // Step 4: Communicate with clients
    while (1)
    {
        // Block until input arrives on one or more active sockets
        readfds = active_fds;
        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("\nError: ");
            return 0;
        }

        // Process all sockets with input
        for (int i = 0; i <= max_fd; i++)
        {
            if (FD_ISSET(i, &readfds))
            {
                // new connection
                if (i == listen_sock)
                {
                    conn_sock = accept(listen_sock, NULL, NULL);
                    if (conn_sock < 0)
                    {
                        perror("\nError: ");
                        return 0;
                    }
                    FD_SET(conn_sock, &active_fds);
                    if (conn_sock > max_fd)
                    {
                        max_fd = conn_sock;
                    }
                    printf("You got a connection from %d\n", conn_sock); /* prints client's IP */
                }
                // Data arrived on an already-connected socket
                else
                {
                    // receive message from client
                    if ((byte_received = recv(i, recv_data, BUFF_SIZE - 1, 0)) <= 0)
                    {
                        // Got error or connection closed by client
                        FD_CLR(i, &active_fds);
                        close(i); // bye!
                    }
                    else
                    {
                        printf("Received data from socket: %d\n", atoi(recv_data));
                        // echo to client
                        switch (atoi(recv_data))
                        {
                        case 1:
                            handleLogin(i);
                            break;
                        case 2:
                            FILE *fout = fopen("account.txt", "w+");
                            printList(root, fout);
                            fclose(fout);
                            handleLogout(i);
                            break;
                        case 3:
                            byte_received = recv(i, recv_data, BUFF_SIZE - 1, 0);
                            //handle chat
                            if(atoi(recv_data) == 1){
                                handleChat(i);
                            }
                            //hanlde image
                            else if(atoi(recv_data) == 2){
                                handleSendImage(i);
                            }
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }
        close(listen_sock);
        return 0;
}