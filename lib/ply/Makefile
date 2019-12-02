ply.o: $(LIB)/ply_lex.l $(LIB)/ply.c
	flex -o ply_lex.yy.c "$<"
	$(CC) -o ply_lex.yy.o -c ply_lex.yy.c $(CFLAGS)
	$(CC) -o _ply.o -c $(LIB)/ply.c $(CFLAGS)
	ar rvs ply.o _ply.o ply_lex.yy.o
	rm ply_lex.yy.c
