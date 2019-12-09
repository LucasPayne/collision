# This library consists of lexer modules which are compiled separately and put in an archive.
# The dictionary.h header declares all interface to these modules, so the user should not care
# which part of the library the functionality comes from.

dictionary.o: $(LIB)/dictionary_read.l $(LIB)/dictionary_query.l $(LIB)/dictionary_base.c
	flex -o dictionary_read.yy.c $(LIB)/dictionary_read.l
	$(CC) -o dictionary_read.o -c dictionary_read.yy.c $(CFLAGS)
	flex -o dictionary_query.yy.c $(LIB)/dictionary_query.l
	$(CC) -o dictionary_query.o -c dictionary_query.yy.c $(CFLAGS)
	$(CC) -o dictionary_base.o -c $(LIB)/dictionary_base.c $(CFLAGS)
	ld -relocatable -o dictionary.o dictionary_base.o dictionary_read.o dictionary_query.o

# This manages to put everything in a ".o" library, but interreferences aren't resolved. So, above partial link works.
# ar rvs dictionary.o dictionary_base.o dictionary_read.o dictionary_query.o
