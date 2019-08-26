CC = gcc
OBJS = main.o main_aux.o game.o solver.o parser.o stack.o doubly_linked_list.o structs.o
EXEC = project
COMP_FLAG = -ansi -Wall -Wextra -Werror -pedantic-errors
GUROBI_COMP = -I/usr/local/lib/gurobi563/include
GUROBI_LIB = -L/usr/local/lib/gurobi563/lib -lgurobi56

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(GUROBI_LIB) -o $@ -lm
	
main.o: main.c main_aux.h parser.h structs.h
	$(CC) $(COMP_FLAGS) -c $*.c
main_aux.o: main_aux.c main_aux.h parser.h structs.h
	$(CC) $(COMP_FLAG) -c $*.c
game.o: game.c game.h solver.h parser.h structs.h doubly_linked_list.h stack.h
	$(CC) $(COMP_FLAG) -c $*.c
solver.o: solver.c solver.h parser.h structs.h
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
parser.o: parser.c parser.h structs.h
	$(CC) $(COMP_FLAG) -c $*.c
stack.o: stack.c stack.h solver.h structs.h
	$(CC) $(COMP_FLAG) -c $*.c
doubly_linked_list.o: doubly_linked_list.c doubly_linked_list.h structs.h
	$(CC) $(COMP_FLAG) -c $*.c
structs.o: structs.c structs.h
	$(CC) $(COMP_FLAG) -c $*.c

clean:
	rm -f $(OBJS) $(EXEC)
