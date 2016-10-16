CC = gcc
CFLAGS = -g -Wall

EXEC = ep2
SOURCES = ep2.c ep2_u.c ep2_u.h ep2_v.c ep2_v.h

$(EXEC): $(EXEC).c $(SOURCES)
    $(CC) $(CFLAGS) $(SOURCES) -o $(EXEC) -lpthread
 
clean:
    @rm -f $(EXEC)
