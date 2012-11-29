/**
* @file smclient.c
* Betriebssysteme Aufgabe 2 - simple message client
*
* @author Roland Hochreiter   <ic11b025@technikum-wien.at>
* @author Mihajlo Milanovic   <ic11b081@technikum-wien.at>
* @date 2012/11/29
*
* @version $Revision: 001 $
*
* @todo - alles
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

/*
* --------------------------------------------------------------- defines --
*/
/*
* -------------------------------------------------------------- typedefs --
*/

/*
* --------------------------------------------------------------- globals --
*/

/*
* ------------------------------------------------------------- functions --
*/

/**
*
* \brief prints the usage information and exits
*
* \parm stream - stream where to write the usage (stderr)
* \parm executable - name of the executed file
* \parm exitcode - exit code for termination
*
* \return EXIT_FAILURE
*
*/
void usagefunc(FILE * stream, const char * cmd, int exitcode) {
	fprintf(stream, "usage: %s\noptions:\n", cmd);
	fprintf(stream, "    -s, --server <server>   hostname, full qualified domain name or IP address of the server\n");
	fprintf(stream, "    -p, --port <port>       TCP port of the server [0..65535]\n");
	fprintf(stream, "    -u, --user <name>       name of the posting user\n");
	fprintf(stream, "    -i, --image <URL>       URL pointing to an image of the posting user\n");
	fprintf(stream, "    -m, --message <message> message to be added to the bulletin board\n");
	fprintf(stream, "    -v, --verbose           verbose output\n");
	fprintf(stream, "    -h, --help\n");
	exit(exitcode);
}

/**
*
* \brief ckecks if the struct sockaddr contains an IPv4 or IPv6 address information
*
* \parm struct sockaddr * - the struct which point to the struct with the address information
*
* \return void * - pointer to the IPv4 or IPv6 address
*
*/
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
*
* \brief get address information about the server and connect
*
* \parm ?? - 
* \parm 
* \parm 
*
* \return ?
*
*/
int get_server_info(const char * server, const char * port) {

    int status = 0;
    int sockfd = 0;
    struct addrinfo hints;
    struct addrinfo *servinfo;       /* will point to the results */
    struct addrinfo *p;              /* auxiliary pointer for *servinfo */
    char ipstr[INET6_ADDRSTRLEN];    /* string to store IP address */
    
    memset(&hints, 0, sizeof hints); /* make sure the struct is empty */
    hints.ai_family = AF_UNSPEC;     /* IPv4 and IPv6 may be used */
    hints.ai_socktype = SOCK_STREAM; /* use TCP stream sockets */
    /*hints.ai_flags = AI_PASSIVE;*//*only for server*/     /* fill in my IP for me */    
    /* call getaddrinfo to fill the struct servinfo */
    if ((status = getaddrinfo(server, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
    }
/* servinfo now points to a linked list of 1 or more struct addrinfo */

    /* this loop goes throu the linked list, to catch all server addresses */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;   /* debug - print the IP version */
        /* get the pointer to the address itself,
           different fields in IPv4 and IPv6:      */
        if (p->ai_family == AF_INET) {     /* IPv4 */
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else {                           /* IPv6 */
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        /* debug info only */
        /* convert the IP address to a string and print it: */
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("found %s address: %s Port: %s\n", ipver, ipstr, port);
    
        /* call socket(). Returns the socketnumber, required for connect() */
        errno  = 0;
        if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) != 0) {
        /* error handling and continue with next struct (ai_next) */
        perror("client: socket");
        }

        /* now we call connect */
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;      /* go to ai_next, maybe we can connect there */
        }
        printf("break\n"); /* printf for debug */
        break;             /* leave the for loop, because connect() did not report error */
  } 
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;         /* the for loop did not find any address where we could connect - all tries have failed */
    }
    
    
    /* now we are connected! Huhuu! */
    printf("now connected!\n"); /* printf for debug */
    /* convert the IP address to a string and print it */
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), ipstr, sizeof ipstr);
    printf("client: connected to %s : %s\n", ipstr, port);

    
    freeaddrinfo(servinfo); /* free the linked-list */
    return 0;
    /* htons();*/
}

/*
void connect_to_server() {
    struct sockaddr_in sa;  IPv4
    struct sockaddr_in6 sa6;  IPv6
    inet_pton(AF_INET, "192.0.2.1", &(sa.sin_addr));  IPv4
    inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr));  IPv6

}*/

/**
*
* \brief implements the message client
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
    const char *server  = NULL;
    const char *port    = NULL;
    const char *user    = NULL;
    const char *message = NULL;
    const char *img_url = NULL;
    int verbose         = 0;

    /* fill the const char *server, port, user, message, img_url and int verbose with data from command line */    
    smc_parsecommandline(argc, argv, *usagefunc, &server, &port, &user, &message, &img_url, &verbose);

    /* just debug info */
    fprintf(stdout, "von Commandline erhalten:\n"
                    "server=%s\n"
                    "port=%s\n"
                    "user=%s\n"
                    "message=%s\n"
                    "img=%s\n"
                    "verbose=%d\n\n", server, port, user, message, img_url, verbose);

    /* create structs with address information */
    get_server_info(server, port);


    return(0);
}

/*
* =================================================================== eof ==
*/
