.PHONY: all clean

CC = g++ # compiler to use
CFLAGS = -c -g -O3 -I. -IFormulaParser #-Wall -Werror -Wextra -pedantic# flags to use at the compliation
LINKERFLAG = -lm

all: prog

prog: obj/parser_test.o
	${CC} -o parser $(LIB_BINS) obj/parser_test.o ${LINKERFLAG}

obj:
	@echo "Making obj directory..."
	mkdir obj

obj/parser_test.o: obj parser_test.cpp
	@echo "Compiling parser_test.cpp..."
	${CC} ${CFLAGS} -o obj/parser_test.o parser_test.cpp

clean:
	@echo "Cleaning up..."
	rm -rvf obj
	rm -vf parser