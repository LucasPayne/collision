
#note: should probably export $(INCLUDE) and depend on module interface headers.

# Using M_CFLAGS, this module adds its own additional flags.


# data_dictionary.o: $(LIB)/data_dictionary_scanner.l $(LIB)/data_dictionary_parser.y $(LIB)/data_dictionary_implementation.c
# 	bison -o data_dictionary_parser.yy.c $(LIB)/data_dictionary_parser.y --defines=tokens.yy.h --name-prefix=dd_yy
# 	flex -o data_dictionary_scanner.yy.c $(LIB)/data_dictionary_scanner.l
# 	$(CC) -c data_dictionary_parser.yy.c $(M_CFLAGS)
# 	$(CC) -c data_dictionary_scanner.yy.c $(M_CFLAGS)
# 	$(CC) -o data_dictionary_implementation.o -c $(LIB)/data_dictionary_implementation.c $(M_CFLAGS)
# 	$(CC) -o DEBUG.o -c $(LIB)/DEBUG.c $(M_CFLAGS)
# 	ld -relocatable -o $@ data_dictionary_implementation.o
# 	# ld -relocatable -o $@ data_dictionary_scanner.yy.o data_dictionary_parser.yy.o data_dictionary_implementation.o


M_CFLAGS=$(CFLAGS) -I$(LIB)/include
#------------------------------------------------------------------------
data_dictionary.o: $(LIB)/include/data_dictionary_implementation.h $(LIB)/data_dictionary.c $(LIB)/data_dictionary_implementation.c $(LIB)/data_dictionary_scanner.l $(LIB)/data_dictionary_parser.y
	bison -o data_dictionary_parser.yy.c $(LIB)/data_dictionary_parser.y --defines=tokens.yy.h --name-prefix=dd_yy
	flex -o data_dictionary_scanner.yy.c $(LIB)/data_dictionary_scanner.l
	$(CC) -c data_dictionary_parser.yy.c $(M_CFLAGS)
	$(CC) -c data_dictionary_scanner.yy.c $(M_CFLAGS)
	$(CC) -o _data_dictionary.o -c $(LIB)/data_dictionary.c $(M_CFLAGS)
	$(CC) -o data_dictionary_implementation.o -c $(LIB)/data_dictionary_implementation.c $(M_CFLAGS)
	ld -relocatable -o $@ _data_dictionary.o data_dictionary_implementation.o data_dictionary_scanner.yy.o data_dictionary_parser.yy.o


