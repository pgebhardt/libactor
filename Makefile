# libactor
#
# Implementation of an erlang style actor model using libdispatch
# Copyright (C) 2012  Patrik Gebhardt
# Contact: patrik.gebhardt@rub.de
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Directories
SRC = src
INCLUDES = include
BUILD = build
EXAMPLES = examples

# Copmiler and flags
CC = clang
CFLAGS = -fblocks
LDFLAGS = -L$(BUILD) -lactor

# Install directories
INSTALL_INCLUDES = /usr/local/include/actor
INSTALL_LIB = /usr/local/lib

# Object files
_OBJ = actor.o message.o process.o node.o distributer.o error.o
OBJ = $(patsubst %, $(BUILD)/%, $(_OBJ))

# Dependencies
_DEPS = actor.h message.h process.h node.h distributer.h error.h common.h
DEPS = $(patsubst %, $(INCLUDES)/%, $(_DEPS))

# Examples
EXAMPLEOBJ = $(patsubst $(EXAMPLES)/%.c, %, $(wildcard $(EXAMPLES)/*.c))

# Library
LIB = libactor.a

# Rule for library
$(LIB): $(OBJ) $(DEPS)
	ar rc $(BUILD)/$(LIB) $(OBJ)
	ranlib $(BUILD)/$(LIB)

# Rule for examples
examples: $(LIB) $(EXAMPLEOBJ)

# Rule for object files
$(BUILD)/%.o: $(SRC)/%.c $(DEPS)
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule for example executables
%: $(EXAMPLES)/%.c
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $< $(LDFLAGS)

# Install
install: $(BIN)
	mkdir -p $(INSTALL_INCLUDES)
	install -m 0644 $(INCLUDES)/*.h $(INSTALL_INCLUDES)
	install -m 0644 $(BUILD)/$(LIB) $(INSTALL_LIB)

# Uninstall
uninstall:
	rm -rf $(INSTALL_INCLUDES)
	rm -rf $(INSTALL_LIB)/$(LIB)

# Cleanup
clean:
	rm -rf $(BUILD)
	rm -rf $(BIN)

# Flags
.PHONY: clean install
