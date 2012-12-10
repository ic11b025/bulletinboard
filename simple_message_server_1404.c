/**
* @file smserver.c
* Betriebssysteme Aufgaben 2 bis 4 - simple message server
*
* @author Roland Hochreiter   <ic11b025@technikum-wien.at>
* @author Mihajlo Milanovic   <ic11b081@technikum-wien.at>
* @date 2012/12/10
*
* @version $Revision: 001 $
*
* @todo - remove debug printf(), add doxygen Kommentare
*
* URL: $HeadURL$
*
* Last Modified: $Author: Roland $
*/

/*
* -------------------------------------------------------------- includes --
*/
#include <stdio.h>
#include <stdlib.h>                             /* exit() */
#include <errno.h>                              /* error handling */
#include <unistd.h>
#include "simple_message_client_commandline_handling.h"
#include <sys/types.h>                           /* getaddrinfo() */
#include <sys/socket.h>                          /* getaddrinfo() */
#include <netdb.h>                               /* getaddrinfo() */
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <limits.h>
#include <sys/wait.h>
#include <signal.h>
/*
* --------------------------------------------------------------- defines --
*/

#define BACKLOG 10   /* how many pending connections queue will hold */
#define PATHBULOGIC "/usr/local/bin/simple_message_server_logic" /*the path of the business_logic*/

/*
* -------------------------------------------------------------- typedefs --
*/

/*
* --------------------------------------------------------------- globals --
*/

/*
* ------------------------------------------------------------- functions --
*/

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
    s=s; /*avoid the warning */
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
*
* \brief prints the usage information and exits
*
* \parm const char *cmd - name of the executed file
*
* \return failure in all cases
*
*/
void usagefunc(const char * cmd) {
	fprintf(stderr, "usage:\n%s <options>\noptions:\n", cmd);
	fprintf(stderr, "    -p, --port <port>       listening TCP port of the server [0..65535]\n");
	fprintf(stderr, "    -h, --help\n");
	exit(EXIT_FAILURE);
}

void parse_commandline(int argc, const char * const * argv){
	
	char *endptrstrtol  = NULL;
	long lport          = -1;
    /* parse the commandline */
    if (argc != 3) {  /*if using "-h" it will not work......*/
            fprintf(stderr, "Fehler: argc = %d\n", argc);
    	    usagefunc(argv[0]);
    } else {
    	    if ((strcmp(argv[1], "-p")  != 0) && (strcmp(argv[1], "--port")) != 0) {
    	    	    fprintf(stderr, "Fehler: argv[1] ist nicht -p oder --port, sondern %s\n", argv[1]);
                    usagefunc(argv[0]);
    	    } else {
    	    	    lport = strtol(argv[2], &endptrstrtol, 10);
                    
    	    	if ((errno == ERANGE && (lport == LONG_MAX || lport == LONG_MIN)) || (errno != 0)) {
        			fprintf(stderr, "%s\n",strerror(errno));
        			usagefunc(argv[0]);
    			}
				if (endptrstrtol == argv[2]) {
        			fprintf(stderr, "No digits were found\n");
        			usagefunc(argv[0]);
    			}
    			if (lport < 0 || lport > 65535) {
                       fprintf(stderr, "Fehler: Port ist zu hoch : %ld\n", lport);
                       usagefunc(argv[0]);
                }
                    fprintf(stderr, "Debug: Portnummer von strtol() = %ld\n", lport);
            /* argv[2] is a valid number from 0 to 65535 */
    	    }
    }
    /* just debug info */
    fprintf(stdout, "von commandline hat smserver erhalten:\n"
                    "\tstring argv[2] = %s\n"
                    "\tlong port      = %ld\n"
                    , argv[2], lport);

}

int get_port(int sockfd, const char * const * argv){

struct addrinfo hints, *servinfo, *p;
struct sigaction sa;
int yes             =  1;
    int rv              = -1;

memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;        /* IPv4 and IPv6 supported */
    hints.ai_socktype = SOCK_STREAM;    /* TCP */
    hints.ai_flags = AI_PASSIVE;        /* use my IP address */
    
    if ((rv = getaddrinfo(NULL, argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    /* loop through all the results and bind to the first we can */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    
    freeaddrinfo(servinfo);           /* free the struct, we do not need it anymore */
    
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    sa.sa_handler = sigchld_handler;  /* catch all dead processes */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    fprintf(stderr, "server: waiting for connections...\n");
    return sockfd;

}

/**
*
* \brief implements the message server, which spawn the buisness logic process
*
* This is the main entry point for any C program.
*
* \param argc the number of arguments
* \param argv the arguments itselves (including the program name in argv[0])
*
* \return success or failure
*
*/
int main(int argc, const char * const * argv)
{
    
    char s[INET6_ADDRSTRLEN];
    
    int sockfd          = -1;  /* listen on sockfd */
    int sockfdchild     = -1;  /* new connection on sockfdchild */
    struct sockaddr_storage their_addr; /* client's address information */
    socklen_t sin_size  = -1;
    pid_t pid = -1;
    
    parse_commandline(argc,argv);
    
	sockfd = get_port(sockfd, argv);
    
    
    while(1) {  /* loop to accept() connections */
        sin_size = sizeof their_addr;
        sockfdchild = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (sockfdchild == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s); /* get information about the connected client */
        fprintf(stderr, "server: got connection from %s\n", s);
        
       switch (pid = fork())
       {
		   case -1: { /*error*/
					close(sockfdchild);
					close(sockfd);
					fprintf(stderr,"error after forking");
					exit(1);
					break;
			}
		   case 0:{/*child process*/
					close(sockfd);
					if (dup2(sockfdchild,0) == -1){ /*Umwandeln stdin*/
						close(sockfdchild);
						exit(EXIT_FAILURE);
					}
					if (dup2(sockfdchild,1) == -1){ /*Umwandeln stdout*/
						close(sockfdchild);
						exit(EXIT_FAILURE);
					}
                                        errno = 0;
                                        (void) execlp(PATHBULOGIC,"simple_message_server_logic" ,NULL);
                                        
                                        fprintf(stderr, "execlp() failed: %s\n", strerror(errno));
					exit(EXIT_FAILURE);
		   }
		   default:{ /*mother process*/
					close(sockfdchild);  /* parent doesn't need this */
			}
	   }  
    }
    exit(EXIT_SUCCESS);
}

/*
* =================================================================== eof ==
*/
