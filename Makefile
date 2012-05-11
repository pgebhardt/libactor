# Copmiler and flags
CC = clang
CFLAGS = -fblocks
LDFLAGS = 

# Directories
SRC = src
BUILD = build
OUTPUT = actor

# Object files
_OBJ = actor.o message.o process.o node.o distributer.o error.o
OBJ = $(patsubst %, $(BUILD)/%, $(_OBJ))

# Dependencies
_DEPS = actor.h message.h process.h node.h distributer.h error.h common.h
DEPS = $(patsubst %, $(SRC)/%, $(_DEPS))

# Output file
BIN = libactor.a

# Rule for object files
$(BUILD)/%.o: $(SRC)/%.c $(DEPS)
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule for library
libactor: $(OBJ)
	ar rc $(BIN) $(OBJ)
	ranlib $(BIN)
	mkdir -p $(OUTPUT)
	mv $(BIN) $(OUTPUT)
	cp $(SRC)/*.h $(OUTPUT)

# Cleanup
clean:
	rm -rf $(BUILD)

# Flags
.PHONY: clean
