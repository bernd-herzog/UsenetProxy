#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
//#include <sys/sockio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <error.h>
#include <stropts.h>
#include <poll.h>
#include <string.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <net/if.h>  

// 8000 always!
#define ips 8000 
#define cpip 1
#define cpoff 0


	//news.ipv6.eweka.nl.       14400   IN      AAAA    2001:4de0:1::1:1
	//newszilla.ipv6.xs4all.nl. 14400   IN      AAAA    2001:888:0:18::119
	//reader.ipv6.xsnews.nl.    14400   IN      AAAA    2a02:d28:5580:c::101:5 // xsnews mit pw

#define remoteIP "2001:4de0:1::1:1"
#define localPort 2540

int errno;
unsigned char netaddr[4];

int main(int argc, char **argv);
int childMain(int client_sock, int i);
int SetUpServerSocket();
int SetUpClientSocket();
void closeConnection(int news_sock, int client_sock);
void sigTERM();

int g_ssock=0;

void sigTERM(){
	shutdown(g_ssock, SHUT_RDWR);
	close(g_ssock);
	printf("Socket closed on SIGTERM\n");
}
void sigINT(){
	shutdown(g_ssock, SHUT_RDWR);
	close(g_ssock);
	printf("Socket closed on SIGINT\n");
}

int main(int argc, char **argv)
{
	signal(SIGTERM, (void*)sigTERM);
	signal(SIGINT, (void*)sigINT);

	int sock;
	if ((g_ssock = sock = SetUpServerSocket()) <= 0){
		return 1;
	}

	int i;
	for (i = 0;; i = (i+1)%(ips*cpip)){
		
		int client_sock;
		if ((client_sock = accept(sock, NULL, NULL)) == -1){
			perror("accept");
			return 1;
		}

		// double fork
		int child;
		if ((child = fork()) == -1) {
  			printf("fork error %d\n", errno);
 			return 1;
 		}
 		if (child != 0) { /* this is the parent */
			waitpid(child, NULL, 0);
			close(client_sock);
 		} else {  /* the child */
			int gchild;
			if ((gchild = fork()) == -1) {
  				printf("fork error %d\n", errno);
 				return 1;
 			}
 			if (gchild != 0) { /* this is the child */
				return 0;
 			} else {  /* the grandchild */
				return childMain(client_sock, i);
			}
 		}
	}
}

int SetUpServerSocket()
{
	struct sockaddr_in local_addr4;
	memset( &local_addr4, 0, sizeof(local_addr4));
	//local_addr4.sin_len 	= sizeof(local_addr4);
	local_addr4.sin_family 	= AF_INET;
	local_addr4.sin_port 	= htons(localPort);
	local_addr4.sin_addr.s_addr = inet_addr("10.0.0.1");

	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0){
		perror("sock");
		return -1;
	}
	
	if (bind (sock, (struct sockaddr*)&local_addr4, sizeof(local_addr4)) < 0){
		perror("bind");
		printf("port busy (%d)?, waiting ", errno);
		while (bind (sock, (struct sockaddr*)&local_addr4, sizeof(local_addr4)) < 0){
			printf(".");
			fflush(stdout);
			usleep(500000);
		}
	}
		
	if (listen(sock, 300) < 0){
		perror("listen");
		return -1;
	}

	struct ifreq ifr;
	strcpy(ifr.ifr_name, "ppp0");
	if(ioctl(sock, SIOCGIFADDR, (caddr_t)&ifr, sizeof(struct ifreq)) < 0){ // normalerweise sock, hier aber nich, geht das?
		perror("ioctl");
		return -1;
	}

	memcpy(netaddr, &((struct sockaddr_in *)(&(ifr.ifr_addr)))->sin_addr, 4);

	return sock;
}

int SetUpClientSocket(int i)
{
	int news_sock;
	if ((news_sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) <= 0){
		perror("socket");
		return -1;
	}
	
	struct sockaddr_in6 local_addr6;
	memset( &local_addr6, 0, sizeof(local_addr6));
	//local_addr6.sin6_len 	= sizeof(local_addr6);
	local_addr6.sin6_family = AF_INET6;
	//local_addr6.sin6_port 	= htons(0);
	
	//int iaddr = inet_addr(argv[1]);
	
	char buf[80];
	sprintf(buf, "2002:%0.2x%0.2x:%0.2x%0.2x::2537:%d", netaddr[0], netaddr[1], netaddr[2], netaddr[3], (i/cpip)+1+cpoff);
	printf("%s\n", buf);
	inet_pton(AF_INET6, buf, local_addr6.sin6_addr.s6_addr);
	
	if (bind (news_sock, (struct sockaddr*)&local_addr6, sizeof(local_addr6)) < 0){
		perror("bind");
		return -1;
	}
	
	struct sockaddr_in6 news_addr;
	memset( &news_addr, 0, sizeof(news_addr));
	//news_addr.sin6_len 		= sizeof(news_addr);
	news_addr.sin6_family 	= AF_INET6;
//	news_addr.sin6_flowinfo = 0;
	news_addr.sin6_port 	= htons(119);

	inet_pton(AF_INET6, remoteIP, news_addr.sin6_addr.s6_addr); 

	if (connect( news_sock, (struct sockaddr*)&news_addr, sizeof(news_addr)) < 0) {
		perror("connect");
		return -1;
	}

	return news_sock;
}

int childMain(int client_sock, int i)
{
	//connect each other

	int news_sock;
	if ((news_sock = SetUpClientSocket(i)) <= 0){
		return 1;
	}
	
	//printf("connectet\n");
	struct pollfd fds[2];
	fds[0].fd = news_sock;
	fds[0].events = POLLIN;
	fds[1].fd = client_sock;
	fds[1].events = POLLIN;
	
	char nbuf[64*1024]; // maximale gröeines packets
	int len;
	while (1)
	{
		if (poll(fds, 2, -1) < 0){
			perror("poll");
			closeConnection(news_sock, client_sock);
			return 1;
		}

		if (fds[0].revents & POLLIN){
			// daten von news server
			if ((len = recv(news_sock, nbuf, 64*1024, 0)) <= 0){
				closeConnection(news_sock, client_sock);
				return 1;
			}
			if ((send (client_sock, nbuf, len, 0)) != len){
				closeConnection(news_sock, client_sock);
				return 1;
			}
		}

		if (fds[1].revents & POLLIN){
			// daten von news client
			if ((len = recv(client_sock, nbuf, 64*1024, 0)) <= 0){
				closeConnection(news_sock, client_sock);
				return 1;
			}
			if (send (news_sock, nbuf, len, 0) != len){
				closeConnection(news_sock, client_sock);
				return 1;
			}
		}

		if (fds[0].revents & POLLHUP || fds[1].revents & POLLHUP){
			closeConnection(news_sock, client_sock);
			return 0;
		}
	}
	
	return;
}

void closeConnection(int news_sock, int client_sock){
	shutdown(news_sock, SHUT_RDWR);
	close(news_sock);
	shutdown(client_sock, SHUT_RDWR);
	close(client_sock);
	//printf("closed a connection\n");
}



