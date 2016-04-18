//
//  server.c
//  final
//
//  Created by Connor on 12/7/15.
//  Copyright (c) 2015 Connor. All rights reserved.
//
#define PORT 61230
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>
#include <ctype.h>
struct Account
{
    char accountName[100];
    float balance;
    int useFlag;                //1 = IN SESSION; 0 = NOT IN SESSION
    struct Account *next;
    
};
typedef struct Account account;
//Global Variables
int bankSize;                   //Current # of Accounts Present
account* bankHead;              //Current head of LL of Bank Accounts

void bankPrinter();
void intHandler();
account* createAccount(char* userName);
account* accountLookup(char* accountName, account* head);
void openAccount(char* accountName, int socket);
void* clientHandler(void * socket);

int main(int argc, const char * argv[]) {
    /** Server
     1. Open a socket
     2. Bind to an address and port
     3. Listen for incoming connections
     4. Accept Connections
     5. Thread those connections
     **/
    
    //0. Initialize
    int sockfd, socketSize, newSocket, *newerSocket;
    struct hostent *serv;
    struct sockaddr_in server, client;
    char* welcome;
    struct sigaction sa;
    struct itimerval printTimer;
    signal(SIGALRM, &bankPrinter);
    
    
    struct itimerval timer={0};
    timer.it_value.tv_sec = 1;
    timer.it_interval.tv_sec = 20;
    
    setitimer(ITIMER_REAL, &timer, NULL);
    
    /*
     if(argc != 2){
     printf("Error: Enter the correct number of inputs\n");
     EXIT_FAILURE;
     }
     serv = gethostbyname(argv[1]);
     if(serv == NULL){
     printf("Error ServerHostName\n");
     EXIT_FAILURE;
     }
     */
    welcome = "Welcome User! Please Input A Command:\n open 'accountname'\n start 'accountname'\n credit 'amount'\n debit 'amount'\n balance\n finish\n exit\n";
    socketSize = sizeof(struct sockaddr);
    
    //1. Open Socket
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    
    if(sockfd == -1)
    {
        puts("Failed to Create Socket\n");
        perror("Failed to Create Socket\n");
        return 1;
    }
    puts("Socket Created");
    
    //2. Bind
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    
    /*
     client.sin_family = AF_INET;
     client.sin_port = htons(PORT);
     client.sin_addr = *((struct in_addr *)serv->h_addr);
     */
    
    if(bind(sockfd,(struct sockaddr *)&server, sizeof(server)) < 0){
        puts("Bind Failed");
        perror("Bind Failed");
        return 1;
    }
    puts("Bind Successful");
    
    
    //3. Listen
    listen(sockfd, 3);
    
    //4. Accept Incoming Connections
    puts("Waiting for Connections");
    while((newSocket = accept(sockfd, (struct sockaddr *)&client,(socklen_t*)&socketSize)))
    {
        puts("Connection Accepted");
        write(newSocket, welcome, strlen(welcome));
        
        pthread_t threadCreator;
        newerSocket = malloc(1);
        *newerSocket = newSocket;
        
        if(pthread_create(&threadCreator, NULL, clientHandler, (void*) newerSocket)<0)
        {
            puts("Failure to create new Thread");
            perror("Failure to create new Thread");
            return 1;
        }
        //pthread_join(threadCreator,NULL);
        puts("Thread Created");
        //close(sockfd);
    }
    if(newSocket < 0)
    {
        puts("Failure to Accept");
        perror("Failure to Accept");
        return 1;
    }
    
    return 0;
    
    
}
void bankPrinter(){
    account* curr = bankHead;
    while(curr != NULL){
        if(curr->useFlag == 1){
            printf("%s %.2f IN SESSION\n", curr->accountName, curr->balance);
            curr = curr->next;
            
        }
        else{
            printf("%s %.2f\n",curr->accountName, curr->balance);
            curr = curr->next;
            
        }
    }
}



//Checks if a word is all blankspace...
/* createAccount:
 - Allocates memory for the account Struct
 - Sets the fields
 - Returns created struct
 */


account* createAccount(char* userName){
    
    account* temp = malloc(sizeof(account));
    strcpy(temp->accountName, userName);
    temp->balance = 0;
    temp->next = NULL;
    temp->useFlag = 0;      //0: Not In Session, 1: In Session
    return temp;
}

/* accountLookup
 - iterates through the bank.
 - SUCCESS: Returns account
 - FAILURE: Returns NULL
 */
account* accountLookup(char* accountName, account* head){
    account* curr = bankHead;
    while(curr != NULL){
        if(strcmp(accountName, curr->accountName) == 0){
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/*openAccount : ALMOST DONE: Just NEED TO PREVENT OPENING IN SESSION
 - Error checks.
 - SUCCESS = 0
 - FAILURE = -1
 */
void openAccount(char* accountName, int socket){
    /* Errors:
     - Banksize is Full
     - Account already exists
     - User is Currently in a customer session
     Objectives:
     - Bank is Empty
     - Accountname will be MAX 100 Chars
     */
    
    
    // Bank has no accounts. If size is 0 we set the head to this.
    account *check = accountLookup(accountName, bankHead);
    //Success
    if(bankSize == 0){
        write(socket, "openAccount: Success! Account created\n", strlen("openAccount: Success! New Account created\n"));
        bankSize++;
        bankHead = createAccount(accountName);
        return;
    }
    //Error : Bank is full.
    else if(bankSize == 20){
        send(socket,"openAccount: Error! Bank is Full\n",strlen("openAccount: Error! Bank is Full\n"),0);
        return;
    }
    //Error : Existing Entry
    else if(check != NULL){
        send(socket, "openAccount: Error! Account exists already\n", strlen("openAccount: Error! Account exists already\n"),0);
        return;
    }
    //Success
    else{
        send(socket, "openAccount: Success! New Account created\n", strlen("openAccount: Success! New Account created\n"),0);
        bankSize++;
        account* temp = createAccount(accountName);
        temp->next = bankHead;
        bankHead = temp;
        printf("bankSize = %d.\n",bankSize);
        return;
    }
}

//Parses input and calls the respective function.
//0 = SUCCESS
//1 = FAILURE

//printf("Hold String is %s\n", hold);
//printf("First Word is %s\n",firstWord);
//printf("Second Word is %s\n", secondWord);

// At this point I've successfully parsed the client input...
// Now it's time to do some defesneive programming
// Set string to lowercase

//printf("First Word is %s\n",firstWord);
//printf("Second Word is %s\n", secondWord);

//This is called if commandParser encounters "start" as the first word.
//Reasoning for a separate function is so that we can't open accounts while in session.
void* clientHandler(void * socket){
    
    //Trying to scan input client side
    puts("Client has Connected to Server\n");
    int sock = *(int*)socket;
    unsigned long int inputLength;
    ssize_t read_size;
    char clientInput[108];
    char firstWord[7], secondWord[100];
    char* message = "Please Enter a Command\n";
    //pthread_mutex_t a_mutex = pthread_mutex_initializer;
    //Free the socket pointer
    account* currClient = NULL;
    pthread_mutex_t a_mutex;
    pthread_mutex_init(&a_mutex, NULL);
    
    while((read(sock, clientInput, sizeof(clientInput)))>0)
    {
        char firstWord[7], secondWord[100];
        inputLength = strlen(clientInput);
        
        //Send the message back to client
        if(strlen(clientInput) > 108){
            write(sock,"commandParser Error: Input too large\n",strlen("commandParser Error: Input too large\n"));
            continue;
        }
        int i = 0;
        while(clientInput[i]){
            clientInput[i] = tolower(clientInput[i]);
            i++;
        }
        sscanf(clientInput, "%s %[^\r]\n", firstWord, secondWord);
        //Parse the input...;
        //    printf("ClientInput %s, FirstWord %s, Second Word %s\n", clientInput, firstWord, secondWord);
        
        //printf("First Word is %s", firstWord);
        if(strlen(firstWord) < 4 || strlen(firstWord) > 7){
            write(sock,"commandParser Error: firstWord too small or large\n",strlen("commandParser Error: firstWord too small or large\n"));
            
        }
        else if(strlen(secondWord) > 100){
            write(sock,"commandParser Error: Input too large\n",strlen("commandParser Error: Input too large\n"));
            
        }
        
        //Error Check
        //IF YOU WANT TO CHECK FOR VALID 2nd WORD INPUT DO IT HERE.
        
        //Open
        else if(strcmp(firstWord, "open") == 0)
        {
            //If currClient has a value then there's a client in use
            if(currClient != NULL)
            {
                send(sock, "clientHandler Error: Can't open account during session\n", strlen("clientHandler Error: Can't open account during session\n)"),0);
            }
            else
            {
                openAccount(secondWord, sock);
                
            }
        }
        else if(strcmp(firstWord, "start") == 0)
        {
            //Set currClient
            currClient = accountLookup(secondWord, bankHead);
            //Error : Account does not exist
            if(currClient == NULL){
                send(sock, "clientHandler START Error: Account does not exist\n", strlen("clientHandler START Error: Account does not exist\n"),0);
            
                
            }
            //Error : Account already in session
            else if(currClient->useFlag == 1){
                send(sock, "clientHandler START Error: Account in session by another user\n", strlen("clientHandler START Error: Account in session by another user\n"),0);
            
                
            }
            //Success
            else{
                send(sock, "clientHandler START Success: Session Started\n", strlen("clientHandler START Success: Session Started\n"),0);
                currClient->useFlag = 1;
                
            }
            
        }
        else if(strcmp(firstWord, "credit") == 0)
        {
            //Error : If not in a session
            if(currClient == NULL){
                send(sock, "clientHandler CREDIT Error: Not currently in session\n", strlen("clientHandler CREDIT Error: Not currently in session\n"),0);
                
            }
            else{
                //Success
                char* hold;
                double curr = strtod((secondWord), &hold);
                if(pthread_mutex_trylock(&a_mutex) == 0){
                    currClient->balance = currClient->balance + curr;
                    printf("Current balance - %.2f\n", currClient->balance);
                    send(sock,"clientHandler CREDIT Success!", strlen("clientHandler CREDIT Success!"),0);
                    pthread_mutex_unlock(&a_mutex);
                }
                else{
                    send(sock,"lock error\n", strlen("lock error\n"),0);
                }
                
                
                
            }
        }
        else if(strcmp(firstWord, "debit") == 0)
        {
            //Error : If not in a session
            if(currClient == NULL){
                send(sock, "clientHandler DEBIT Error: Not currently in session\n", strlen("clientHandler DEBIT Error: Not currently in session\n"),0);
            }
            else{
                //Change second-word to a string
                
                char* hold;
                double curr = strtod((secondWord), &hold);
                double calc = currClient->balance - curr;
                if(calc < 0){
                    send(sock, "clientHandler DEBIT Error: Overdraw \n", strlen("clientHandler DEBIT Error: Overdraw \n"),0);
                }
                else if(pthread_mutex_trylock(&a_mutex) == 0){
                    currClient->balance = currClient->balance - curr;
                    send(sock,"clientHandler DEBIT Success!", strlen("clientHandler DEBIT Success!"), 0);
                    pthread_mutex_unlock(&a_mutex);
                }
                else{
                    send(sock,"lock error\n", strlen("lock error\n"), 0);
                }
                
            }
        }
        //First Word is balance
        else if(strcmp(firstWord, "balance") == 0)
        {
            if(currClient == NULL){
                write(sock, "clientHandler BALANCE Error: Not currently in session\n", strlen("clientHandler BALANCE Error: Not currently in session\n"));
                
                
            }
            //Print the balance to client...
            else{
                if(pthread_mutex_trylock(&a_mutex) == 0){
                    char balance[100];
                    sprintf(balance, "%f", currClient->balance);
                    send(sock,balance,strlen(balance),0);
                    pthread_mutex_unlock(&a_mutex);
                    
                }
                else{
                    write(sock,"lock error\n", strlen("lock error\n"));
                }
                
            }
        }
        //First Word is finish
        else if(strcmp(firstWord, "finish") == 0)
        {
            //Error : Not currently in session
            if(currClient == NULL){
                send(sock, "clientHandler Finish Error: Not currently in session\n", strlen("clientHandler Finish Error: Not currently in session\n"),0);
                
            }
            else{
                currClient->useFlag = 0;
                send(sock, "clientHandler Finish Success\n",strlen("clientHandler Finish Success\n"),0);
                
            }
        }
        //First word is exit
        else if(strcmp(firstWord, "exit") == 0)
        {
            if(currClient->useFlag == 1){
                currClient->useFlag = 0;
                printf("Client session finished due to exit\n");
            }
            send(sock,"Exit", strlen("Exit"),0);
        }
        sleep(3);
        memset(&clientInput[0], 0, sizeof(clientInput));
        memset(&firstWord[0], 0, sizeof(firstWord));
        memset(&secondWord[0], 0, sizeof(secondWord));
    }
    
    if(read_size == 0)
    {
        puts("Client Disconnected\n");
        fflush(stdout);
        
    }
    else if(read_size == -1)
    {
        perror("recv failed");
        printf("Input Too Long: Disconnecting Client\n");
    }
    
    write(sock, "Thank you for banking with United Connro\n", strlen("Thank you for banking with United Connro\n"));
    printf("Client Disconnected\n");
    
    free(socket);
    
    
    /* */
    
    
    /*
     Client Handler
     Check User Input:
     0. Request User Input
     1. Open Account
     2. Start Account
     3. Credit Amount
     4. Debit Amount
     5. Balance
     6. Finish
     7. Exit
     */
    
    
    return NULL;
}

