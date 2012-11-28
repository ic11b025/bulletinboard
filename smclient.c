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
#include "stdio.h"
#include <simple_message_client_commandline_handling.h>
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
int main(int argc, char * const * argv)
{
    argc = argc;                 /*supress waring*/
    executable   = argv[0];      /* Name der ausgeführten Datei für Fehlerausgabe */
    
    fprintf(stdout, "Hello\n");
    return(0);
}

/*
* =================================================================== eof ==
*/
