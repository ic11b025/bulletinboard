##
## @file Makefile
## Betriebssysteme Aufgabe 2 Makefile
##
## @author Mihajlo Milanovic   <ic11b081@technikum-wien.at>
## @author Roland Hochreiter   <ic11b025@technikum-wien.at>
## @date 2012/11/27
##
## @version $Revision: 001 $
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

OBJECTCLIENT := smclient.o

##
## ----------------------------------------------------------------- rules --
##

%.o : %.c
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

all: smclient

smclient: $(OBJECTCLIENT)
	$(CC) $(CFLAGS) -o $@ $^ -lsimple_message_client_commandline_handling

clean:
	$(RM) $(OBJECTCLIENT) smclient

distclean: clean
	$(RM) -r doc

doc:
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
