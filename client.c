/* client.c
client for the "chat" program


*/
#include "chat.h"

void client(const char *SERVER_STRING, const char *PORT)
{
	int status, sockfd, sent, received, i, j;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	char username[MAX_USERNAME_SIZE];
	char message[MAX_USERNAME_SIZE + MAX_LINE_SIZE + 4];
	char line[MAX_LINE_SIZE];
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	
	/* get the user's username */
	printf("username: ");
	if(fgets(username, MAX_USERNAME_SIZE, stdin) == 0)
	{
		fprintf(stderr, "fgets error.\n");
		exit(RETURN_FGETS_ERROR);
	}
	//strip newlines from username
	for(i=0; i<MAX_USERNAME_SIZE; i++)
	{
		if(username[i] == '\n') username[i] = '\0';
	}
	
	
	printf("attempting connection to %s:%s... ", SERVER_STRING, PORT);
	
	if((status = getaddrinfo(SERVER_STRING, PORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(RETURN_GETADDRINFO_ERROR);
	}
	//make a socket, exiting on failure
	if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol))==-1)
	{
		perror("socket");
		exit(RETURN_SOCKET_ERROR);
	}
	//connect this socket, exiting on failure
	if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)==-1)
	{
		perror("connect");
		exit(RETURN_CONNECT_ERROR);
	}
	printf("connected.\n");
	
	
	/* do client stuff here
	
	
	if((received = recv(sockfd, data, MAX_MESSAGE_SIZE, 0)) == -1)
	{
		perror("recv");
		exit(RETURN_RECV_ERROR);
	}
	//*/
	do
	{
		printf(">: ");
		if(fgets(line, MAX_LINE_SIZE, stdin) == 0)
		{
			fprintf(stderr, "fgets error.\n");
			exit(RETURN_FGETS_ERROR);
		}
		for(i=0; i<MAX_LINE_SIZE; i++)
		{
			if(line[i] == '\n') line[i] = '\0';
		}
		/* "/quit" builtin */
		if(strcmp(line, "/quit") == 0)
		{
			close(sockfd);
			freeaddrinfo(servinfo);
			exit(RETURN_SUCCESS);
		}
		
		
		/* format the message we will send as username+": "+line+'\0'
		j walks the buffer we will send, i walks each sub-buffer */
		j=0;
		message[j++] = '\n';
		for(i=0; i<MAX_USERNAME_SIZE && username[i] != 0; i++, j++)
		{
			message[j] = username[i];
		}
		message[j++] = ':';
		message[j++] = ' ';
		for(i=0; i<MAX_LINE_SIZE && line[i] != 0; i++, j++)
		{
			message[j] = line[i];
		}
		message[j] = '\0';
		
		/*send the formatted message*/
		if((sent = send(sockfd, message, j+1, 0)) == 0)
		{
			perror("send");
			exit(RETURN_SEND_ERROR);
		}
		/*receive message from server*/
		if((received = recv(sockfd, message, sizeof(message), 0)) == -1)
		{
			perror("recv");
			exit(RETURN_RECV_ERROR);
		}
		else 
		if(received != 0)
		{
			if(message[received] != '\0') message[received] = '\0';
			printf("%s\n", message);
		}
		
	}
	while(received);
	
	printf("remote host closed connection\n");
	
	close(sockfd);
	freeaddrinfo(servinfo);
	exit(RETURN_SUCCESS);
}
