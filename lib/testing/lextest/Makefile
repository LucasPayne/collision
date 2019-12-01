lextest.o: $(LIB)/lextest.l
	flex -o lextest.yy.c "$<"
	$(CC) -o "$@" -c lextest.yy.c $(CFLAGS)
	rm lextest.yy.c
