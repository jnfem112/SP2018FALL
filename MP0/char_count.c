#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMBER_OF_CHARACTER 128

int main(int argc , char **argv)
{
	int record[NUMBER_OF_CHARACTER] = {0};
	FILE *file;
	char input;
	int count = 0;

	int length = strlen(argv[1]);
	for (int i = 0 ; i < length ; i++)
		record[argv[1][i]] = 1;

	if (argc < 3)
	{
		while ((input = getchar()) != EOF)
		{
			if (record[input])
				count++;

			if (input == '\n')
			{
				printf("%c\n", count);;
				count = 0;
			}
		}
	}
	else
	{
		file = fopen(argv[2] , "r");

		if (file == NULL)
		{
			fprintf(stderr , "error\n");
			exit(EXIT_FAILURE);
		}
		else
		{
			while ((input = fgetc(file)) != EOF)
			{
				if (record[input])
					count++;

				if (input == '\n')
				{
					printf("%c\n", count);;
					count = 0;
				}
			}
		}

		fclose(file);
	}

	exit(EXIT_SUCCESS);
}
