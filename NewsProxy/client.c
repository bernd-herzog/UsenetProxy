#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "client.h"
#include "util.h"
#include "server.h"

#define REMOTE_ADDRESS "2001:4de0:1::1:1"

struct {
	int socket;
	int remoteSocket;
	unsigned char bindAddress[16];
} client;

void clientSetup();
void clientLoop();
void createRemoteSocket();
void bindRemoteSocket();
void connectRemoteSocket();

void clientMain(int connectionSocket)
{
	clientSetup(connectionSocket);
	clientLoop();
}

void clientSetup(int socket)
{
	//TODO: close on disconnect/exit
	client.socket = socket;

	createRemoteSocket();
	bindRemoteSocket();
	connectRemoteSocket();
}

void createRemoteSocket()
{
	if ((client.remoteSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) <= 0)
	{
		perror("socket");
	}
}

void getUniqueAddress()
{
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

	//TODO: close on disconnect/exit
	if (connect(client.remoteSocket, (struct sockaddr*)&news_addr, sizeof(news_addr)) < 0)
	{
		perror("connect");
	}
}

void clientLoop()
{
	int running = 1;
	while (running == 1)
	{
		printf("connection %d\n", client.socket);
		//TODO: implement forwarding loop
		close(client.socket);
		exit(0);
	}
}
