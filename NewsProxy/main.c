#include <stdio.h>
#include <stdlib.h>

#include "server.h"

#include "client.h"

int main(int argc, char **argv)
{
	client_test();
	exit(0);

	printf("main\n");

	serverSetup();
	serverLoop();
}
