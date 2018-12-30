CC=gcc
CFLAGS=
DEPS =
OBJ = unpackfw.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

unpackfw: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f unpackfw.o unpackfw