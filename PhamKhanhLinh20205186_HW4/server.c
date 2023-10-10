#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#define MAX 50
#define BUF_SIZE 1024


int numLoginErr = 0;
char currentUser[30];


typedef struct Account{
    char username[30];
    char password[40];
    int status;    
}Account;


//linked list include data and the pointer points to next node
typedef struct node{
    Account data;
    struct node *next;
}node;


//crate the first element of the link list
node *createHead(Account x){
    node *root = (node*)malloc(sizeof(node));
    root->next = NULL;
    root->data = x;
    return root;
}

//insert node at the end of the linked list
node *addNode(node *endlist, Account x){
    node *temp = createHead(x);
    endlist->next = temp;
    return temp;
}

//find the data in linked list by username
node* Find(node *root, char username[]){
    node *p = root;
    while(p != NULL){
        if(strcmp(p->data.username, username) == 0){
            return p;
        }
        p = p->next;
    }
    return p;
}
void printList(node *root, FILE *fout){
    node *p = root;
    while(p != NULL){
        fprintf(fout,"%s %s %d\n",p->data.username,p->data.password,p->data.status);
        p = p->next;
    }
}

    
//check whether username exists then check account is acctive or not
//check number of consecutive incorrect password attempts >= 3 then account is block   
char* signIn(node *root, char username[], char password[]){
    char *message;
    node *p = Find(root, username);
    if(p == NULL){
        strcpy(message ,"Cannot find account");
        return message;
    }else if(p->data.status == 0){
        strcpy(message , "Account is blocked");
        return message;
    }else{
        if(strcmp(p->data.password, password) == 0){
           
           //nếu đăng nhập thành công thì gửi về Client đăng nhập thành công

           
            strcpy(message , "Successfully login");
            numLoginErr = 0;
        }else{
            strcpy(message , "Password is incorrect");
            //increase the number of incorrect password 
            //if number of incorrect password is greater than 3 then account is block
            numLoginErr++; 
            if(numLoginErr >= 3){
                p->data.status = 0;
                strcpy(message ,"Account is block");
            }
            return message;
        }
    }
} 


char* changePassword(node *root, char username[], char password[]){
    node *p = Find(root, username);
    strcpy(p->data.password, password); 
    char *message;
    printf("Change password successfully");
    strcpy(message, "Change password successfully");
    return message;
}

char* signOut(node *root){
    printf("test sign out");
    FILE *fout = fopen("newaccount.txt", "w+");
    printList(root, fout);
    char *message;
    strcpy(message,"Sign out. Byee");
    return message;
}



int main(int argc, char *argv[]){
    //read the file from account.txt file
    FILE* fin = fopen("account.txt", "r");
    //read file. If error then terminate the program
    if(fin == NULL){
        printf("Cannot open file %s", "account.txt\n");
        return 0;
    }

    Account account;
    fscanf(fin, "%s %s %d", account.username, account.password, &account.status);
    node *root = createHead(account);
    node *p = root;
    while(fscanf(fin, "%s %s %d", account.username, account.password, &account.status) != EOF){
        p = addNode(p, account);
    }
    fclose(fin);
    char username[MAX];
    char password[MAX];
    //declear variable for server:
    int server_sock;
    char message[BUF_SIZE];
    int str_len;
    socklen_t client_addr_size;
    struct sockaddr_in server_addr, client_addr1, client_addr2;

    //check whether the port number is provided 
    if(argc != 2){
        printf("Syntax error: %s <port>\n", argv[0]);
        exit(0);
    }

    //Create UPD socket
    server_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(server_sock < 0){
        perror("Error. Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //set the address info for server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    //Bind the socket to server address
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //Create client address information
    client_addr_size = sizeof(client_addr1);
    memset(&client_addr1, 0, client_addr_size);
    memset(&client_addr2, 0, client_addr_size);
    
    //track client is connect or not
    int client_cnt1 = 0;
    int client_cnt2 = 0;
    while(1){
        if(client_cnt1 == 0)
        {
            //receive 1
            str_len = recvfrom(server_sock, message, BUF_SIZE, 0, (struct sockaddr *)&client_addr1, &client_addr_size);
            message[str_len] = '\0';

            //reply to client 1 that they connect successfully
            char reply[BUF_SIZE] = "client 1 connected";
            //send 2
            sendto(server_sock, reply, strlen(reply), 0, (struct sockaddr *)&client_addr1, client_addr_size);
			client_cnt1 = 1;
        }
        //wait for client 2 to connect
        else if(client_cnt2 == 0){
            str_len = recvfrom(server_sock, message, BUF_SIZE, 0, (struct sockaddr *)&client_addr2, &client_addr_size);
			message[str_len] = '\0';
            
            //reply to client 2 that they connect successfully
            char reply[BUF_SIZE] = "Client2 connected";
			sendto(server_sock, reply, strlen(reply), 0, (struct sockaddr *)&client_addr2, client_addr_size);
			client_cnt2 = 1;
        }
        //Both client 1 and 2 are connected to server. Server receives mess 
        //from client 1 and send to client 2
        else{
            //clear buffer
        	memset(message, 0, BUF_SIZE);

            //receive the account info from client 1
            //receive name, pwd
            str_len = recvfrom(server_sock, message, BUF_SIZE, 0, (struct sockaddr *)&client_addr1, &client_addr_size);
			if (str_len == 0){
				break;
			}
			sscanf(message, "%s %s", username, password);
            char *messageSignIn = signIn(root, username, password);
         	//send message of login
            sendto(server_sock, messageSignIn, strlen(messageSignIn), 0, (struct sockaddr *)&client_addr1, client_addr_size);
            if(strcmp(messageSignIn, "Successfully login") == 0){
                int isSignOut = 0;
                while(1){
                    memset(message, 0, BUF_SIZE);

                    //receive option from client 1
                    str_len = recvfrom(server_sock, message, BUF_SIZE, 0, (struct sockaddr *)&client_addr1, &client_addr_size);
                    message[str_len] = '\0';
                    //1 means: send message, 0 means: change pwd
                    if(strcmp(message, "1") == 0){
                        printf("Client 1 send message:");
                        //receive message
                        str_len = recvfrom(server_sock, message, BUF_SIZE, 0, (struct sockaddr *)&client_addr1, &client_addr_size);
                        message[str_len] = '\0';
                        if (strcmp(message, "bye") == 0){
                           //char *messageSignOut = signOut(root);
                            char messageSignOut[20] = "Sign out\n";
                            //printf("byee %s\n", messageSignOut);
                            FILE *fout = fopen("account.txt", "w+");
                            printList(root, fout);
                            fclose(fout);
                            isSignOut = 1;
                            sendto(server_sock, messageSignOut, strlen(messageSignOut), 0, (struct sockaddr *)&client_addr1, client_addr_size);
                            break;
                        }else{
                            //printf("Client1: %s\n", message);
                            //send message to client 2
                            sendto(server_sock, message, strlen(message), 0, (struct sockaddr *)&client_addr2, client_addr_size);
							printf("Sent letters '%s' to client 2\n", message);
                        }
                    }
                    //if client 1 choose 0 => change password
                    else{
                        str_len = recvfrom(server_sock, message, BUF_SIZE, 0, (struct sockaddr *)&client_addr1, &client_addr_size);
                        message[str_len] = '\0';
                        char *messageChangePwd = changePassword(root, username, message);
                        sendto(server_sock, messageChangePwd, strlen(messageChangePwd), 0, (struct sockaddr *)&client_addr1, client_addr_size);
                    }
                }
            }
        }
    }
    close(server_sock);
    fclose(fin);
    return 0;

}