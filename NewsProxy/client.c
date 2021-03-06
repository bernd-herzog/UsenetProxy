#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <poll.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include "client.h"
#include "util.h"
#include "server.h"

#define REMOTE_ADDRESS "2001:4de0:1::1:1"

struct {
	int socket;
	int remoteSocket;
	unsigned char bindAddress[16];
	struct pollfd pollFileDescriptors[2];
} client;

void clientSetup();
void clientLoop();
void createRemoteSocket();
void bindRemoteSocket();
void connectRemoteSocket();
void clientShutdown();
void forwardPacket(int src, int dst);
void removeUniqueAddress();

void clientMain(int connectionSocket)
{
	printf("clientMain\n");

	clientSetup(connectionSocket);
	clientLoop();
	clientShutdown();
}

void clientSetup(int socket)
{
	printf("clientSetup\n");

	client.socket = socket;

	createRemoteSocket();
	bindRemoteSocket();
	connectRemoteSocket();

	client.pollFileDescriptors[0].fd = client.remoteSocket;
	client.pollFileDescriptors[0].events = POLLIN;
	client.pollFileDescriptors[1].fd = client.socket;
	client.pollFileDescriptors[1].events = POLLIN;
}

void createRemoteSocket()
{
	printf("createRemoteSocket\n");

	if ((client.remoteSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) <= 0)
	{
		perror("socket");
		exit(0);
	}
}



void printIpv6(struct in6_addr *addr) {
   printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                 (int)addr->s6_addr[0], (int)addr->s6_addr[1],
                 (int)addr->s6_addr[2], (int)addr->s6_addr[3],
                 (int)addr->s6_addr[4], (int)addr->s6_addr[5],
                 (int)addr->s6_addr[6], (int)addr->s6_addr[7],
                 (int)addr->s6_addr[8], (int)addr->s6_addr[9],
                 (int)addr->s6_addr[10], (int)addr->s6_addr[11],
                 (int)addr->s6_addr[12], (int)addr->s6_addr[13],
                 (int)addr->s6_addr[14], (int)addr->s6_addr[15]);
}



void getUniqueAddress()
{
	printf("getUniqueAddress\n");

	struct ifaddrs *sifaddrs;
    int count = getifaddrs(&sifaddrs);

    for (;sifaddrs != NULL; sifaddrs = sifaddrs->ifa_next)
    {
    	if(sifaddrs->ifa_addr->sa_family==AF_INET) {
			printf("IPv4 ");
    		struct sockaddr_in *saddr = (struct sockaddr_in *) sifaddrs->ifa_addr;

    	    char addressBuffer[INET6_ADDRSTRLEN];

    		inet_ntop(
    				AF_INET,
    				&saddr->sin_addr.s_addr,
    				addressBuffer,
					sizeof(addressBuffer));

    		printf("%s\n", addressBuffer);


		} else if(sifaddrs->ifa_addr->sa_family==AF_INET6) {
			printf("IPv6 ");
    		struct sockaddr_in6 *saddr = (struct sockaddr_in6 *) sifaddrs->ifa_addr;

			printIpv6(&saddr->sin6_addr);

		} else {

		}
    }

    freeifaddrs(sifaddrs);

	exit(0);

	struct ifreq ifr;

	ifr.ifr_addr.sa_family = AF_INET6;
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
	printf("getUniqueAddress2\n");

	ioctl(client.socket, SIOCGIFADDR, &ifr);
	struct sockaddr_in6 *ipv6Addr = (struct sockaddr_in6 *) &ifr.ifr_addr;
	printIpv6(&ipv6Addr->sin6_addr);

	ioctl(client.socket, SIOCGIFNETMASK, &ifr);
	struct sockaddr_in6 *ipv6NetMask = (struct sockaddr_in6 *) &ifr.ifr_netmask;
	printIpv6(&ipv6NetMask->sin6_addr);

	char buf[80];
	sprintf(buf, "::1");
	inet_pton(AF_INET6, buf, client.bindAddress);
}

void client_test()
{
	getUniqueAddress();

}

void bindRemoteSocket()
{
	printf("bindRemoteSocket\n");

	struct sockaddr_in6 local_addr6;
	memset( &local_addr6, 0, sizeof(local_addr6));
	//local_addr6.sin6_len 	= sizeof(local_addr6);
	local_addr6.sin6_family = AF_INET6;

	getUniqueAddress();

	memcpy(local_addr6.sin6_addr.s6_addr, client.bindAddress, sizeof(client.bindAddress));

	if (bind (client.remoteSocket, (struct sockaddr*)&local_addr6, sizeof(local_addr6)) < 0)
	{
		perror("bind");
		exit(0);
	}
}

void connectRemoteSocket()
{
	printf("connectRemoteSocket\n");

	struct sockaddr_in6 news_addr;
	memset( &news_addr, 0, sizeof(news_addr));
	//news_addr.sin6_len 		= sizeof(news_addr);
	news_addr.sin6_family 	= AF_INET6;
	news_addr.sin6_port 	= htons(119);

	inet_pton(AF_INET6, REMOTE_ADDRESS, news_addr.sin6_addr.s6_addr);

	if (connect(client.remoteSocket, (struct sockaddr *)&news_addr, sizeof(news_addr)) < 0)
	{
		perror("connect");
		exit(0);
	}
}

void clientLoop()
{
	printf("clientLoop\n");

	printf("connection %d\n", client.socket);

	int running = 1;
	while (running == 1)
	{
		if (poll(client.pollFileDescriptors, 2, -1) < 0)
		{
			perror("poll");
			exit(0);
		}

		if (client.pollFileDescriptors[0].revents & POLLHUP || client.pollFileDescriptors[1].revents & POLLHUP){
			return;
		}

		if (client.pollFileDescriptors[0].revents & POLLIN)
		{
			forwardPacket(client.remoteSocket, client.socket);
		}

		if (client.pollFileDescriptors[1].revents & POLLIN)
		{
			forwardPacket(client.socket, client.remoteSocket);
		}
	}
}

void forwardPacket(int src, int dst)
{
	printf("forwardPacket\n");

	int len;
	char nbuf[64*1024];

	if ((len = recv(src, nbuf, 64*1024, 0)) <= 0)
	{
		perror("recv");
		exit(0);
	}

	if (send (dst, nbuf, len, 0) != len)
	{
		perror("send");
		exit(0);
	}
}

void clientShutdown()
{
	printf("clientShutdown\n");

	shutdown(client.socket, SHUT_RDWR);
	shutdown(client.remoteSocket, SHUT_RDWR);

	close(client.socket);
	close(client.remoteSocket);

	removeUniqueAddress();
}

void removeUniqueAddress()
{
	printf("removeUniqueAddress\n");

}
