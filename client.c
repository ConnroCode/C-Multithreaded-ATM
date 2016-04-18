//
//  client.c
//  final
//
//  Created by Connor on 12/7/15.
//  Copyright (c) 2015 Connor. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define PORT 61230
int main(int argc, char * argv[]){
    int sockfd, comm;
    struct sockaddr_in server;
    struct hostent* serverMachine;
    char buffer[108], serverTalk[250];
    //My client takes 2 inputs...
    //argv[0] = exe
    //argv[1] = machineName
    if(argc != 2){
        puts("Enter valid input to command prompt plox");
        exit(1);
    }
    //Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(sockfd <= 0){
        printf("Error. Create Socket Failed\n");
        exit(1);
    }
    serverMachine = gethostbyname(argv[1]);
    //Connecting to server
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    memcpy(&server.sin_addr, serverMachine->h_addr_list[0], serverMachine->h_length);
    
    while(connect(sockfd,(struct sockaddr*)&server,sizeof(server)) != 0){
     puts("Looking for server");
     sleep(3);
     }
    
    
    printf("Connected to server!\n");
    
    while(1){
        recv(sockfd, serverTalk, sizeof(serverTalk), 0);
        puts(serverTalk);
        if(strcmp(serverTalk,"Exit") == 0){
            break;
        }
        fgets(buffer, 108, stdin);
        write(sockfd, buffer, strlen(buffer));
        memset(&buffer[0], 0, sizeof(buffer));
        memset(&serverTalk[0],0,sizeof(serverTalk));
        
        
    }
    //Send messages to the server.
    close(sockfd);
    return 0;
}