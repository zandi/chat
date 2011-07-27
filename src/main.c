#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>


void usage(char *name)
{
	printf("USAGE:\n\t%s s [port]\n\t%s c [ip] [port]\n", name, name);
	printf("s: server mode, c: client mode\n");

	return;
}


int main(int argc, char **argv)
{
	int port, s, sfd;
	struct addrinfo hints;
	struct addrinfo *res=0, *rp=0;


	if(argc == 3 || argc == 4){
		if(toupper((int)(*argv[1])) == 'C'){

			/*client mode*/

			port = atoi(argv[3]);
			if(port >= 0  || port < 65536){

				/*for reference, read the example client from "man getaddrinfo"*/

				memset(&hints, 0, sizeof(struct addrinfo));
				hints.ai_family = AF_UNSPEC; /*ipv4 or ipv6, don't care*/
				hints.ai_socktype = SOCK_STREAM; /*stream sockets only*/
				
				s = getaddrinfo(argv[2], argv[3], &hints, &res);
				if(s != 0)
				{
					printf("getaddrinfo: %s\n", gai_strerror(s));
					exit(-1);
				}

				/*try to connect*/

				for(rp = res; rp != NULL; rp = rp->ai_next)
				{
					sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

					if(sfd == -1)
						continue; /*some error, try the next entry*/

					if(connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
						break; /*success!*/

					close(sfd); /*something's wrong, just try the next one*/
				}
				freeaddrinfo(res); /*connected or not, we don't need this anymore*/

				if(rp == NULL){
					printf("could not connect.\n");
					exit(-1);
				}

				/*TODO: call client() function here*/
				printf("entering client()\n");
			}
			else{
				printf("error: [port] must be an integer between 0 and 65535, inclusive\n");
				exit(-1);
			}
		}
		else
		if(toupper((int)(*argv[1])) == 'S'){
			/*likewise, here argv[2] must be a valid port number*/
			port = atoi(argv[2]);
			if(port >= 0  || port < 65536){

				/* server mode*/
				memset(&hints, 0, sizeof(struct addrinfo));
				hints.ai_family = AF_UNSPEC; /*ipv4 or ipv6, don't care*/
				hints.ai_socktype = SOCK_STREAM; /*stream sockets only*/
				hints.ai_flags = AI_PASSIVE; /*wildcard IP*/
				
				s = getaddrinfo(NULL, argv[2], &hints, &res);
				if(s != 0)
				{
					printf("getaddrinfo: %s\n", gai_strerror(s));
					exit(-1);
				}

				/*try to connect*/

				for(rp = res; rp != NULL; rp = rp->ai_next)
				{
					sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

					if(sfd == -1)
						continue; /*some error, try the next entry*/

					if(bind(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
						break; /*success!*/

					close(sfd); /*something's wrong, just try the next one*/
				}
				freeaddrinfo(res); /*connected or not, we don't need this anymore*/

				if(rp == NULL){ /*we couldn't bind for some reason*/
					printf("could not bind.\n");
					exit(-1);
				}

				/*and now we enter the server code*/
				printf("entering server()\n");


			}
			else{
				printf("error: [port] must be an integer between 0 and 65535, inclusive\n");
				exit(-1);
			}
		}
	}
	usage(argv[0]);
	return 0;
}
