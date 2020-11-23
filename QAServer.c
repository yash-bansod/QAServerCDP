#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXIN 100
#define MAXOUT 100
#define MAXREQ 100
#define MAXQUEUE 5

struct userinfo {
	char username[10];
	char ip[10];
	bool isgrp;
	int mode;		//-1:nil
	int qtype;
	int qno;
};
struct userinfo users[5];
int numusers = 0;

char *getreq(char *inbuf, int len) {
  /* Get request char stream */
  printf("REQ: ");              /* prompt */
  memset(inbuf,0,len);          /* clear for good measure */
  return fgets(inbuf,len,stdin); /* read up to a EOL */
}

void client(int sockfd, char* sndbuf) {
  int n;
  write(sockfd, sndbuf, strlen(sndbuf));
  //getreq(sndbuf, MAXIN);        /* prompt */
  //while (strlen(sndbuf) > 0) {
  //  write(sockfd, sndbuf, strlen(sndbuf)); /* send */

  //  //printf("Wrote message: %s\n",sndbuf);
  //  
  //  getreq(sndbuf, MAXIN);                 /* prompt */
  //}
}

// Server address
struct hostent *buildServerAddr(struct sockaddr_in *serv_addr,
	char *serverIP, int portno) {
  /* Construct an address for remote server */
  memset((char *) serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr->sin_family = AF_INET;
  inet_aton(serverIP, &(serv_addr->sin_addr));
  serv_addr->sin_port = htons(portno);
 }


void cli(struct userinfo * user, char *inbuf) {
	//Client protocol
	char *serverIP = user->ip;
	int sockfd, portno = 6000;
	struct sockaddr_in serv_addr;
	buildServerAddr(&serv_addr, serverIP, portno);

	/* Create a TCP socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* Connect to server on port */
	connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	printf("Connected to %s:%d\n",serverIP, portno);
	/* Carry out Client-Server protocol */
	char sndbuf[100];
	memset(sndbuf,0,100);
	printf("Username: %s\nip: %s\nLength: %d\nInput: %c\n",user->username, user->ip, inbuf[8], inbuf[9]);
	if(user->mode == -1) {
		printf("Mode: %d\n",user->mode);
		strcpy(sndbuf,"Welcome ");
		strcat(sndbuf,user->username);
		strcat(sndbuf,"\nSelect Mode:\n1)Individual Mode\n2)Group Mode\n3)Admin Mode");
		user->mode = 0;
		client(sockfd,sndbuf);
		printf("Mode: %d\n",user->mode);
	}
	else if(user->mode == 0) {
		printf("Mode: %d\n",user->mode);
		if(inbuf[9] == '1') {
			user->mode = 1;
			strcpy(sndbuf, "Select type of question\n");
			strcat(sndbuf, "1) 2) 3)..");
			client(sockfd,sndbuf);
		}
		else if(inbuf[9] == '2') {
			strcpy(sndbuf, "Select teammate\n");
			strcat(sndbuf, "1) 2) 3)..");
			client(sockfd,sndbuf);
			user->isgrp = true;
		}
		else {
			//admin
		}
		printf("Mode: %d\n",user->mode);
	}
	else if(user->mode == 1) {
		printf("Mode: %d\n",user->mode);
		user->mode = 2;
		if(inbuf[9] == '1') {
			user->qtype = 0;
			//Draw question from type 1
			//set qno
			strcpy(sndbuf, "Question:\n");
			// Display question with 1,2,3,4 choices
			client(sockfd,sndbuf);
		}
		else if(inbuf[9] == '2') {
			user->qtype = 1;
			//Draw question from type 1
			strcpy(sndbuf, "Question:\n");
			// Display question with 1,2,3,4 choices
			client(sockfd,sndbuf);
		}
		printf("Mode: %d\n",user->mode);
	}
	else if(user->mode == 2) {
		printf("Mode: %d\n",user->mode);
		if(inbuf[9] == '1') {
			//check qno.answer equals 1
			strcpy(sndbuf, "Answer:\n");
			//show answer
			user->mode = 1; // and clear qno
			strcpy(sndbuf, "Select type of question\n");
			strcat(sndbuf, "1) 2) 3)..\n");
			client(sockfd, sndbuf);
		}
		printf("Mode: %d\n",user->mode);
	}


	/* Clean up on termination */
	close(sockfd);
}

void server(int consockfd, char* ipa) {
  char reqbuf[MAXREQ];
  int n;
  while (1) {                   
    memset(reqbuf,0, MAXREQ);
    n = read(consockfd,reqbuf,MAXREQ-1); /* Recv */
    printf("Recvd msg:%s\n", reqbuf);
	if(numusers==0) {
		strncpy(users[numusers].username,reqbuf,7);
		strcpy(users[numusers].ip,ipa);
		users[numusers].isgrp = false;
		users[numusers].mode = -1;
		users[numusers].qtype = -1;
		users[numusers].qno = -1;
		cli(&users[numusers], reqbuf);
		numusers++;
	}
	else {
		int index=0;
		int found=0;
		for(int i=0;i<numusers;i++) {
			if(strcmp(users[i].ip, ipa) == 0) {
				found=1;
				printf("found\t%d\n",users[i].mode);
				cli(&users[i], reqbuf);
				break;
			}
		}
		if(found == 0) {
			strncpy(users[numusers].username,reqbuf,7);
			strcpy(users[numusers].ip,ipa);
			users[numusers].mode = -1;
			users[numusers].qtype = -1;
			users[numusers].qno = -1;
			cli(&users[numusers], reqbuf);
			numusers++;
		}
	}
    if (n <= 0) return;
    //n = write(consockfd, reqbuf, strlen(reqbuf)); /* echo*/
  }
}

int main() {

int lstnsockfd, portno = 5000;
struct sockaddr_in serv_addr;

 memset((char *) &serv_addr,0, sizeof(serv_addr));
 serv_addr.sin_family      = AF_INET;
 serv_addr.sin_addr.s_addr = INADDR_ANY;
 serv_addr.sin_port        = htons(portno);

// Server protocol
/* Create Socket to receive requests*/
lstnsockfd = socket(AF_INET, SOCK_STREAM, 0);

/* Bind socket to port */
bind(lstnsockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr));
printf("Bounded to port\n");
while (1) {
	struct sockaddr_in cli_addr;
   printf("Listening for incoming connections\n");

/* Listen for incoming connections */
   if(listen(lstnsockfd, MAXQUEUE)!=0) {
	   printf("listen failed\n");
   } 

   //clilen = sizeof(cl_addr);

/* Accept incoming connection, obtaining a new socket for it */
	int clilen;
	int consockfd = accept(lstnsockfd, (struct sockaddr *) &cli_addr,&clilen);
	if(consockfd<0) {
	   printf("accept failed\n");
	}
	else { 
		printf("Accepted connection\n");
		char *ipa = inet_ntoa(cli_addr.sin_addr);
		//pthread_t thrd;
		//pthread_create(&thrd,NULL,cli, (void*)ipa);
		server(consockfd, ipa);
	}

   close(consockfd);
  }
close(lstnsockfd);
}
