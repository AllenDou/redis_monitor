CC=gcc
CFLAGS= -Wall -O0 -g -rdynamic -ggdb
MONITOR=redis_monitor
BIN=bin
LIB=lib
OBJ=common/net.o common/sds.o common/hiredis.o redis_monitor.o

all: $(MONITOR)
$(MONITOR): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ 

clean:
	$(RM) *.o common/*.o  $(BIN)/*
