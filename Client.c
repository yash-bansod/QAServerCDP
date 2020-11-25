// Client.c
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

#define MAXIN 300
#define MAXOUT 300
char *serverIP = "14.139.34.11";

int flag=0;
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

char *getreq(char *inbuf, int len) {
  /* Get request char stream */
  memset(inbuf,0,len);          /* clear for good measure */
  fgets(inbuf,len,stdin); /* read up to a EOL */
	return inbuf;
}
struct hostent *buildServerAddr(struct sockaddr_in *serv_addr,
	char *serverIP, int portno) {
  /* Construct an address for remote server */
  memset((char *) serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr->sin_family = AF_INET;
  inet_aton(serverIP, &(serv_addr->sin_addr));
  serv_addr->sin_port = htons(portno);
 }


void client(int sockfd, char *userId) {
  int n;
  char sndbuf[MAXIN]; 
  char rcvbuf[MAXOUT];
  if(flag==0) {
	getreq(sndbuf, MAXIN);        /* prompt */
  memset(sndbuf, 0, MAXIN);
  //printf("This is buff %s\n", sndbuf);
  //while (strlen(sndbuf) > 0) {
	strcpy(sndbuf, makeHeader(sndbuf,userId));
    write(sockfd, sndbuf, strlen(sndbuf)); /* send */

    //printf("Wrote message: %s\n",sndbuf);
    
    memset(rcvbuf,0,MAXOUT);               /* clear */
    n=read(sockfd, rcvbuf, MAXOUT-1);      /* receive */
    //printf("Received reply: %d",n);
    
    write(STDOUT_FILENO, rcvbuf, n);	      /* echo */
	if(strcmp(rcvbuf,"<EXIT>")==0) {
		exit(0);
	}
  }
  close(sockfd);
    getreq(sndbuf, MAXIN);                 /* prompt */
	int portno = 5000;
	struct sockaddr_in serv_addr;
	buildServerAddr(&serv_addr, serverIP, portno);

	/* Create a TCP socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	strcpy(sndbuf, makeHeader(sndbuf,userId));
    write(sockfd, sndbuf, strlen(sndbuf)); /* send */

    //printf("Wrote message: %s\n",sndbuf);
    
    memset(rcvbuf,0,MAXOUT);               /* clear */
    n=read(sockfd, rcvbuf, MAXOUT-1);      /* receive */
    //printf("Received reply: %d",n);
    
    write(STDOUT_FILENO, rcvbuf, n);	      /* echo */
	if(strcmp(rcvbuf,"<EXIT>")==0) {
		exit(0);
	}
  //}
}

// Server address
void sendInitMsg(int sockfd, char *userId) {
	char header[12];
	memset(header, 0, 12);
	strcpy(header,userId);
	strcat(header,"2");
	int *tmp = (int*)&header[8];
	tmp[0] = 1;
	int n;
	n = write(sockfd, header, strlen(header)); /* send */
	n = read(sockfd, header, strlen(header)); /* send */
}

void * thrdRead(void * sockfd) {
	int *consockfd = (int *)sockfd;
	char buf[MAXIN];
	memset(buf, 0, MAXIN);
	while(1) {
		read(*consockfd, buf, MAXIN);
	}
}

char userId[8];
int main() {
	//Client protocol
	while(1) {
	if(flag==0) {
		printf("Enter your userid\nNote: User id should be 7 letters exact\n> ");
		memset(userId,0,8);
		fgets(userId,8,stdin);
		//flag=1;
	}
	int sockfd, portno = 5000;
	struct sockaddr_in serv_addr;
	buildServerAddr(&serv_addr, serverIP, portno);

	/* Create a TCP socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* Connect to server on port */
	connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
//	if(flag==0) {
//		pthread_t thrd;
//		pthread_create(&thrd, NULL, thrdRead, (void *)&sockfd);
//	}
	//printf("Connected to %s:%d\n",serverIP, portno);
	/* Carry out Client-Server protocol */
	//sendInitMsg(sockfd, userId);
	client(sockfd, userId);

	/* Clean up on termination */
	close(sockfd);
	flag=1;
	}
}
