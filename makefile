CC=gcc
CFLAGS = -std=c99 -O -DDIMACS
LFLAGS = -lm
OBJS = netgen.o index.o random.o

netgen: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) 

netgen.o: netgen.c
	$(CC) $(CFLAGS) -c $< -o $@

index.o: index.c
	$(CC) $(CFLAGS) -c $< -o $@

random.o: random.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
