# Copmiler and flags
CC = clang
CFLAGS = -fblocks
LDFLAGS = 

# Directories
SRC = src
BUILD = build
OUTPUT = actor

# Install directories
INCLUDES = /usr/local/include/actor
LIBS = /usr/local/lib

# Object files
_OBJ = actor.o message.o process.o node.o distributer.o error.o
OBJ = $(patsubst %, $(BUILD)/%, $(_OBJ))

# Dependencies
_DEPS = actor.h message.h process.h node.h distributer.h error.h common.h
DEPS = $(patsubst %, $(SRC)/%, $(_DEPS))

# Output file
BIN = libactor.a

# Rule for library
$(BIN): $(OBJ)
	ar rc $(BIN) $(OBJ)
	ranlib $(BIN)
	mkdir -p $(OUTPUT)
	mv $(BIN) $(OUTPUT)
	cp $(SRC)/*.h $(OUTPUT)

# Rule for object files
$(BUILD)/%.o: $(SRC)/%.c $(DEPS)
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

# Cleanup
clean:
	rm -rf $(BUILD)

# Install
install: $(BIN)
	mkdir -p $(INCLUDES)
	install -m 0644 $(OUTPUT)/*.h $(INCLUDES)
	install -m 0644 $(OUTPUT)/$(BIN) $(LIBS)

# Uninstall
uninstall:
	rm -rf $(INCLUDES)
	rm -rf $(LIBS)/$(BIN)

# Flags
.PHONY: clean install
