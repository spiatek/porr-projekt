CC=gcc
CFLAGS = -msse4.1 -O
LFLAGS = -lm
OBJS = auction.o

auction: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) 

auction.o: auction.c
	$(CC) $(CFLAGS) -c $< -o $@

read_network.o: read_network.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
