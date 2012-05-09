CC = clang
CFLAGS = -fblocks
LDFLAGS = 

OBJ = actor.o message.o process.o node.o distributer.o error.o
BIN = libactor.a
SRC = src
OUTPUT = actor

libactor: $(OBJ)
	ar rc $(BIN) $(OBJ)
	ranlib $(BIN)
	#mkdir $(OUTPUT)
	mv $(BIN) $(OUTPUT)
	cp $(SRC)/*.h $(OUTPUT)

%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -rf $(OUTPUT)/$(BIN) $(OBJ)
