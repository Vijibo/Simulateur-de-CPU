CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC = $(wildcard src/*.c)
SRC_NO_MAIN = $(filter-out src/cpu_simulator.c, $(SRC))
OBJ = $(SRC_NO_MAIN:.c=.o)
BIN = cpu_simulator

TESTS = $(patsubst tests/%.c,tests/%,$(wildcard tests/*.c))

all: $(BIN)

# Compilation du simulateur principal avec main.c
$(BIN): $(OBJ) src/cpu_simulator.c
	$(CC) -o $@ $^ $(CFLAGS)

# Compilation des tests sans src/main.c
tests/test_%: tests/test_%.c $(SRC_NO_MAIN)
	$(CC) -o $@ $^ $(CFLAGS)

test: $(TESTS)
	@echo "Tests compilés. Lancez ./tests/test_nom"

clean:
	rm -f $(BIN) src/*.o tests/*.o $(TESTS)

.PHONY: all clean test


#make           # compile le simulateur principal
#make test      # compile les tests 

# ./tests/test_parser
# ./tests/test_addressing
