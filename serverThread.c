#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include "linkedlist.h"

#define BUF_SIZE 512
#define NO_OF_CLIENTS 10

struct Llist{
    int client_id;
    struct Llist *next;
};

struct Llist *head = NULL,*tail = NULL;


void create(int c_id){

 struct Llist *nn = (struct Llist*)malloc(sizeof(struct Llist));

 if(nn == NULL){
  printf("Error in allocating memory!");
  return;
 }

 nn->client_id = c_id;
 
 if(head == NULL && tail == NULL){
  head = nn; 
  tail = nn;
  nn->next = NULL;
 }else{
  tail->next = nn;
  tail = nn;
  tail->next = NULL;
 }
}

void findandremove(int c_id){
 
 if(head == NULL){
  printf("Linked list is empty!");
  return;
 }

 struct Llist *ptr = head;
 struct Llist *prevptr = NULL;
 
 while(ptr != NULL){
  if(ptr->client_id == c_id){
    
   if(prevptr == NULL){ //if present in head
     head = head->next;
     if(head == NULL)
     	tail = NULL;
     free(ptr);
     break;
   }else{
     prevptr->next = ptr->next;
     free(ptr);
     break;
   }
  }

  prevptr = ptr;
  ptr = ptr->next;
 }
}

char* print(int c){
 
 struct Llist *ptr = head;
 int i = 1;
 int online = 0;
 char *result=(char*)malloc(500 * sizeof(char));
 strcat(result, "Clients online:\n");
 while(ptr != NULL){
   if(ptr->client_id != c){
	strcat(result, "[*] Client-");
	char a = ptr->client_id + '0';
  	strncat(result, &a,1);
   	strcat(result,"\n");
	online++;
   }
   //printf("%c", a);
   //printf("Client %d id = %d\n", i, ptr->client_id);
   ptr = ptr->next;
   }
 if(online == 0)
	strcpy(result, "No one is online\n");
   return result;
}

int isValid(int c, int clientfd){
	struct Llist *ptr = head;
	while(ptr != NULL){
	   if(ptr->client_id == c && ptr->client_id != clientfd){
		return 1;
	   }
	 	ptr = ptr->next;  
	}
	return 0;
}

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int clientfd[NO_OF_CLIENTS];

void* socketChat (void *arg);

int main(int argc, char *argv[]){
	if(argc < 2){
		printf("SYNTAX :%s [portno]", argv[0]);
		exit(1);
	}
	
	int sockfd, clientfd, portno, retval;
	struct sockaddr_in server_addr, client_addr;
	socklen_t clientlen;
	portno = atoi(argv[1]);

//1-)Socket function
	sockfd = socket(AF_INET, SOCK_STREAM,0);
	if(sockfd < 0){
		perror("Socket Creation Error");
		exit(1);
	}
	bzero((char *)&server_addr, sizeof(server_addr)); //fills with zero
	
	//set the values in structure
	server_addr.sin_family =AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //any local address
	server_addr.sin_port = htons(portno);
	
//2-)Bind function
	if(bind(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		perror("Binding Error");
		exit(1);
	}
	
//3-)Listen function
	if(listen(sockfd, NO_OF_CLIENTS)== 0)
		printf("Listenig on port: %d", portno);
	else
		printf("Error in listening");
	pthread_t pid[NO_OF_CLIENTS]; //pthread id
	int i = 0;
	while(i<NO_OF_CLIENTS){
//4-)Accept function
		clientlen = sizeof(client_addr);
		clientfd = accept(sockfd,(struct sockaddr*)&client_addr, &clientlen);
		if(clientfd <0){
			perror("Accepting Error");
			exit(1);
		}
		else{
			pthread_mutex_lock(&lock);
			create(clientfd);
			pthread_mutex_unlock(&lock);
			printf("\n [+] Client %d connected to server\n", clientfd);
		}						
//5-)Threading
		pthread_create(&pid[i],NULL,&socketChat,&clientfd);
		i++;
	}
	close(sockfd);
	return 0;
}	
	
	
void* socketChat(void *arg){
	char buffer[BUF_SIZE];
	char temp[BUF_SIZE];
	int retval;
	int clientfd = *((int*)arg);
	int targetclient= -1;
	int result = clientfd;
	char a =clientfd + '0';
	
	retval = write(clientfd,&a,1);
	if(retval < 0){
		perror("\nError in sending client id");
		exit(1);
	}
	
	while(1){
		bzero(buffer,BUF_SIZE);
		retval = read(clientfd, buffer, BUF_SIZE);
		
		if(retval <0){
			perror("\nREading Error\n");
			break;
		}
		printf("Client %d: %s", clientfd, buffer);
		if(strncmp(buffer, "exit",4)==0 || strlen(buffer)<1){
			//printf("Client %d: %s", clientfd, buffer);
			result=clientfd;
			break;
		}
		else if(strncmp(buffer, "connect" ,7) == 0){
			targetclient = buffer[8] - '0';
			if(isValid(targetclient, clientfd) == 1){
				printf("[+]Client %d is connected to client %d\n", clientfd, targetclient);
				bzero(buffer,BUF_SIZE);
				strcpy(buffer, "Connected Successfully\n");
				result = clientfd;
				fflush(stdin);
			}
			else{
				printf("[+]Client %d cannot connect to client %d\n", clientfd, targetclient);
				bzero(buffer,BUF_SIZE);
				strcpy(buffer, "Invalid client\n");
				targetclient = -1;
			}
		}
		else if(strncmp(buffer,"show",4)== 0){
			bzero(buffer, BUF_SIZE);
			strcpy(buffer, print(clientfd));
			printf("\n%s", print(clientfd));
			result = clientfd;
			fflush(stdin);
		}
		else if(strncmp(buffer,"disconnect", 10)== 0){
			printf("\n[+] Client %d disconnected from client %d\n", clientfd, targetclient);
			memset(buffer, 0, BUF_SIZE);
			strcpy(buffer,"Disconnected Successfully\n");
			targetclient= -1;
			result = clientfd;
			fflush(stdin);
		}
		else{
			result = targetclient;
			if(result != -1){
				//Changing message format
				//Msg from client CLIENTFD : msg
				bzero(temp, BUF_SIZE);
				strcpy(temp, buffer);
				bzero(buffer, BUF_SIZE);
				strcpy(buffer,"Msg from client");
				a = clientfd + '0';
				strncat(buffer,&a,1);
				strcat(buffer," :");
				strcat(buffer, temp);
				
				printf("\n[+] Sending message from client %d to client %d\n",clientfd, result);
				//Changing message format
				//Sending msg to client TARGET CLIENT
				bzero(temp, BUF_SIZE);
				strcpy(temp,"Sending msg to client ");
				a = targetclient + '0';
				strncat(temp,&a,1);
				strcat(temp, "\n");
				retval = write(clientfd, temp, sizeof(temp));
				if(retval < 0){
					perror("Writing Error");
					break;
				}
			}
		}
		if(result == -1){
			result = clientfd;
		}
		fflush(stdin);
		retval = write(result, buffer, strlen(buffer));
		if(retval <0){
			perror("Writing Error");
			break;
		}
		
	}
	pthread_mutex_lock(&lock);
	findandremove(clientfd);
	pthread_mutex_unlock(&lock);
	retval = write(result, buffer, strlen(buffer)); //send exit message to client
	if(retval < 0){
		perror("Writing Error");
	}
	printf("\n[+] Client %d exiting\n", clientfd);
	close(clientfd);								
}						
			
