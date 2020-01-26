M_CFLAGS=$(CFLAGS) -I$(LIB)/include

data_dictionary.o: _data_dictionary.o data_dictionary_scanner.o data_dictionary_parser.o data_dictionary_implementation.o
	ld -relocatable -o $@ $^
_data_dictionary.o: $(LIB)/data_dictionary.c
	$(CC) -o $@ -c $< $(M_CFLAGS)
data_dictionary_implementation.o: $(LIB)/data_dictionary_implementation.c
	$(CC) -o $@ -c $< $(M_CFLAGS)
data_dictionary_scanner.o: data_dictionary_scanner.yy.c
	$(CC) -o $@ -c $< $(M_CFLAGS)
data_dictionary_parser.o: data_dictionary_parser.yy.c
	$(CC) -o $@ -c $< $(M_CFLAGS)
data_dictionary_scanner.yy.c: $(LIB)/data_dictionary_scanner.l data_dictionary_parser.yy.c
	# This depends on the parser source, since the tokens header file is generated with it.
	flex -o $@ $(LIB)/data_dictionary_scanner.l
data_dictionary_parser.yy.c: $(LIB)/data_dictionary_parser.y
	bison -o $@ $< --defines=tokens.yy.h --name-prefix=dd_yy
