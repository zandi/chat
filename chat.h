/* chat.h
various #defines and prototype functions

*/

/*various defines*/
#define MAX_MOTD_SIZE 800
#define MAX_MESSAGE_SIZE 800
#define MAX_LINE_SIZE 80
#define MAX_USERNAME_SIZE 12
#define QUIT "/quit"
#define DEFAULT_MOTD "this is the default message of the day.\n\0"

/*exit value codes*/
#define RETURN_SUCCESS 0
#define RETURN_USAGE 1
#define RETURN_GETADDRINFO_ERROR 2
#define RETURN_SOCKET_ERROR 3
#define RETURN_CONNECT_ERROR 4
#define RETURN_BIND_ERROR 5
#define RETURN_LISTEN_ERROR 6
#define RETURN_ACCEPT_ERROR 7
#define RETURN_SEND_ERROR 8
#define RETURN_RECV_ERROR 9
#define RETURN_FGETS_ERROR 10

/* various includes */
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>



void server(const char *PORT);
void client(const char *SERVER_STRING, const char *PORT);
