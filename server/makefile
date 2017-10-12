CC = gcc
CFLAGS = -Wall -std=gnu11
OUTFILE = server
OBJS = server.o
SRCS = server.c

$(OUTFILE): $(OBJS)
	$(CC) $(CFLAGS) -o $(OUTFILE) $(OBJS) && ./$(OUTFILE)
$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS)
clean:
	rm -rf *.o *.out

