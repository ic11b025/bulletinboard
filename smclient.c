/**
* @file smclient.c
* Betriebssysteme Aufgabe 2 - simple message client
*
* @author Roland Hochreiter   <ic11b025@technikum-wien.at>
* @author Mihajlo Milanovic   <ic11b081@technikum-wien.at>
* @date 2012/11/27
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
#include <stdlib.h>
#include <errno.h>
#include "simple_message_client_commandline_handling.h"
/*
* --------------------------------------------------------------- defines --
*/
/*
* -------------------------------------------------------------- typedefs --
*
*/

/*
* --------------------------------------------------------------- globals --
*/
const char * executable;   /*speichert den Namen der ausführbaren Datei*/
/*
* ------------------------------------------------------------- functions --
*/

/**
*
* \brief prints the usage information
*
*/

void usagefunc(FILE * file, const char * text, int exitCode) {
    
    if (file) {fprintf(stderr, "\n");}
    if (text) {fprintf(stderr, "\n");}
    if (exitCode) {fprintf(stderr, "\n");}
    fprintf(stderr, "Usage: bla bla");
    exit(exitCode);
    
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
    /*smc_usagefunc_t usagefunc = NULL;*/
    /*smc_usagefunc_t usagefunc = print_usage;*/  /* irgendwas passt da nicht */
    const char *server  = NULL;
    const char *port    = NULL;
    const char *user    = NULL;
    const char *message = NULL;
    const char *img_url = NULL;
    int verbose;

    executable   = argv[0];      /* Name der ausgeführten Datei für Fehlerausgabe */
    
    fprintf(stdout, "Hello\n");
    
    smc_parsecommandline(argc, argv, *usagefunc, &server, &port, &user, &message, &img_url, &verbose);
    
    fprintf(stdout, "Hello2\n"); 
    fprintf(stdout, "server=%s\n"
                    "port=%s\n"
                    "user=%s\n"
                    "message=%s\n"
                    "img=%s\n"
                    "verbose=%d\n", server, port, user, message, img_url, verbose);

    return(0);
}

/*
* =================================================================== eof ==
*/
