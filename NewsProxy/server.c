#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <error.h>

#include "server.h"
#include "client.h"

#define LISTEN_ADDRESS "0.0.0.0"
#define LISTEN_PORT 119

int errno;

struct {
	int socket;
	int connection;
} server;

void setupListenSocket()
{
	struct sockaddr_in local_addr4;
	memset( &local_addr4, 0, sizeof(local_addr4));
	//local_addr4.sin_len 	= sizeof(local_addr4);
	local_addr4.sin_family 	= AF_INET;
	local_addr4.sin_port 	= htons(LISTEN_PORT);
	local_addr4.sin_addr.s_addr = inet_addr(LISTEN_ADDRESS);

	if ((server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0){
		perror("sock");
		return;
	}

	if (bind (server.socket, (struct sockaddr*)&local_addr4, sizeof(local_addr4)) < 0){
		perror("bind");
		printf("port busy (%d)?, waiting ", errno);
		while (bind (server.socket, (struct sockaddr*)&local_addr4, sizeof(local_addr4)) < 0){
			printf(".");
			fflush(stdout);
			usleep(500000);
		}
	}
}

void serverSetup()
{
	setupListenSocket();

	if (listen(server.socket, 300) < 0){
		perror("listen");
		return;
	}
}

void clientFunc()
{
	close(server.socket);

	clientMain(server.connection);
}

void serverLoop()
{
	int running = 1;
	while (running == 1)
	{
		if ((server.connection = accept(server.socket, NULL, NULL)) == -1){
			perror("accept");
			return;
		}

		doubleFork(clientFunc);
		close(server.connection);
	}
}