

data_dictionary: data_dictionary.c scanner.l parser.y
	bison -o parser.yy.c parser.y --defines=tokens.yy.h
	flex -o scanner.yy.c scanner.l
	gcc -c data_dictionary.c
	gcc -o $@ data_dictionary.o scanner.yy.c parser.yy.c



.PHONY: clean
clean:
	rm parser.yy.c scanner.yy.c
	rm *.o
	rm data_dictionary
