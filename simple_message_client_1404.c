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
#include <limits.h>

/*
* --------------------------------------------------------------- defines --
*/

#define BUFFLEN 1024


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
* \parm FILE * stream   - stream where to write the usage (stderr)
* \parm const char *cmd - name of the executed file
* \parm int exitcode    - exit code for termination
*
* \return failure in all cases
*
*/
void usagefunc(FILE * stream, const char * cmd, int exitcode) {
	fprintf(stream, "usage:\n%s <options>\noptions:\n", cmd);
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
* \brief get address information about the server and connect
*
* \parm const char *server - name/address of the server to which we want to connect 
* \parm const char *port   - name/number of the port to which we want to connect
*
* \return int - filedescriptor of the socket
*
*/
int get_server_info(const char * server, const char * port) {

    int status = 0;
    int sockfd = 0;
    struct addrinfo hints;
    struct addrinfo *servinfo;       /* will point to the results */
    struct addrinfo *p;              /* auxiliary pointer for *servinfo */
    
    memset(&hints, 0, sizeof hints); /* make sure the struct is empty */
    hints.ai_family = AF_UNSPEC;     /* IPv4 and IPv6 may be used */
    hints.ai_socktype = SOCK_STREAM; /* use TCP stream sockets */
        
    /* call getaddrinfo to fill the struct servinfo */
    if ((status = getaddrinfo(server, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(EXIT_FAILURE);
    }
/* servinfo now points to a linked list of 1 or more struct addrinfo */

    /* this loop goes throu the linked list, to catch all server addresses */
    for(p = servinfo; p != NULL; p = p->ai_next) {   
        /* call socket(). Returns the socketnumber, required for connect() */
        errno  = 0;
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) != 0) {
        /* error handling and continue with next struct (ai_next) */
        perror("client: socket");
        }
        /* now we call connect */
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;      /* go to ai_next, maybe we can connect there */
        }
        break;             /* leave the for loop, because connect() did not report error */
  } 
    
    if (p == NULL) {
        return -1;         /* the for loop did not find any address where we could connect - all tries have failed */
    }
    freeaddrinfo(servinfo); /* free the linked-list */
    return sockfd;
}

/**
*
* \brief close a socket. either read/write/both directions
*
* \parm FILE *         - Filepointer to the socket to be closed
* \parm const int mode - SHUT_WR to close write only
*                      - SHUT_RD to close read only
*                      - SHUT_RDWR to close both directions
*
* \return void
*
*/
void close_conn(FILE * fp, const int mode)
{
        int sd = -1;
        sd = fileno(fp); /* get the file descriptor which to close */

        errno = 0;
        if (fflush(fp) == EOF){   /* flush the buffer */
                fprintf(stderr, "Fehler bei fflush() : %s\n", strerror(errno));
        }

        errno = 0;
        if (shutdown(sd, mode) == -1) { /* close the socket descriptor */
                fprintf(stderr, "Fehler bei shutdown() : %s\n", strerror(errno));
        }

        errno = 0;
        if (fclose(fp) == EOF){  /* now cleanup the FILE Pointer */
                fprintf(stderr, "Fehler bei fclose() : %s\n", strerror(errno));
        }
}

/**
*
* \brief sends the data to the server, throu the socket
*
* \parm const int sockfdwrite - filedescriptor of the socket to be used for writing data
* \parm const char *user      - name of the posting user
* \parm const char *message   - message to be posted
* \parm const char *img_url   - URL pointing to a picture of the posting user
*
* \return void
*
*/
void write_to_serv(const int sockfdwrite, const char *user, const char *message, const char *img_url)
{
	FILE * fpwrite;

        errno = 0;
	if((fpwrite = fdopen(sockfdwrite, "w")) == NULL){
		fprintf(stderr, "%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
 
	if(img_url == NULL)
		fprintf(fpwrite,"user=%s\n%s\n",user,message);
	else
		fprintf(fpwrite,"user=%s\nimg=%s\n%s\n",user, img_url,message);

        close_conn(fpwrite, SHUT_WR);  /* close the write direction of the socket. Server will receive EOF */
}

/**
*
* \brief receives the data from the server, throu the socket
*
* \parm const int sockfd - filedescriptor of the socket to be used for reading data
*
* \return void
*
*/

void read_from_serv(const int sockfd)
{
	char buffer[BUFFLEN];
	FILE * fpread  = NULL;
	FILE *htmlfile = NULL;
        FILE *pngfile  = NULL;
	int reccount   = 1;
	unsigned int i = 0;
	char *len      = NULL;
	char *endptrstrtol = NULL;
	long filelen   = 0;
	char *htmlname = NULL;
        char *pngname  = NULL;

	errno = 0;
	if((fpread = fdopen(sockfd, "r")) == NULL){
		fprintf(stderr, "%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	while (fgets(buffer, BUFFLEN, fpread) != NULL){  /* fgets() reads until linefeed or EOF */

		if(reccount == 1){
			if (strncmp(buffer, "status=0", 8) != 0) { /* check the status returned by the server */
                                fprintf(stderr, "Server returned error status %c\nexiting!\n", buffer[7]);
                                exit(EXIT_FAILURE);
			}
			reccount++;
			continue;
		}
		if(reccount == 2){
            if (strncmp(buffer, "file=", 5) != 0) {
				fprintf(stderr, "cannot determine filename for HTML content in the servers response\nexiting!\n");
                exit(EXIT_FAILURE);

            }
			htmlname = malloc((strlen(buffer)-6) * sizeof(char));
			if(htmlname == NULL)
				fprintf(stderr, "Fehler malloc vom HTML-File\n");
			for(i=5;i<(strlen(buffer)-1);i++){
				htmlname[i-5] = buffer[i];
			}
			if((htmlfile = fopen(htmlname,"w+")) ==  NULL) {
				fprintf(stderr, "cannot open file %s to write HTML content\n"
                                "check for write permission in directory\n"
                                "exiting!\n", htmlname);
                exit(EXIT_FAILURE);
			}
			free(htmlname); /* free memory allocated for array */
                        reccount++;
			continue;
		}
		if(reccount == 3 || reccount == 6){ /*length of html or png file*/
                        
                         if (strncmp(buffer, "len=", 4) != 0) {
				fprintf(stderr, "cannot determine content length in the servers response\nexiting!\n");
				exit(EXIT_FAILURE);
			}
                        if ( len != NULL ) {
                        len = realloc((void*)len, (strlen(buffer)-5) * sizeof(char));
                        }
                        else {
			len = malloc((strlen(buffer)-5) * sizeof(char));
			}
                        if(len == NULL)
				fprintf(stderr,"Fehler beim malloc von File-Länge\n");
			for(i=4;i<(strlen(buffer)-1);i++){
				len[i-4] = buffer[i];
			}

                        errno = 0;
                        filelen = strtol(len, &endptrstrtol, 10);
                        if ((errno == ERANGE && (filelen == LONG_MAX || filelen == LONG_MIN)) || (errno != 0 && filelen == 0)) {
        			fprintf(stderr, "%s\n",strerror(errno));
        			exit(EXIT_FAILURE);
    			}

   			if (endptrstrtol == len) {
        			fprintf(stderr, "No digits were found\n");
        			exit(EXIT_FAILURE);
    			}
			i=0;
			if (reccount == 6) { 
				free(len);  /* free memory allocated for array only when len will not be rallocated */
				break;      /* now stop using fgets(), because we want to recveice binary data for the png file */ 
			}
                        reccount++;
			continue;
		}
		if(reccount == 4){
                        
			if(i<filelen){
				fprintf(htmlfile, "%s", buffer);
			}else{
				reccount++;
                                filelen = 0;
			}
			i += strlen(buffer);
        }
		if(reccount == 5){
            if (strncmp(buffer, "file=", 5) != 0) {            
				fprintf(stderr, "cannot determine filename for PNG content in the servers response\nexiting!\n");
                exit(EXIT_FAILURE);
            }
            pngname = malloc((strlen(buffer)-6) * sizeof(char));
            if(pngname == NULL)
				fprintf(stderr,"Fehler beim malloc von png-File");
            
            for (i=5; i < (strlen(buffer)-1); i++) {
                pngname[i-5] = buffer[i];
            }
            
            if((pngfile = fopen(pngname,"w+")) ==  NULL){
				fprintf(stderr, "cannot open file %s to write PNG content\n"
                                "check for write permission in directory\n"
                                "exiting!\n", pngname);
                 exit(EXIT_FAILURE);
            }
            free(pngname); /* free memory allocated for array */
            reccount++;
            continue;
        }

	}   
	/* now the binary content for the png file is read from the filepointer */
	while (i<filelen){
		if(i+BUFFLEN>filelen){
			if(fread(&buffer,sizeof(char), filelen - i, fpread) != (unsigned long)(filelen-i)){
				fprintf(stderr, "error reading with fread....\n");
				exit(EXIT_FAILURE);
			}
			fwrite(&buffer,sizeof(char), filelen - i, pngfile);
		} else {
			if (fread(&buffer,sizeof(char), BUFFLEN, fpread) != BUFFLEN){
				fprintf(stderr, "error reading with fread....\n");
				exit(EXIT_FAILURE);
			}
			fwrite(&buffer,sizeof(char), BUFFLEN, pngfile);
		}

		i+=BUFFLEN;
	}
	errno = 0;
	if(fclose(htmlfile) == EOF){ /* close the HTML file */
		fprintf(stderr, "%s\n", strerror(errno));
	}
	errno = 0;
	if(fclose(pngfile) == EOF){ /* close the PNG file */
		fprintf(stderr, "%s\n", strerror(errno));
	}

	
       if(fclose(fpread) == EOF){
			fprintf(stderr, "%s\n", strerror(errno));
	   }
}

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
    int verbose         = -1;
    int sockfd          = -1;
    int sockfd2         = -1;

    /* fill the const char *server, port, user, message, img_url and int verbose with data from command line */    
    smc_parsecommandline(argc, argv, *usagefunc, &server, &port, &user, &message, &img_url, &verbose);

    /* create structs with address information */
    if ((sockfd = get_server_info(server, port)) == -1) {
		fprintf(stderr, "client: failed to connect\n");
		exit(EXIT_FAILURE);
	}
	errno = 0;

	if ((sockfd2 = dup(sockfd)) == -1){             /* duplicate the socketdescriptor */
		fprintf(stderr, "%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	write_to_serv(sockfd2, user, message, img_url); /* send request to server */

	read_from_serv(sockfd);                         /* receive response from server */

    exit(EXIT_SUCCESS);
}

/*
* =================================================================== eof ==
*/

