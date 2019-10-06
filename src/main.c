#include <stdio.h>

int main (int argc, char* argv[])
{
	printf("Hello img4helper!\n");

	for (int i = 0; i < argc; i++) {
		printf("[%d]: %s\n", i, argv[i]);
	}

	return 0;
}
