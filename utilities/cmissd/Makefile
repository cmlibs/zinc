OPENSSLDIR=
CMGUI_SRC=../..
CFLAGS=-g -I$(CMGUI_SRC) -I$(OPENSSLDIR)/include -I.
LD= -lssl -lcrypto


all: cmiss_client cmissd

cmiss_client: cmiss_client.o message.o ssl_utilities.o
	$(CC) cmiss_client.o message.o ssl_utilities.o -o cmiss_client $(LD)

cmissd: cmissd.o message.o ssl_utilities.o
	$(CC) cmissd.o message.o ssl_utilities.o -o cmissd $(LD)

message.o: 
	$(CC) $(CFLAGS) -c -o message.o $(CMGUI_SRC)/user_interface/message.c

ssl_utilities.o: ssl_utilities.h ssl_utilities.c