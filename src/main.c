#include <stdio.h>
#include <stdlib.h>

void usage(char *name)
{
	printf("USAGE:\n\t%s s [port]\n\t%s c [ip] [port]\n", name, name);
	printf("s: server, c: client\n");
}
int main(int argc, char **argv)
{
	if(argc != 3 || argc!= 4){
		usage(argv[0]);
		exit(0);
	}


	
	return 0;
}
