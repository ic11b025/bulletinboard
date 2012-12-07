##
## @file Makefile
## Betriebssysteme Aufgaben 2,3,4 Makefile
##
## @author Mihajlo Milanovic   <ic11b081@technikum-wien.at>
## @author Roland Hochreiter   <ic11b025@technikum-wien.at>
## @date 2012/12/07
##
## @version $Revision: 002 $
##
## URL: $HeadURL$
##
## Last modified: $Author: Roland $
##

##
## ------------------------------------------------------------- variables --
##

CC := gcc
CFLAGS := -I/usr/local/include -Wall -Wextra -Werror -pedantic -g -O3
RM := rm -f
DOXYGEN := doxygen

OBJECTCLIENT := simple_message_client_1404.o
OBJECTSERVER := simple_message_server_1404.o

##
## ----------------------------------------------------------------- rules --
##

%.o : %.c
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

all: client server

client: $(OBJECTCLIENT)
	$(CC) $(CFLAGS) -o $@ $^ -lsimple_message_client_commandline_handling

server: $(OBJECTSERVER)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(OBJECTCLIENT) $(OBJECTSERVER) client server

distclean: clean
	$(RM) -r doc

doc:
	$(CC) $(CFLAGS) -c $<
	$(DOXYGEN) doxygen.dcf

##
## -------- cleanup Semaphore and Shared Memory -----------------------------
##

##
## ---------------------------------------------------------- dependencies --
##

##
## =================================================================== eof ==
##
