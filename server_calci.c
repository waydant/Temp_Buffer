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

    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr,&clilen);

    if(newsockfd <0){
        error("Error on Accept");
    }

    // now starting the communication part

    int num1, num2, ans, choice;
    
S:  n = write(newsockfd,"Enter number 1: ", strlen("Enter Number 1")); // Ask for number 1
    if(n<0)
        error("Error writing to socket");
    
    read(newsockfd, &num1, sizeof(int));        // read no 1
    printf("Client - Number 1 is: %d\n", num1);

    n = write(newsockfd,"Enter number 2: ", strlen("Enter Number 2")); // Ask for number 2
    if(n<0)
        error("Error writing to socket");
    
    read(newsockfd, &num2, sizeof(int));        // read no 2
    printf("Client - Number 2 is: %d\n", num2);
    
    n = write(newsockfd,"Enter your choice : \n1.Addition\n2.Subtraction\n3.Multiplication\n4.Division\n5.Exit\n", strlen("Enter your choice : \n1.Addition\n2.Subtraction\n3.Multiplication\n4.Division\n5.Exit\n"));
    if(n<0)
        error("Error writing to socket");

    read(newsockfd, &choice, sizeof(int));
    printf("Client - Choice is: %d\n", choice);

    switch(choice){
        case 1: ans = num1+num2;
            break;
        case 2: ans = num1-num2;
            break;
        case 3: ans = num1*num2;
            break;
        case 4: ans = num1/num2;
            break;
        case 5: goto Q;
            break;
    }

    write(newsockfd, &ans, sizeof(int));

Q:  close(newsockfd);
    close(sockfd);
    return 0;
}