/*
    command line argument expected:
    filename server_ipaddress portno

    arg[0] filename
    arg[1] server_ipaddress
    arg[2] portno

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for read/write
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> // defines the hostent struct, which is used to store info about host(name, address etc)


void error(const char* msg){
    perror(msg); // outputs through STD_ERR
    exit(1); // terminates our program
}

int main( int argc, char *argv[]){
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buff[255];
    if(argc<3){
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(1);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if(sockfd<0)
        error("ERROR opening socket.");
    // no newsockfd created, as we dont build a connection here

    server = gethostbyname(argv[1]);
    if(server == NULL){
        fprintf(stderr, "Error, no such host");
    }

    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // bcopy function copies n bytes from hostent to serv_addr

    bcopy((char*)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    
    serv_addr.sin_port = htons(portno); // host to network short

    // connect function
    if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
        error("Connection failed!");
    // now we are connected to our server

    while(1){
        bzero(buff, 255);
        fgets(buff,255, stdin);
        n = write(sockfd, buff, strlen(buff));
        if(n<0)
            error("Error on writing");
        bzero(buff, 255);
        n = read(sockfd, buff, 255);
        if(n<0)
            error("Error on reading");
        printf("Server: %s", buff);
        // printf("Client : \n");
        int i = strncmp("Bye", buff,3);
        if(i == 0)
            break;   
    }

    close(sockfd);
    return 0;

}