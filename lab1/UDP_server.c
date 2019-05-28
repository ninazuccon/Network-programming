#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>

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
	char myport[30] = "1234";
	int sockfd;
	struct addrinfo hints, *res;
	struct sockaddr_in botaddr;
	int error, numbytes;
	socklen_t botlen;
	char msg[MAXLEN];
	char fullMsg[MAXLEN+10];
	char buf[MAXLEN];
	char prefix[10];
	strcpy(prefix, "PAYLOAD:");
	char ch;
			
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; 

	memset(buf, 0, MAXLEN);
	memset(msg, 0, MAXLEN);
		
	//provjera argumenata
	while((ch = getopt(argc, argv, "l:p:")) != -1) {
		switch(ch) {
			case 'l':
				memset(&myport, 0, 30);
				strcpy(myport, optarg);
				break;
			case 'p':
				strcpy(msg, optarg);
			break;
			default:
				fprintf(stderr, "Usage: ./UDP_server [-l port] [-p payload]\n");
				exit(1);
		}
	}
	
	if ((error = getaddrinfo(NULL, myport, &hints, &res)) != 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(1);
	}
	
	sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	Bind(sockfd, res->ai_addr, res->ai_addrlen);
	
	//prefiks + poruka
	strcpy(fullMsg, strcat(prefix, msg));
	strcat(fullMsg, "\n\0");
	
	while (1) {
		botlen = sizeof(botaddr);
		numbytes = recvfrom(sockfd, buf, MAXLEN, 0, (struct sockaddr *)&botaddr, &botlen);
		if (numbytes<0) {
			fprintf(stderr, "Error: %s\n", strerror(errno));
			exit(1);
		}
		if(strcmp(buf, "HELLO\n")==0) {
			sendto(sockfd, fullMsg, MAXLEN, 0, (struct sockaddr *)&botaddr, botlen);		
		}
	}
}
