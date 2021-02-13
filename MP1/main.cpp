#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include "loser.h"
using namespace std;

int main(int argc , char **argv)
{
	if (strcmp(argv[1] , "status") == 0)
	{
		chdir(argv[2]);
		__status__();
	}

	if (strcmp(argv[1] , "commit") == 0)
	{
		chdir(argv[2]);
		__commit__();
	}

	if (strcmp(argv[1] , "log") == 0)
	{
		chdir(argv[3]);
		__log__(atoi(argv[2]));
	}

	return 0;
}

