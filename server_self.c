#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


void error(const char* msg){
    perror(msg); // outputs through STD_ERR
    exit(1); // terminates our program
}

int main(int argc, char *argv[]){
    // 2 parameters we will pass
    /*hence argc = 2. 1-> filename, 2->portno*/
    if(argc<2){
        fprintf(stderr,"Port Number not provided. Program Terminated\n");
        exit(1);
    }

    int sockfd, newsockfd, portno, n;
    // newsockfd is the socket pointer after the connection is est.
    char buff[255];

    struct sockaddr_in serv_addr, cli_addr; // sockaddr_in gives/ contains the internet address, this is included in the netinet/in.h file
    // serv_addr -> server, cli_addr -> client
    socklen_t clilen; //socklen_t is a 32 bit datatype in socket.h 

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){
        error("Error opening Socket.");
    }
    // now socket has been created
    // bzero clears the text/data of anything it is referenced to
    bzero((char *)&serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno); // htons= host to network short

    // now binding
    if(bind(sockfd,(struct sockaddr*)&serv_addr, sizeof(serv_addr))<0){
        error("Binding Failed");
    }

    listen(sockfd, 5); // 5 = max no of clients server can service at a time
    clilen = sizeof(cli_addr);

    sockfd = accept(sockfd, (struct sockaddr*)&cli_addr,&clilen);

    if(sockfd <0){
        error("Error on Accept");
    }

    // now starting the communication part

    while(1){
        bzero(buff,255);
        n = read(sockfd, buff, 255);
        if(n<0){
            error("Error on reading.");
        }
        printf("Client : %s\n", buff);
        // printf("Server : \n");
        bzero(buff,255);
        //fgets belongs to to stdio.h lib, it reads bytes from stream
        fgets(buff,255,stdin); // reads the buffer, of len 255, from the stdin stream

        n = write(sockfd, buff,strlen(buff));
        if(n<0)
            error("Error on writing.");
        int i = strncmp("Bye", buff,3);
        if(i == 0)
            break;            
    }

    // close(sockfd);
    close(sockfd);
    return 0;
}