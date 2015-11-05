#include <stdio.h>
#include <error.h>
#include <stdlib.h>

#include "util.h"

int errno;

void doubleFork(void (*childFunc)())
{
	printf("doubleFork\n");

	int pid = fork();

	if (pid == -1)
	{
		printf("fork error %d\n", errno);
		exit(0);
	}

	if (pid != 0)
	{
		printf("doubleFork p\n");

		return;
	}

	if (pid == 0)
	{
		printf("doubleFork c\n");

		int gpid = fork();

		if (gpid == -1)
		{
			printf("fork error %d\n", errno);
			exit(0);
		}

		if (gpid != 0)
		{
			printf("doubleFork c:p\n");
			exit(0);
		}

		if (gpid == 0)
		{
			printf("doubleFork c:c\n");
			childFunc();
			exit(0);
		}
	}
}
