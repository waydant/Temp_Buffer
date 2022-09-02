#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>

#define MAX_NO 99999

#define MAX_MSG 100

#define MEM_SZ 4096

int main(int argc, char *argv[])
{
    if(argc!=3){
        printf("Invalid command line format.\nEnter in the format: ./client <server_address(eg. 127.0.0.1)> <port_number>\n");
        exit(1);
    }
    int i;
    int portno = atoi(argv[2]);
    int sockfd, rc, len;
    char *localhost = argv[1];
    char *password;
    int choice, quantity, price, itemId, sellId, buyId, userId;

    struct sockaddr_in localAddr, servAddr;
    char buf[MAX_MSG], buffer[MAX_MSG];
    printf("Declared all the variables\n");
    FILE *buy = fopen("buy.txt", "a"),
         *sell,
         *trade = fopen("trades.txt", "a");

    printf("Opened the necessary files\n");
    /* build socket address data structure */
    bzero((char *)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(argv[1]);
    // bcopy((char*)localhost, (char *)&servAddr.sin_addr.s_addr,10);
    servAddr.sin_port = htons(7076);

    /* create socket, active open */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("cannot open socket ");
        exit(1);
    }

    printf("Starting the connection!\n");
    /* connect to server */
    rc = connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (rc < 0)
    {
        perror("cannot connect ");
        exit(1);
    }

    printf("\nConnected to the server localhost at TCP port %d\n",portno);

    printf("\nEnter user id to log in: ");
    scanf("%d", &userId);
    password = (char *)(malloc(sizeof(char)*100));
    printf("\nEnter your password:  ");
    scanf("%s", password);
    printf("\nPassword entered is: %s", password);
    sprintf(buf, "%d@%s", userId, password);
    // here we send to the server which user signed in

    send(sockfd, buf, strlen(buf) + 1, 0);
    // here we read the message from the server side
    if (len = read(sockfd, buf, sizeof(buf)))
    {
        if (!strcmp(buf, "failed"))
        {
            printf("\nUser logged in from another client!\n");
            close(sockfd);
            exit(0);
        }else if (!strcmp(buf, "unknown")){
            printf("\nUser doesn't exist!\n");
            close(sockfd);
            exit(0);
        }else if(!strcmp(buf, "rejectedpassword")){
            printf("\nPassword entered is wrong!\n");
            close(sockfd);
            exit(0);
        }else{
            printf("\nLogged you in!\n");
            while (1){
                printf("\nChoose a number among the following options: ");
                printf("\n\t1 - The status of the items");
                printf("\n\t2 - Sell an item");
                printf("\n\t3 - Buy an item");
                printf("\n\t4 - Status of your trades");
                printf("\n\t5 - Exit");
                printf("\n\nPlease select your choice: ");
                scanf("%d", &choice);
                switch (choice)
                {
                case 1:
                    printf("\nWhich Item number's status do you want to know? (Give the id 1-10) :");
                    scanf("%d", &choice);
                    int bestBuyUserId=0, bestBuyPrice=MAX_NO,bestBuyQuantity=0;
                    buy = fopen("buy.txt", "r");
                    sell = fopen("sell.txt", "r");
                    printf("\nBuy requests:");
                    printf("\nBuyer\tItem\tQuantity\tPrice per item");
                    while (fgets(buffer, 100, buy) != NULL)
                    {
                        sscanf(buffer, "%d %d %d %d\n", &buyId, &itemId, &price, &quantity);
                        if (choice == itemId && buyId!=0 && buyId!=userId){
                            printf("\n%d\tItem %d\t%d\t\t%d", buyId, itemId, quantity, price);
                            if(bestBuyPrice>price){
                                bestBuyPrice=price;
                                bestBuyQuantity=quantity;
                                bestBuyUserId=buyId;
                            }
                        }
                    }
                    printf("\n");
                    if(bestBuyUserId==0){
                        printf("\n-\t-\t\t-\n");
                        bestBuyPrice=0;
                    }
                    printf("\n\nBest Buy Details\n");
                    printf("Best Buy Price:\t%d\nNumber of Quantities available for the Best Buy:%d\nSold by(trader ID):%d\n",bestBuyPrice,bestBuyQuantity,bestBuyUserId);
                    fclose(buy);
                    printf("\nSale Requests:");
                    printf("\nSeller\tItem\tQuantity\tPrice per item");
                    int bestSellUserId=0, bestSellPrice=0, bestSellQuantity=0;
                    while (fgets(buffer, 100, sell) != NULL)
                    {
                        sscanf(buffer, "%d %d %d %d\n", &sellId, &itemId, &price, &quantity);
                        if (choice == itemId && sellId!=0 && sellId!=userId){
                            printf("\n%d\tItem %d\t%d\t\t%d", sellId, itemId, quantity, price);
                            if(bestSellPrice<price){
                                bestSellPrice=price;
                                bestSellUserId=sellId;
                                bestSellQuantity=quantity;
                            }
                        }
                    }
                    printf("\n");
                    if(bestSellUserId==0){
                        printf("\n-\t-\t\t-\n");
                        bestSellPrice=0;
                    }
                    printf("Best Sell Price:\t%d\nNumber of Quantities available for the Best Sell:\t%d\nTrader who is ready to buy at Best Sell:\t%d\n",bestSellPrice,bestSellQuantity,bestSellUserId);
                    fclose(sell);
                    break;
                case 2:
                    // printf("\nEnter the details of the item you want to sell:");
                    // printf("\nEnter the item id, price offered, and no of units: ");
                    printf("\nEnter the Item Id you want to sell: ");
                    scanf("%d", &itemId);
                    printf("\nEnter the Price you want to sell: ");
                    scanf("%d", &price);
                    printf("\nEnter the Quantity you want to sell: ");
                    scanf("%d", &quantity);
                    // scanf("%d%d%d", &itemId, &price, &quantity);
                    if (itemId < 1 || itemId > 10)
                    {
                        printf("\nItem doesn't exist. Please try again!");
                        break;
                    }
                    sprintf(buf, "1 %d %d %d ", itemId, price, quantity);
                    send(sockfd, buf, strlen(buf) + 1, 0);
                    printf("\nRequest sent\n");
                    if (len = read(sockfd, buf, sizeof(buf)))
                        printf("%s\n", buf);
                    break;

                case 3:
                    // printf("\nEnter the details of the item you want to buy:");
                    // printf("\nEnter the item id, price offered, and no of units: ");
                    // scanf("%d%d%d", &itemId, &price, &quantity);
                    printf("\nEnter the Item Id you want to buy: ");
                    scanf("%d", &itemId);
                    printf("\nEnter the maximum price you are ready to buy: ");
                    scanf("%d", &price);
                    printf("\nEnter the Quantity you want to buy: ");
                    scanf("%d", &quantity);
                    if (itemId < 1 || itemId > 10)
                    {
                        printf("\nItem doesn't exist. Please try again!");
                        break;
                    }
                    sprintf(buf, "2 %d %d %d ", itemId, price, quantity);
                    send(sockfd, buf, strlen(buf) + 1, 0);
                    printf("\nRequest sent\n");
                    if (len = read(sockfd, buf, sizeof(buf)))
                        printf("%s\n", buf);
                    break;
                case 4:
                    trade = fopen("trades.txt", "r");
                    printf("\nYour trade status:");
                    printf("\nSeller\tBuyer\tItem\tQuantity\tPrice per item");
                    while (fgets(buffer, 100, trade) != NULL)
                    {
                        sscanf(buffer, "%d %d %d %d %d\n", &sellId, &buyId, &itemId, &price, &quantity);
                        if (sellId == userId || buyId == userId)
                            printf("\n%d\t%d\tItem %d\t%d\t\t%d", sellId, buyId, itemId, quantity, price);
                    }
                    printf("\n");
                    fclose(trade);
                    break;
                case 5:
                    printf("\nLogged you out!\nExiting...\n");
                    sprintf(buf, "exit");
                    send(sockfd, buf, strlen(buf) + 1, 0);
                    exit(0);
                    break;
                default:
                    printf("\nPlease try again!");
                    break;
                }
            }
        }
    }
    printf("\nTask Accompolished!!\n");
}