#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <error.h>
#include <stdlib.h>

#include "server.h"
#include "client.h"

#define LISTEN_ADDRESS "0.0.0.0"
#define LISTEN_PORT 119

int errno;

struct {
	int socket;
	int connection;
} server;

void serverSetup();
void createListenSocket();
void bindListenSocket();
void listenListenSocket();
void serverLoop();
void clientFunc();

void serverMain()
{
	printf("serverMain\n");

	serverSetup();
	serverLoop();
}

void serverSetup()
{
	printf("serverSetup\n");

	createListenSocket();
	bindListenSocket();
	listenListenSocket();
}

void createListenSocket()
{
	printf("createListenSocket\n");

	if ((server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)
	{
		perror("sock");
		exit(0);
	}
}

void bindListenSocket()
{
	printf("bindListenSocket\n");

	struct sockaddr_in local_addr4;
	memset( &local_addr4, 0, sizeof(local_addr4));
	//local_addr4.sin_len 	= sizeof(local_addr4);
	local_addr4.sin_family 	= AF_INET;
	local_addr4.sin_port 	= htons(LISTEN_PORT);
	local_addr4.sin_addr.s_addr = inet_addr(LISTEN_ADDRESS);

	if (bind (server.socket, (struct sockaddr*)&local_addr4, sizeof(local_addr4)) < 0)
	{
		perror("bind");
		printf("port busy (%d)?, waiting ", errno);
		while (bind (server.socket, (struct sockaddr*)&local_addr4, sizeof(local_addr4)) < 0){
			printf(".");
			fflush(stdout);
			usleep(500000);
		}
	}
}

void listenListenSocket()
{
	printf("listenListenSocket\n");

	if (listen(server.socket, 300) < 0)
	{
		perror("listen");
		exit(0);
	}
}

void serverLoop()
{
	printf("serverLoop\n");

	int running = 1;
	while (running == 1)
	{
		if ((server.connection = accept(server.socket, NULL, NULL)) == -1){
			perror("accept");
			exit(0);
		}

		doubleFork(clientFunc);
		close(server.connection);
	}
}

void clientFunc()
{
	printf("clientFunc\n");

	close(server.socket);
	clientMain(server.connection);
}
