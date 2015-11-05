#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <poll.h>

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
	clientSetup(connectionSocket);
	clientLoop();
	clientShutdown();
}

void clientSetup(int socket)
{
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
	if ((client.remoteSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) <= 0)
	{
		perror("socket");
		exit(0);
	}
}

void getUniqueAddress()
{
	struct ifreq ifr;

	ifr.ifr_addr.sa_family = AF_INET6;
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

	ioctl(client.socket, SIOCGIFADDR, &ifr);
	ioctl(client.socket, SIOCGIFNETMASK, &ifr);

	struct sockaddr_in6 *ipv6Addr = (struct sockaddr_in6 *) &ifr.ifr_addr;
	struct sockaddr_in6 *ipv6NetMask = (struct sockaddr_in6 *) &ifr.ifr_netmask;

	//ipv6Addr->sin6_addr.s6_addr;

	printf("ip: %s\n", inet_ntoa(ipv6Addr->sin6_addr));
	printf("net: %s\n", inet_ntoa(ipv6NetMask->sin6_addr));

	char buf[80];
	sprintf(buf, "::1");
	inet_pton(AF_INET6, buf, client.bindAddress);
}

void bindRemoteSocket()
{
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
	shutdown(client.socket, SHUT_RDWR);
	shutdown(client.remoteSocket, SHUT_RDWR);

	close(client.socket);
	close(client.remoteSocket);

	removeUniqueAddress();
}

void removeUniqueAddress()
{

}
