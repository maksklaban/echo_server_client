CC = gcc
CFLAGS = -Wall -std=gnu11
OUTFILE = client
OBJS = client.o
SRCS = client.c

$(OUTFILE): $(OBJS)
	$(CC) $(CFLAGS) -o $(OUTFILE) $(OBJS)
$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS)
clean:
	rm -rf *.o *.out


