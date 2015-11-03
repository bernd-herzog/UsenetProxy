#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "util.h"
#include "server.h"

#define REMOTE_ADDRESS "2001:4de0:1::1:1"

struct {
	int socket;
} client;

void clientSetup()
{
	//int sock = connect();
}

void clientLoop()
{
	int running = 1;
	while (running == 1)
	{
		printf("connection %d\n", client.socket);
		close(client.socket);
		exit(0);
	}
}

void clientMain(int connectionSocket)
{
	client.socket = connectionSocket;

	clientSetup();
	clientLoop();
}
