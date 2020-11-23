#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXIN 40
#define MAXOUT 40
#define MAXREQ 40
#define MAXQUEUE 5

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

char *makeHeader(char *inbuf) {
	
}

char *getreq(char *inbuf, int len) {
  /* Get request char stream */
  memset(inbuf,0,len);          /* clear for good measure */
  return fgets(inbuf,len,stdin); /* read up to a EOL */
}

void client(int sockfd) {
  int n;
  char sndbuf[MAXIN];
  getreq(sndbuf, MAXIN);        /* prompt */
  while (strlen(sndbuf) > 0) {
    write(sockfd, sndbuf, strlen(sndbuf)); /* send */

    //printf("Wrote message: %s\n",sndbuf);
    
    getreq(sndbuf, MAXIN);                 /* prompt */
  }
}

void sendInitMsg(int sockfd, char *userId) {
	int n;
	n = write(sockfd, userId, strlen(userId)); /* send */
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
	char userId[10];
	memset(userId,0,10);
	fgets(userId,10,stdin);
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
	client(sockfd);

	/* Clean up on termination */
	close(sockfd);
}
