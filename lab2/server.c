#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/time.h>

#define MAXLEN 512
#define STDIN 0

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

int main (int argc, char *argv[]) {
	char ch;
	char tcpport[30] = "1234";
	char udpport[30] = "1234";
	char popis[MAXLEN];
	char naredba[100];
	fd_set readfds, writefds;
	
	//parsiraj ulaz
	while((ch = getopt(argc, argv, "t:u:p:")) != -1) {
		switch(ch) {
			case 't':
				memset(&tcpport, 0, 30);
				strcpy(tcpport, optarg);
				break;
			case 'u':
				memset(&udpport, 0, 30);
				strcpy(udpport, optarg);
				break;
			case 'p':
				strcpy(popis, optarg);
				break;
			default:
				fprintf(stderr, "Usage: ./server [-t tcp_port] [-u udp_port] [-p popis]\n");
				exit(1);
		}
	}
	
	int udpsock, tcpsock, newsock, n;
	struct addrinfo hints, *res, *rest;
	struct sockaddr_in botaddr;
	int error, numbytes;
	socklen_t botlen;
	char buf[MAXLEN];
	struct timeval tv;
	tv.tv_sec = 100;
	tv.tv_usec = 0;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; 

	memset(buf, 0, MAXLEN);
	
	strcpy(popis, strcat(popis, "\n\0"));
	
	//udp
	if ((error = getaddrinfo(NULL, udpport, &hints, &res)) != 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(1);
	}
	
	udpsock = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	Bind(udpsock, res->ai_addr, res->ai_addrlen);
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 
	
	//tcp
	if ((error = getaddrinfo(NULL, tcpport, &hints, &rest)) != 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(1);
	}
	
	tcpsock = Socket(rest->ai_family, rest->ai_socktype, rest->ai_protocol);
	Bind(tcpsock, rest->ai_addr, rest->ai_addrlen);
	
	if(listen(tcpsock, 1) == -1) {
		perror("Error on listen!\n");
		exit(1);
	}	
	
	while(1) {
		//select
		FD_ZERO (&readfds);
		FD_ZERO (&writefds);
		FD_SET(STDIN, &readfds);
		FD_SET(udpsock, &readfds);
		FD_SET(tcpsock, &readfds);
		FD_SET(udpsock, &writefds);
		FD_SET(tcpsock, &writefds);
		n = tcpsock + 1;
		error = select(n, &readfds, &writefds, NULL, &tv);
		if (error==-1) {
			perror("Error on select!\n");
		} else if (error==0) {
			printf("Timeout occurred!\n");
		} else {
			//received data
			if (FD_ISSET(udpsock, &readfds)) {
				//podatak s udp socketa
				botlen = sizeof(botaddr);
				numbytes = recvfrom(udpsock, buf, MAXLEN, 0, (struct sockaddr *)&botaddr, &botlen);
				if(strcmp(buf, "HELLO\n")==0) {
					sendto(udpsock, popis, strlen(popis), 0, (struct sockaddr *)&botaddr, botlen);		
				}
			} 
			if (FD_ISSET(tcpsock, &readfds)) {
				//podatak s tcp socketa
				botlen = sizeof(botaddr);
				newsock = accept(tcpsock, (struct sockaddr*)&botaddr, &botlen);
				if(newsock==-1) {
					perror("Error on accept!\n");
				}
				numbytes = recv(newsock, buf, MAXLEN, 0);
				if (numbytes==-1) {
					perror("Error on recv!\n");
				}
				if(strcmp(buf, "HELLO\n")==0) {
					send(newsock, popis, strlen(popis), 0);		
				}
			}
			if (FD_ISSET(STDIN, &readfds)) {
				//podatak sa stdina
				if(scanf("%s",naredba)<0) {
					perror("Error on scanf!\n");
				} else if (strcmp(naredba, "PRINT")==0) {
					printf("Trenutni popis payloadova: %s\n", popis);
					memset(naredba, 0, 100);
				} else if (strncmp(naredba, "SET", 3)==0) {	
					memset(naredba, 0, 100);
					memset(popis, 0, MAXLEN);
					scanf("%s", popis);	
				} else if (strcmp(naredba, "QUIT")==0) {
					return 0;
				} else {
					printf("Kriva naredba!\n");
				}
			}
		}
	}
}
	
	
	
