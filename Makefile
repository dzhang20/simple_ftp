CC = gcc
CFLAGES = -Wall -pthread
LDFLAGS =
LIBS = .
SRC = ftp_srv.c ftp_cli.c

all: server client
server:
	$(CC) $(CFLAGES) ftp_srv.c -lpthread $(OUT) -o ftp_srv
client:
	$(CC) $(CFLAGES) ftp_cli.c $(OUT) -o ftp_cli
clean: 
	@rm ftp_srv ftp_cli
	@echo Cleaned!
