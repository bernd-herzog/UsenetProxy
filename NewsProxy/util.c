#include <stdio.h>
#include <error.h>
#include <stdlib.h>

#include "util.h"

int errno;

void doubleFork(void (*childFunc)())
{
	int pid = fork();

	if (pid == -1)
	{
		printf("fork error %d\n", errno);
		return;
	}

	if (pid != 0)
	{
		return;
	}

	if (pid == 0)
	{
		int gpid = fork();

		if (gpid == -1)
		{
			printf("fork error %d\n", errno);
			return;
		}

		if (gpid != 0)
		{
			return;
		}

		if (gpid == 0)
		{
			childFunc();
			exit(0);
			return;
		}
	}
}