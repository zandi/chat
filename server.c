/* server.c
server for the "chat" program.


*/
#include "chat.h"

struct slave_server_node{
	int m2spfd[2]; //m2spfd[0] is the read end, m2spfd[1] is the write end.
	int s2mpfd[2];
	struct slave_server *next;
};

//here, we waitpid() any terminated child processes away.
//we use waitpid() in case we have multiple terminated children	
void sig_chld(int signum)
{
	pid_t pid;
	int status;

	//wait away any children that have terminated
	while((pid = waitpid(-1, &status, WNOHANG)) > 0)
		printf("waitpid()'d child - %i = %i\n", pid, status);
	if(pid==-1)
		printf("waitpid(): %s\n", strerror(errno));
	if(pid==0)
		printf("no children have changed state.\n");

	return;
}

void slave_server(int clientfd, int m2spfd, int s2mpfd, char *message, int bufsize)
{
	fdset rset, eset;
	char *p=message;
	int maxsize=bufsize;
	int sent=0, received=0;
	int nfds= (clientfd>m2spfd) ? clientfd : m2spfd;
	if(nfds<s2mpfd)
		nfds=s2mpfd;
	nfds++; //nfds is now the largest file descripter we care about plus 1

	//we only need to select between our connected socket and the pipe
	FD_ZERO(&rset);
	FD_ZERO(&eset);
	FD_SET(clientfd, &rset);
	FD_SET(m2spfd, &rset);
	FD_SET(clientfd, &eset);//these may not even be necessary... not sure.
	//FD_SET(m2spfd[0], &eset); //i don't think this is necessary on a pipe

	//now we wait for data to relay
	if((select(nfds, &rset, 0, &eset, NULL)) == -1){
		perror("select");
		exit(-1);
	}
	if(FD_ISSET(m2spfd, &rset)){//the master server has a message for us
		//read the message from the pipe to our message buffer
		//until we hit EOF that is (hopefully)
		p=message;
		memset(message, 0x00, bufsize);//clean the buffer
		received=0;
		while((received = read(m2spfd, p, bufsize))!=0)
		{
			if(received==-1){
				if(errno==EINTR) continue;
				perror("pipe read");
				exit(-1);
			}
			bufsize -= received;
			p+= received;
		}
		//and now we send the message to the connected client
		if((sent = send(clientfd, message, received, 0)) == -1)
		{
			perror("send");
			exit(RETURN_SEND_ERROR);
		}
	}
	if(FD_ISSET(clientfd, &rset)){
		//we have a message from the client
		memset(message, 0x00, maxsize);//clean the buffer
		p=message;
		sent=0;
		received=0;
		received=recv(clientfd, message, maxsize, 0);
		if(received == -1)
		{
			perror("recv");
			exit(RETURN_RECV_ERROR);
		}
		//null-terminate if necessary, just in case
		if(message[received-1] != '\0') message[received-1] = '\0';
		printf("received %i bytes\n", received);
		//TODO: check if this really works
		while(sent = write(s2mpfd, p, received))
		{
			if(sent == -1){
				if(errno==EINTR) continue;
				perror("send");
				exit(RETURN_SEND_ERROR);
			}
			received -= sent;
			p+= sent;
		}

		printf("sent %i bytes\n", sent);
	}
	if(FD_ISSET(clientfd, &eset)){
		//we have an error on the conected socket.
		if((recv(clientfd, message, maxsize, 0))==-1){
			perror("recv");
			exit(-1);
		}
		//not sure if there's something else i should be doing here...
	}
	return;
}




void server(const char *PORT)
{
	/* server setup stuff */
	int status, sockfd, clientfd, sent, received;
	int i, m2spfd[2], s2mpfd[2], numclients=0, nfds=0;
	pid_t pid;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // servinfo will point to the results
	struct sockaddr_storage their_addr;
	struct sigaction act, oact; //new and old signal action structures
	socklen_t addr_size = sizeof(their_addr);
	struct slave_server_node *curslavenode=0, *head=0;
	fd_set rset, wset, eset;
	
	char message[MAX_USERNAME_SIZE + MAX_LINE_SIZE + 4]; //+4 for the '\n', ':', ' ', and '\0' in any message
	
	memset(&hints, 0, sizeof hints); // make sure the struct is empty (nulled out)
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me (server)
	if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) 
	{
	    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	    exit(RETURN_GETADDRINFO_ERROR);
	}
	if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
	{
		perror("socket");
		exit(RETURN_SOCKET_ERROR);
	}
	if(sockfd>nfds)
		nfds=sockfd;
	if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		perror("bind");
		exit(RETURN_BIND_ERROR);
	}
	if(listen(sockfd, 10) == -1)
	{
		perror("listen");
		exit(RETURN_LISTEN_ERROR);
	}
	//we're done with servinfo now, so let's free it
	freeaddrinfo(servinfo);

	//if we made it here we should be properly listening right now.
	printf("listening on port %s.\n", PORT);
	
	//now we set up the handler for SIGHCHLD
	printf("registering signal handler\n");
	act.sa_handler = sig_chld;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	if(sigaction(SIGCHLD, &act, &oact)<0)
	{
		printf("error registering signal handler\n");
		exit(1);
	}


	
	/* do server stuff here 
		early version just echos the client until connection close
	*/
	//first time through we're just waiting for a new connection
	while((clientfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) < 0){
		//error stuff in here
		if(errno==EINTR)
			continue;  //interrupted by a signal, just try again
		perror("accept");
		close(sockfd);
		exit(-1);
	}
	if(clientfd>nfds)
		nfds=clientfd;
	//we have our first connection!
	//set up the new pipes between master and slave
	if(pipe(m2spfd)){
		perror("pipe");
		exit(-1);
	}
	if(m2spfd[0]>nfds)
		nfds=m2spfd[0];
	if(m2spfd[1]>nfds)
		nfds=m2spfd[1];
	if(pipe(s2mpfd)){
		perror("pipe");
		exit(-1);
	}
	if(s2mpfd[0]>nfds)
		nfds=s2mpfd[0];
	if(s2mpfd[1]>nfds)
		nfds=s2mpfd[1];
	//now the initial node in our linked list
	//TODO: move this stuff to only be in the master server
	if((head = malloc(sizeof(struct slave_server_node))) == NULL){
		perror("malloc");
		exit(-1);
	}
	curslavenode = head;
	curslavenode->m2spfd[0] = m2spfd[0];
	curslavenode->m2spfd[1] = m2spfd[1];
	curslavenode->s2mpfd[0] = s2mpfd[0];
	curslavenode->s2mpfd[1] = s2mpfd[1];
	curslavenode->next = 0;//no next node yet.
	numclients++;

	//now we fork(), and do our respective select() loops in both master server and slave server

	if((pid=fork()) < 0){
		perror("fork"); exit(-1);
	}
	if(pid==0){//we're in the slave server
		close(sockfd);
		close(m2spfd[1]);
		close(s2mpfd[0]); //the slave doesn't need these file descriptors

		//this function takes care of all the tedious stuff we need to do as the slave server
		slave_server(clientfd, m2spfd[0], s2mpfd[1], message, sizeof(message));
	}









	while(1)
	{
	
		if(clientfd<0)
		{
			//error stuff in here
			if(errno==EINTR)
				continue;  //interrupted by a signal, just try again

			printf("accept: %s", strerror(errno));
			exit(1);
		}

		//now the actual work
		//for now we just recv a message and echo it 
		pid=fork();
		if(pid == -1)	{
			printf("fork error: %s\n", strerror(errno));
			exit(1);
		}
		if(pid == 0)
		{
			//SLAVE_SERVER
			close(sockfd);//close the listening socket in the client.
			while(received = recv(clientfd, message, sizeof(message), 0))
			{
				if(received == -1)
				{
					perror("recv");
					exit(RETURN_RECV_ERROR);
				}
				//null-terminate if necessary, just in case
				if(message[received-1] != '\0') message[received-1] = '\0';
				printf("received %i bytes\n", received);
				if((sent = send(clientfd, message, received, 0)) == -1)
				{
					perror("send");
					exit(RETURN_SEND_ERROR);
				}
				printf("sent %i bytes\n", sent);
			}
		printf("remote client has closed connection\n");
		exit(0);
		}
		else
		{
			//we're in the parent
			close(clientfd);
		}
	}
	
	
	close(sockfd);
	exit(RETURN_SUCCESS);
}
