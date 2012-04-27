CC = clang
CFLAGS = -fblocks
LDFLAGS = 

OBJ = actor.o message.o process.o node.o distributer.o
BIN = libactor.a

libactor: $(OBJ)
	ar rc $(BIN) $(OBJ)
	ranlib $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -rf $(BIN) $(OBJ)
