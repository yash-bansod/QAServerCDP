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
#include <errno.h>

#define MAXIN 300
#define MAXOUT 300
#define MAXREQ 300
#define MAXQUEUE 10

//Questions
int printRandom(int upper) {
	int num = (rand() % upper);
	return num;
}
struct question {
	char q[300];
	int a;
	char desc[300];
};

struct questiontype {
	struct question qs[10];
	int qnum;
};

struct questiontype qtable[4];

void makeTable() {
	qtable[0].qnum=2;
	qtable[1].qnum=1;
	qtable[2].qnum=2;
	qtable[3].qnum=1;
	strcpy(qtable[0].qs[0].q, "which of the following signals cannot be caught or ignored?1)SIGSEGV 2)SIGQUIT 3)SIGSTOP 4)SIGINT\n");
	strcpy(qtable[0].qs[1].q, "If a signal arrives while the process is executing a system call 1) the system call is interrupted 2) signal is ignored 3) signal is held until the system call is completed 4) none of above\n");
	strcpy(qtable[1].qs[0].q, "To avoid deadlock ____________ 1) there must be a fixed number of resources to allocate, 2) resource allocation must be done only once, 3) all deadlocked processes must be aborted, 4) inversion technique can be used\n");
	strcpy(qtable[2].qs[0].q, "which of the following is shared among the threads of the same process 1)registers 2)heap 3)stack 4)none of the above\n");
	strcpy(qtable[2].qs[1].q, "User level threads are ______ than kernel level threads, 1) Faster, 2) Slower, 3) Same speed, to create and manage the users\n");
	strcpy(qtable[3].qs[0].q, "In hierarchical protection domain also called protection rings, what mode does ring -1 represent? 1) Kernel mode, 2) User Mode, 3) Hypervisor mode, 4) Supervisor Mode\n");
	qtable[0].qs[0].a = 3;
	qtable[0].qs[1].a = 3;
	qtable[1].qs[0].a = 1;
	qtable[2].qs[0].a = 2;
	qtable[2].qs[1].a = 1;
	qtable[3].qs[0].a = 3;
	strcpy(qtable[0].qs[0].desc, " SIGSTOP stops execution and it cannot be caught or ignored\n");
	strcpy(qtable[0].qs[1].desc, " system calls are atomic in nature and signal is held at bay until the system call completes after which the signal is delivered\n");
	strcpy(qtable[1].qs[0].desc, " uncertain number of resources to allocate may result in deadlock\n");
	strcpy(qtable[2].qs[0].desc, " Heap memory is shared among all the threads, and there are seperate sets of registers and stack for different threads\n");
	strcpy(qtable[2].qs[1].desc, " Kernel-level threads are slower to create and manage.\n");
	strcpy(qtable[3].qs[0].desc, " Hypervisor controls hardware access, Ring -1 so that a guest operating system can run Ring 0 operations natively without affecting other guests or the host OS.\n");
	
}

//code
struct userinfo {
	char username[10];
	char ip[10];
	bool isgrp;
	int mode;		//-1:nil
	int qtype;
	int qno;
	char tmid[10];
};
struct userinfo users[5];
int numusers = 0;

char *getreq(char *inbuf, int len) {
  /* Get request char stream */
  printf("REQ: ");              /* prompt */
  memset(inbuf,0,len);          /* clear for good measure */
  return fgets(inbuf,len,stdin); /* read up to a EOL */
}
struct hostent *buildServerAddr(struct sockaddr_in *serv_addr,char *serverIP, int portno) {
  /* Construct an address for remote server */
  memset((char *) serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr->sin_family = AF_INET;
  inet_aton(serverIP, &(serv_addr->sin_addr));
  serv_addr->sin_port = htons(portno);
 }

void client(struct userinfo* user,int sockfd,char* inbuf, char* sndbuf) {
  int n;
  write(sockfd, sndbuf, strlen(sndbuf));
  if(user->isgrp == true) {
	char *serverIP;
	for(int i=0;i<numusers;i++) {
		if(strcmp(users[i].username, user->tmid) == 0) {
			serverIP = users[i].ip;
			break;
		}
	}
	int tmsockfd, portno = 6000;
	struct sockaddr_in serv_addr;
	buildServerAddr(&serv_addr, serverIP, portno);

	/* Create a TCP socket */
	tmsockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* Connect to server on port */
	connect(tmsockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	printf("Connected to %s:%d\n",serverIP, portno);
	write(tmsockfd, sndbuf, strlen(sndbuf));
	if(inbuf[7] == '1') {
		write(tmsockfd, inbuf+9, (int)inbuf[8]);
	}
  }
  //getreq(sndbuf, MAXIN);        /* prompt */
  //while (strlen(sndbuf) > 0) {
  //  write(sockfd, sndbuf, strlen(sndbuf)); /* send */

  //  //printf("Wrote message: %s\n",sndbuf);
  //  
  //  getreq(sndbuf, MAXIN);                 /* prompt */
  //}
}

// Server address


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
	char sndbuf[300];
	memset(sndbuf,0,300);
	printf("Username: %s\nip: %s\nLength: %d\nInput: %c\n",user->username, user->ip, inbuf[8], inbuf[9]);
	if(user->mode == -1) {
		printf("Mode: %d\n",user->mode);
		strcpy(sndbuf,"Welcome ");
		strcat(sndbuf,user->username);
		strcat(sndbuf,"\nSelect Mode:\n1)Individual Mode\n2)Group Mode\n3)Admin Mode");
		client(user,sockfd,inbuf,sndbuf);
		user->mode = 0;
		printf("Mode: %d\n",user->mode);
	}
	else if(user->mode == 0) {
		printf("Mode: %d\n",user->mode);
		if(inbuf[9] == '1') {
			user->mode = 1;
			strcpy(sndbuf, "Select type of question\n");
			strcat(sndbuf, "1)Signals 2)Deadlock 3)Threads 4)Dual Mode of Operation\n");
			client(user,sockfd,inbuf,sndbuf);
		}
		else if(inbuf[9] == '2') {
			strcpy(sndbuf, "Select users\n");
			for(int i=0;i<numusers;i++) {
				char t = i + '0';
				strcat(sndbuf, &t);
				strcat(sndbuf, ") ");
				strcat(sndbuf, users[i].username);
			}
			client(user,sockfd,inbuf,sndbuf);
			user->isgrp = true;
			user->mode = 4;
		}
		else {
			//admin
		}
		printf("Mode: %d\n",user->mode);
	}
	else if(user->mode == 1) {
		printf("Mode: %d\n",user->mode);
		user->mode = 2;
		user->qtype = inbuf[9] - '0';
		//Draw question from type 1
		int num = printRandom(qtable[user->qtype].qnum);
		user->qno = num;
		//set qno
		strcpy(sndbuf, "Question:\n");
		// Display question with 1,2,3,4 choices
		strcat(sndbuf, qtable[user->qtype].qs[num].q);
		client(user,sockfd,inbuf,sndbuf);
		printf("Mode: %d\n",user->mode);
	}
	else if(user->mode == 2) {
		printf("Mode: %d\n",user->mode);
		int ans = inbuf[9] - '0';
		if(ans == qtable[user->qtype].qs[user->qno].a) {
			strcpy(sndbuf, "Correct ");
		}
		else {
			strcpy(sndbuf, "Wrong ");
		}
		//check qno.answer equals 1
		strcat(sndbuf, qtable[user->qtype].qs[user->qno].desc);
		client(user,sockfd,inbuf,sndbuf);
		user->mode = 3;
		strcpy(sndbuf, "\nEnter 'n' for new question, 'q' to quit or 'r' to return to main menu\n");
		//show answer
		//user->mode = 1; // and clear qno
		//strcpy(sndbuf, "Select type of question\n");
		//strcat(sndbuf, "1) 2) 3)..\n");
		client(user,sockfd,inbuf,sndbuf);
		printf("Mode: %d\n",user->mode);
	}
	else if(user->mode == 3) {
		if(inbuf[9] == 'n') {
			user->mode = 1;
			strcpy(sndbuf, "Select type of question\n");
			strcat(sndbuf, "1)Signals 2)Deadlock 3)Threads 4)Dual Mode of Operation\n");
			client(user,sockfd,inbuf,sndbuf);
		}
		else if(inbuf[9] == 'r') {
			user->mode = -1;
			strcpy(sndbuf, "returning to main menu\nPRESS ENTER TO CONTINUE\n");
		client(user,sockfd,inbuf,sndbuf);
		}
		else if(inbuf[9] == 'q') {
			strcpy(sndbuf, "<EXIT>");
			client(user,sockfd,inbuf,sndbuf);
		}
	}
	else if(user->mode == 4) {
		int t = inbuf[9] - '0';
		strcpy(user->tmid, users[t].username);
		user->mode = 0;
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
	makeTable();
int lstnsockfd, portno = 5000;
struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;
int clilen;
int consockfd;
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
   printf("Listening for incoming connections\n");

/* Listen for incoming connections */
   if(listen(lstnsockfd, MAXQUEUE)!=0) {
	   printf("error:%s\n",errno);
   } 

   clilen = sizeof(cli_addr);

/* Accept incoming connection, obtaining a new socket for it */
	consockfd = accept(lstnsockfd, (struct sockaddr *) &cli_addr,&clilen);
	if(consockfd<0) {
	   printf("error:%s\n",errno);
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
