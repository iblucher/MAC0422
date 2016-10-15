CC = gcc
CFLAGS = -g -Wall

EXEC = ep2
SOURCES = ep2.c ep2_u.c ep2_v.c

$(EXEC): $(EXEC).c $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXEC) -lpthread

clean:
	@rm -f $(EXEC)