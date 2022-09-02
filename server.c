/* TCPsERver.c */
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define MAX_CLIENT 5
#define MAX_MSG 100

#define TRADER_KEY 0x1000

struct trader
{
    int id;
    char *name;
    int flag;
};

struct buyQueue
{
    int userId;
    int itemId;
    int quantity;
    int price;
    struct buyQueue *next;
};

struct sellQueue
{
    int userId;
    int itemId;
    int quantity;
    int price;
    struct sellQueue *next;
};

struct authorizationresult{  // This Data Structure returns to the server the result of
  char name[50];             // login attempts made by any user
  int userId;
  int number;                // result = 1 - login id and password matched
  int result;                // result = 2 - login id correct but password incorrect
};                           // result = 3 - login id did not match to any username


void bubbleSortBuy(struct buyQueue* head);
void bubbleSortSell(struct sellQueue* head);
void swapBuy(struct buyQueue* a,struct buyQueue* b);
void swapSell(struct sellQueue* a,struct sellQueue* b);
void printListBuy(struct buyQueue* start);
void printListSell(struct sellQueue* start);

void bubbleSortBuy(struct buyQueue* head){
    int swapped, i;
    struct buyQueue *ptr1;
    struct buyQueue *lptr = NULL;
    if(head==NULL)  return;
    do{
        swapped = 0;
        ptr1 = head;
        while(ptr1->next!=lptr){
            if(ptr1->price < ptr1->next->price){
                swapBuy(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    }while(swapped);
}

void bubbleSortSell(struct sellQueue* head){
    int swapped, i;
    struct sellQueue *ptr1;
    struct sellQueue *lptr = NULL;
    if(head==NULL)  return;
    do{
        swapped = 0;
        ptr1 = head;
        while(ptr1->next!=lptr){
            if(ptr1->price > ptr1->next->price){
                swapSell(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    }while(swapped);
}

void swapBuy(struct buyQueue* a,struct buyQueue* b){
    int tmp_userId = a->userId;
    int tmp_itemId = a->itemId;
    int tmp_quantity = a->quantity;
    int tmp_price = a->price;
    a->itemId = b->itemId;
    a->userId = b->userId;
    a->quantity = b->quantity;
    a->price = b->price;
    b->itemId = tmp_itemId;
    b->userId = tmp_userId;
    b->quantity = tmp_quantity;
    b->price = tmp_price;
}

void swapSell(struct sellQueue* a,struct sellQueue* b){
    int tmp_userId = a->userId;
    int tmp_itemId = a->itemId;
    int tmp_quantity = a->quantity;
    int tmp_price = a->price;
    a->itemId = b->itemId;
    a->userId = b->userId;
    a->quantity = b->quantity;
    a->price = b->price;
    b->itemId = tmp_itemId;
    b->userId = tmp_userId;
    b->quantity = tmp_quantity;
    b->price = tmp_price;
}

void printListBuy(struct buyQueue* start){
    struct buyQueue *tmp = start;
    printf("\n");
    while(tmp){
        printf("%d, %d, %d, %d\n", tmp->userId, tmp->itemId, tmp->price, tmp->quantity);
        tmp = tmp->next;
    }
}

void printListSell(struct sellQueue* start){
    struct sellQueue *tmp = start;
    printf("\n");
    while(tmp){
        printf("%d, %d, %d, %d\n", tmp->userId, tmp->itemId, tmp->price, tmp->quantity);
        tmp = tmp->next;
    }
}

/*
    Authorizaton Checker - accepts input of username and password
    return a data structure which contains the authorization result and related details (refer server.h)
*/
struct authorizationresult credcheck1(int uId,char *ps){          
  FILE *fptr=fopen("credentials1.txt","r");                            
  if(fptr==NULL){
    printf("Error in opening the Credentials File\n");
  }
  char details[5][50];
  char buff[50];
  int i=0;
  while(fgets(buff,sizeof(buff),fptr)!=NULL){// usernames and passwords fetched from the database (credentials1.txt)
    strcpy(details[i],buff);
    details[i][strlen(details[i])-1]='\0';
    i=i+1;
  }
  fclose(fptr);

  struct authorizationresult ret;

  for(i=0;i<5;i++){
    char *fnumber=strtok(details[i],":");// username and password matching from database (credentials1.txt)
    int userId=atoi(fnumber); 
    // char *fname=strtok(NULL,":");
    char *fpassword=strtok(NULL,"\0");
    if(userId==uId){
      if(strcmp(ps,fpassword)==0){
        // strcpy(ret.name,fname);
        ret.number=atoi(fnumber); ret.result=1;// username and password both matched
        return ret;
      }
      else{
        // strcpy(ret.name,fname);
        ret.number=atoi(fnumber); ret.result=2;// wrong password for the given username
        return ret;
      }
    }
  }

  strcpy(ret.name,"NO MATCH");// No match for the username in the database
  ret.number=-1; ret.result=3;
  return ret;
}

int tokensizer(char *str, char* passwd){
    char * ans = (char *)malloc(sizeof(char)*100);
    int i=0;
    while(str[i]!='@'){
        ans[i]=str[i];
        i++;
    }
    i++;
    int j=0;
    while(str[i]!='\0'){
        passwd[j]=str[i];
        j++;
        i++;
    }
    return atoi(ans);
}

int main(int argc, char *argv[])
{
    if(argc!=2){
        printf("Invalid format.\n Please run in ./server <port_number> format\n");
        exit(1);
    }

    printf("1\n");
    int i, j;

    char buffer[100];
    
    void *memI = (void *)0;
    
    struct trader *traders;
    
    srand((unsigned int)getpid());
    

    //TODO : understand shmget
    int shmTraderId = shmget((key_t)TRADER_KEY, sizeof(struct trader), 0666 | IPC_CREAT);
    memI = shmat(shmTraderId, (void *)0, 0);
    
    traders = (struct trader *)memI;
    
    printf("2\n");

    for (i = 0; i < 5; i++)
    {
        (traders + i)->id = i + 1;
        (traders + i)->flag = 0;
        int shmTnameId = shmget((key_t)(TRADER_KEY + i), sizeof(char), 0666 | IPC_CREAT);
        (traders + i)->name = (char *)shmat(shmTnameId, 0, 0);
        sprintf((traders + i)->name, "Trader %d", i + 1);
    }

    printf("3\n");
    struct buyQueue *buyhead, *tempBuy = NULL, *buyList;
    struct sellQueue *sellhead, *tempSell = NULL, *sellList;

    int quantity, price, itemId;
    int quantityTemp, priceTemp, itemIdTemp, userIdTemp;

    // THE FILE LOCK REGION
    int fd;
    struct flock f1;

    int sd, newSd, cliLen, len;
    struct sockaddr_in cliAddr, servAddr;
    char buf[MAX_MSG];
    int optval = 1;

    // TCP SOCKET REGION
    /* build socket address data structure */
    bzero((char *)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(argv[1]);

    printf("4\n");
    /* create socket for passive open */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        perror("cannot open socket ");
        exit(1);
    }

    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

    if (bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("cannot bind port ");
        exit(1);
    }

    listen(sd, MAX_CLIENT);
    printf("5\n");
    pid_t child;
    while (1)
    {
        printf("6\n");
        cliLen = sizeof(cliAddr);
        newSd = accept(sd, (struct sockaddr *)&cliAddr, &cliLen);
        printf("7\n");
        if (newSd < 0){
            perror("cannot accept connection ");
            exit(1);
        }

        child = fork();

        if (child < 0){
            perror("Fork Error");
            exit(1);
        }
        
        if (child == 0){
            close(sd);
            memset(buf, 0x0, MAX_MSG);
            if (len = read(newSd, buf, sizeof(buf))){
                printf("\nUser request received from client: %s.\n", buf);
                // int userId = atoi(buf);
                char *passwd;
                passwd = malloc(sizeof(char)*100);
                int userId = tokensizer(buf,passwd);
                printf("UserId is: %d\nPassword is: %s", userId,passwd);
                struct authorizationresult authRest = credcheck1(userId,passwd);
                int valid = authRest.result;
                if (userId < 1 || userId > 5){
                    strcpy(buf, "unknown");
                    printf("\nUser doesn't exist\n");
                    send(newSd, buf, sizeof(buf), 0);
                    printf("\nClosing current connection with Client!\n");
                    close(newSd);
                    shmdt(traders);
                    exit(0);
                }else if (traders[userId - 1].flag == 1){
                    strcpy(buf, "failed");
                    printf("\nUser logged in from another client!\n");
                    send(newSd, buf, sizeof(buf), 0);
                    printf("\nClosing current connection with Client\n");
                    close(newSd);
                    shmdt(traders);
                    exit(0);
                }else if(valid==2){
                    // Id is correct but password is wrong
                    strcpy(buf,"rejectedpassword");
                    printf("\nPassword entered is wrong\n. Try again once again.\n\n");
                    send(newSd,buf,sizeof(buf),0);
                    printf("\nClosing current connection with Client!\n");
                    close(newSd);
                    shmdt(traders);
                    exit(0);
                }else if(valid==3){
                    // id is wrong itself
                    // strcpy(buf,"rejectedid");
                    // printf("\n")
                    printf("\n\nNo such userId exists, Try again!!\n\n");
                }else{
                    printf("\nTrader %d logged in!\n", userId);
                    send(newSd, buf, sizeof(buf), 0);
                    traders[userId - 1].flag = 1;
                    int textchecker;
                    while (len = read(newSd, buf, sizeof(buf)))
                    {
                        if (!strcmp(buf, "exit"))
                        {
                            printf("\nTrader %d logged out!\n", userId);
                            traders[userId - 1].flag = 0;
                            close(newSd);
                            shmdt(traders);
                            exit(0);
                        }
                        else
                        {
                            /* 
                                buf is of the order:
                                x, itemId, price, quantity
                            */
                            sscanf(buf, "%d ", &textchecker);
                            if (textchecker == 1) // SELL
                            {
                                // FILE LOCK SECTION START
                                // f1.l_type = F_WRLCK;
                                // f1.l_whence = SEEK_SET;
                                // f1.l_start = 0;
                                // f1.l_len = 0;
                                // f1.l_pid = getpid();
                                // if ((fd = open("lock.txt", O_RDWR)) == -1)
                                // {
                                //     perror("\nError in opening the file!");
                                //     exit(1);
                                // }
                                // while (fcntl(fd, F_SETLK, &f1) < 0)
                                //     ;
                                // FILE LOCK SECTION END

                                sscanf(buf, "%*d %d %d %d ", &itemId, &price, &quantity);
                                printf("\nSale requested of %d no of item %d at %d by trader %d\n", quantity, itemId, price, userId);
                                printf("Completed reading full buff\n");
                                FILE *buy = fopen("buy.txt", "r"),
                                     *sell = fopen("sell.txt", "a"),
                                     *trade = fopen("trades.txt", "a");
                                printf("Opened all the necessary files\n");
                                buyhead = (struct buyQueue *)malloc(sizeof(struct buyQueue));
                                buyList = buyhead;
                                printf("Starting to read the buy.txt file\n");
                                char* tmp_buff = fgets(buffer, 100, buy);
                                while (tmp_buff != NULL)
                                {
                                    sscanf(buffer, "%d %d %d %d", &userIdTemp, &itemIdTemp, &priceTemp, &quantityTemp);
                                    printf("\n NEE 1 - %d %d %d %d\n", userIdTemp, itemIdTemp, priceTemp, quantityTemp);
                                    buyList->itemId = itemIdTemp;
                                    buyList->quantity = quantityTemp;
                                    buyList->price = priceTemp;
                                    buyList->userId = userIdTemp;
                                    tmp_buff = fgets(buffer, 100, buy);
                                    if(tmp_buff !=NULL){
                                        buyList->next = (struct buyQueue *)malloc(sizeof(struct buyQueue));
                                        buyList = buyList->next;
                                    }else{
                                        buyList->next = NULL;
                                    }
                                }
                                printf("Completed reading buy.txt file\n");
                                // buyList= NULL;
                                fclose(buy);
                                buyList = buyhead;
                                printListBuy(buyhead);
                                bubbleSortBuy(buyhead);
                                printListBuy(buyhead);
                                printf("Now traversing the buyList\n");
                                int tobeAdded = 1;
                                while (buyList!=NULL)
                                {
                                    if (buyList->userId == userId || buyList->price < price || buyList->itemId!=itemId)
                                        buyList = buyList->next;
                                    else
                                    {
                                        if (buyList->quantity > quantity)
                                        {
                                            printf("\nbuyList->quantity > quantity\n");
                                            int tempQ = buyList->quantity - quantity;
                                            buyList->quantity = tempQ;
                                            fprintf(trade, "%d %d %d %d %d\n", userId, buyList->userId, itemId, buyList->price, quantity);
                                            quantity = 0;
                                            tobeAdded=0;
                                            break;
                                        }
                                        else
                                        {
                                            fprintf(trade, "%d %d %d %d %d\n", userId, buyList->userId, itemId, buyList->price, buyList->quantity);
                                            if (buyList->quantity == quantity)
                                            {
                                                // tempBuy = buyList;
                                                // buyList = tempBuy->next;
                                                // free(tempBuy);
                                                buyList->userId=0;
                                                buyList->itemId=0;
                                                buyList->price=0;
                                                buyList->quantity=0;
                                                quantity = 0;
                                                tobeAdded=0;
                                                break;
                                            }
                                            else
                                            {
                                                quantity -= buyList->quantity;
                                                buyList->userId=0;
                                                buyList->itemId=0;
                                                buyList->price=0;
                                                buyList->quantity=0;
                                                buyList = buyList->next;
                                            }
                                        }
                                    }
                                }
                                printf("BuyList traversal over!\n");
                                if (quantity > 0)
                                    fprintf(sell, "%d %d %d %d\n", userId, itemId, price, quantity);
                                FILE *buyW = fopen("buy.txt", "w");
                                printf("Traversing the list once again, to print it in the file!\n");
                                while (buyhead!=NULL)
                                {
                                    if(buyhead->userId==0){
                                        tempBuy = buyhead;
                                        buyhead = tempBuy->next;
                                    }else{
                                        fprintf(buyW, "%d %d %d %d\n", buyhead->userId, buyhead->itemId, buyhead->price, buyhead->quantity);
                                        tempBuy = buyhead;
                                        buyhead = tempBuy->next;
                                    }
                                }
                                printf("Completed the so called the list\n");
                                free(tempBuy);
                                printf("hi1\n");
                                free(buyhead);
                                printf("hi2\n");
                                // free(buyList);
                                printf("hi3\n");
                                fclose(buyW);
                                printf("hi4\n");
                                fclose(sell);
                                printf("hi5\n");
                                fclose(trade);
                                printf("hi6\n");
                                // FILE LOCK SECTION START
                                // f1.l_type = F_UNLCK;
                                // fcntl(fd, F_SETLK, &f1);
                                // close(fd);
                                // FILE LOCK SECTION END

                                printf(buf, "Request accepted. \n");
                                sprintf(buf,"Request accepted.\n Please check the trade status or sale status to check the progress.\n");
                                send(newSd, buf, strlen(buf)+1,0);
                            }else if(textchecker==2){ // BUY
                                // FILE LOCK SECTION START
                                // f1.l_type = F_WRLCK;
                                // f1.l_whence = SEEK_SET;
                                // f1.l_start = 0;
                                // f1.l_len = 0;
                                // f1.l_pid = getpid();
                                // if ((fd = open("lock.txt", O_RDWR)) == -1)
                                // {
                                //     perror("\nError in opening the file!");
                                //     exit(1);
                                // }
                                // while (fcntl(fd, F_SETLK, &f1) < 0)
                                //     ;
                                // FILE LOCK SECTION END

                                sscanf(buf, "%*d %d %d %d ", &itemId, &price, &quantity);
                                printf("\nBuy requested of %d no of item %d at %d by trader %d\n", quantity, itemId, price, userId);
                                FILE *buy = fopen("buy.txt", "a"),
                                     *sell = fopen("sell.txt", "r"),
                                     *trade = fopen("trades.txt", "a");
                                sellhead = (struct sellQueue *)malloc(sizeof(struct sellQueue));
                                sellList = sellhead;
                                char* tmp_buff = fgets(buffer, 100, sell);
                                while (tmp_buff != NULL)
                                {
                                    sscanf(buffer, "%d %d %d %d", &userIdTemp, &itemIdTemp, &priceTemp, &quantityTemp);
                                    sellList->itemId = itemIdTemp;
                                    sellList->quantity = quantityTemp;
                                    sellList->price = priceTemp;
                                    sellList->userId = userIdTemp;
                                    tmp_buff = fgets(buffer, 100, sell);
                                    if(tmp_buff != NULL){
                                        sellList->next = (struct sellQueue *)malloc(sizeof(struct sellQueue));
                                        sellList = sellList->next;
                                    }else{
                                        sellList->next = NULL;
                                    }
                                }
                                // sellList= NULL;
                                printf("completed reading sell.txt file\n");
                                fclose(sell);
                                sellList = sellhead;
                                printListSell(sellhead);
                                bubbleSortSell(sellhead);
                                printListSell(sellhead);
                                // while(sellList->next){
                                //     sellList=sellList->next;
                                // }
                                // sellList->next = (struct sellQueue *)malloc(sizeof(struct sellQueue));
                                // sellList->next->itemId = 0;
                                // sellList->next->price = 0;
                                // sellList->next->quantity = 0;
                                // sellList->next->userId = 0;
                                // sellList=sellhead;
                                while (sellList!= NULL)
                                {
                                    if (sellList->userId == userId || sellList->price > price || sellList->itemId!=itemId)
                                        sellList = sellList->next;
                                    else
                                    {
                                        if (sellList->quantity > quantity)
                                        {
                                            printf("sellList->quantity > quantity");
                                            int tempQ = sellList->quantity - quantity;
                                            sellList->quantity = tempQ;
                                            fprintf(trade, "%d %d %d %d %d\n", sellList->userId, userId, itemId, sellList->price, quantity);
                                            quantity = 0;
                                            break;
                                        }
                                        else
                                        {
                                            fprintf(trade, "%d %d %d %d %d\n", sellList->userId, userId, itemId, sellList->price, sellList->quantity);
                                            if (sellList->quantity == quantity)
                                            {
                                                // tempSell = sellList;
                                                // sellList = tempSell->next;
                                                // free(tempSell);
                                                sellList->itemId=0;
                                                sellList->userId=0;
                                                sellList->quantity=0;
                                                sellList->price=0;
                                                quantity = 0;
                                                break;
                                            }
                                            else
                                            {
                                                quantity -= sellList->quantity;
                                                sellList->itemId=0;
                                                sellList->userId=0;
                                                sellList->quantity=0;
                                                sellList->price=0;
                                                sellList = sellList->next;
                                            }
                                        }
                                    }
                                }
                                if (quantity > 0)
                                    fprintf(buy, "%d %d %d %d\n", userId, itemId, price, quantity);
                                FILE *sellW = fopen("sell.txt", "w");
                                printListSell(sellhead);
                                while (sellhead!= NULL)
                                {
                                    if(sellhead->userId==0){
                                        tempSell = sellhead;
                                        sellhead = sellhead->next;
                                    }else{
                                        printf("\n NEE 2 - %d %d %d %d\n", sellhead->userId, sellhead->itemId, sellhead->price, sellhead->quantity);
                                        fprintf(sellW, "%d %d %d %d\n", sellhead->userId, sellhead->itemId, sellhead->price, sellhead->quantity);
                                        tempSell = sellhead;
                                        sellhead = sellhead->next;
                                    }
                                    
                                }
                                free(tempSell);
                                // free(sellList);
                                free(sellhead);
                                fclose(buy);
                                fclose(sellW);
                                fclose(trade);

                                // FILE LOCK SECTION START
                                // f1.l_type = F_UNLCK;
                                // fcntl(fd, F_SETLK, &f1);
                                // close(fd);
                                // FILE LOCK SECTION END
                                printf(buf, "Request accepted. \n");
                                sprintf(buf, "Request accepted. \nPlease check the trade status or buy status of the item to check the progress.\n");
                                send(newSd, buf, strlen(buf) + 1, 0);
                            }
                        }
                    }
                }
            }
        }
        else
            close(newSd);
    }
}