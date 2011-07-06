/* main.c
main file of "chat".
*/



/*the include that ties this all together */
#include "chat.h"

void usage(char **argv)
{
	printf("USAGE: %s s [port]\n", argv[0]);
	printf("       %s c [server] [port]\n", argv[0]);
	printf("       s = server mode\n");
	printf("       c = client mode\n");
	
	exit(RETURN_USAGE);
}



int main(int argc, char **argv)
{
	if(argc == 3 && !strcmp(argv[1], "s"))
	{
		/* server mode, 
		argv[2] == port to listen on */
		server(argv[2]);
	}
	else
	if(argc == 4 && !strcmp(argv[1], "c"))
	{
		/*client mode, 
		argv[2] == server to connect to
		argv[3] == port to connect on
		*/
		client(argv[2], argv[3]);
	}
	else
	{
		usage(argv);
	}
	
	//i don't think we ever actually get here... whatever
	return(RETURN_SUCCESS);
}


