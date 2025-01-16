# compile with gcc
CC=gcc

# Compiler options:
CFLAGS = -g -Wall -Wextra -pthread #-Werror
#TEST = -f testing.csv -b 2

# set the name of the executable file to compile here
PROGRAM = final

PROGRAM2 = reader

PROGRAM3 = writer

OBJS = main.o util.o

OBJS2 = reader.o

OBJS3 = writer.o

all: $(PROGRAM) $(PROGRAM2) $(PROGRAM3)

$(PROGRAM): $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM)

$(PROGRAM2): $(OBJS2)
	$(CC) $(CFLAGS) $(OBJS2) -o $(PROGRAM2)

$(PROGRAM3): $(OBJS3)
	$(CC) $(CFLAGS) $(OBJS3) -o $(PROGRAM3)

clean:
	rm -f $(PROGRAM) $(OBJS) $(LAB) $(LAB_OBJS)
