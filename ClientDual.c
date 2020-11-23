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
#define MAXQUEUE 10

void server(int consockfd) {
  char reqbuf[MAXREQ];
  int n;
  while (1) {                   
    memset(reqbuf,0, MAXREQ);
    n = read(consockfd,reqbuf,MAXREQ-1); /* Recv */
    printf("%s\n", reqbuf);
    if (n <= 0) return;
    //n = write(consockfd, reqbuf, strlen(reqbuf)); /* echo*/
  }
}

void * serv() {

int lstnsockfd, consockfd, clilen, portno = 6000;
struct sockaddr_in serv_addr, cli_addr;

 memset((char *) &serv_addr,0, sizeof(serv_addr));
 serv_addr.sin_family      = AF_INET;
 serv_addr.sin_addr.s_addr = INADDR_ANY;
 serv_addr.sin_port        = htons(portno);

// Server protocol
/* Create Socket to receive requests*/
lstnsockfd = socket(AF_INET, SOCK_STREAM, 0);

/* Bind socket to port */
bind(lstnsockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr));
//printf("Bounded to port\n");
while (1) {
   //printf("Listening for incoming connections\n");

/* Listen for incoming connections */
   listen(lstnsockfd, MAXQUEUE); 

   //clilen = sizeof(cl_addr);

/* Accept incoming connection, obtaining a new socket for it */
   consockfd = accept(lstnsockfd, (struct sockaddr *) &cli_addr,       
                      &clilen);
   //printf("Accepted connection\n");

   server(consockfd);

   close(consockfd);
  }
close(lstnsockfd);
}

char *makeHeader(char *inbuf, char *userId) {  //7 userid + 1 grpflag + 4 len
	char header[12];
	char buf[100];
	memset(buf,0,100);
	memset(header,0,12);
	strcpy(header, userId);
	if(inbuf[0] == '@') {
		strcat(header,"1");
	}
	else {
		strcat(header,"0");
	}
	if(strncmp("exit", inbuf, 4)==0) {
		strcat(header, "3");
	}
	int len = strlen(inbuf);
	int *tmp = (int*)&header[8];
	tmp[0] = len;
	strcpy(buf, inbuf);
	strcpy(inbuf, header);
	strcat(inbuf, buf);
	return inbuf;
}

char *getreq(char *inbuf, char *userId, int len) {
  /* Get request char stream */
  memset(inbuf,0,len);          /* clear for good measure */
  fgets(inbuf,len,stdin); /* read up to a EOL */
	return makeHeader(inbuf, userId);
}

void client(int sockfd, char *userId) {
  int n;
  char sndbuf[MAXIN];
  getreq(sndbuf, userId, MAXIN);        /* prompt */
  while (strlen(sndbuf) > 0) {
    write(sockfd, sndbuf, strlen(sndbuf)); /* send */

    //printf("Wrote message: %s\n",sndbuf);
    
    getreq(sndbuf, userId, MAXIN);                 /* prompt */
  }
}

void sendInitMsg(int sockfd, char *userId) {
	char header[12];
	memset(header, 0, 12);
	strcpy(header,userId);
	strcat(header,"2");
	int *tmp = (int*)&header[8];
	tmp[0] = 1;
	int n;
	n = write(sockfd, header, strlen(header)); /* send */
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


int main() {
	//Client protocol
	printf("Enter your userid\n");
	char userId[8];
	memset(userId,0,8);
	fgets(userId,8,stdin);
	char *serverIP = "0.0.0.0";
	int sockfd, portno = 5000;
	struct sockaddr_in serv_addr;
	pthread_t thrd;
	pthread_create(&thrd, NULL, serv, NULL);
	buildServerAddr(&serv_addr, serverIP, portno);

	/* Create a TCP socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* Connect to server on port */
	connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	printf("Connected to %s:%d\n",serverIP, portno);
	/* Carry out Client-Server protocol */
	sendInitMsg(sockfd, userId);
	client(sockfd, userId);

	/* Clean up on termination */
	close(sockfd);
}
