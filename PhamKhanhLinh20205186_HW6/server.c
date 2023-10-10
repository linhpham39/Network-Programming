#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define PORT 5500  /* Port that will be opened */
#define BACKLOG 20 /* Number of allowed connections */
#define BUFF_SIZE 1024

int numLoginErr = 0;

typedef struct Account
{
    char username[30];
    char password[40];
    int status;
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
char *signIn(char buff[])
{
    char username[100], password[100];
    int i = 0;
    for (i = 0; buff[i] != ' '; i++)
    {
        username[i] = buff[i];
    }
    username[i] = '\0';
    int j = 0;
    for (j = 0; buff[i] != '\0'; j++)
    {
        i++;
        password[j] = buff[i];
    };
    password[j] = '\0';
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

            // nếu đăng nhập thành công thì gửi về Client đăng nhập thành công

            strcpy(message, "Successfully login");
            numLoginErr = 0;
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

/* Receive and echo message to client */
void *echo(void *);

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

    //check whether the port number is provided 
    if(argc != 2){
        printf("Syntax error: %s <port>\n", argv[0]);
        exit(0);
    }

    Account account;
    fscanf(fin, "%s %s %d", account.username, account.password, &account.status);
    root = createHead(account);
    node *p = root;
    while (fscanf(fin, "%s %s %d", account.username, account.password, &account.status) != EOF)
    {
        p = addNode(p, account);
    }
    fclose(fin);

    int listenfd, *connfd;
    struct sockaddr_in server;  /* server's address information */
    struct sockaddr_in *client; /* client's address information */
    int sin_size;
    pthread_t tid;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

    if (bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    sin_size = sizeof(struct sockaddr_in);
    client = malloc(sin_size);
    while (1)
    {
        connfd = malloc(sizeof(int));
        if ((*connfd = accept(listenfd, (struct sockaddr *)client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n", inet_ntoa((*client).sin_addr)); /* prints client's IP */

        /* For each client, spawns a thread, and the thread handles the new client */
        pthread_create(&tid, NULL, &echo, connfd);
    }

    close(listenfd);
    return 0;
}

void *echo(void *arg)
{
    int connfd;
    int bytes_sent, bytes_received;
    char buff[BUFF_SIZE + 1];

    connfd = *((int *)arg);
    free(arg);
    pthread_detach(pthread_self());

    int status = 0; // 0 : chua dang nhap, 1: da dang nhap
    while (1)
    {
        bytes_received = recv(connfd, buff, BUFF_SIZE, 0); // blocking
        buff[strcspn(buff, "\n")] = '\0';
        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0)
            printf("Connection closed.");
        char *message;
        if (status == 0)
            message = signIn(buff);
        bytes_sent = send(connfd, message, strlen(message), 0); /* gui lai thong tin dang nhap tu client */
        if (strcmp(message, "Account is block") == 0)
        {
            FILE *fout = fopen("account.txt", "w+");
            printList(root, fout);
            fclose(fout);
            break;
        }
        if (strcmp(message, "Successfully login") == 0)
        {
            status = 1;
            bytes_received = recv(connfd, buff, BUFF_SIZE, 0); // blocking
            buff[strcspn(buff, "\n")] = '\0';
            char messageSignOut[] = "Sign out\n";
            FILE *fout = fopen("account.txt", "w+");
            printList(root, fout);
            fclose(fout);
            bytes_sent = send(connfd, messageSignOut, strlen(messageSignOut), 0); /* gui lai thong tin dang xuat tu client */
            break;
        }
        free(message);
        if (bytes_sent < 0)
        {
            perror("\nError: ");
        }
    }

    close(connfd);
}
