# ply.o: $(LIB)/ply_lex.l $(LIB)/ply_query.l $(LIB)/ply.c
# 	flex -o ply_lex.yy.c $(LIB)/ply_lex.l
# 	$(CC) -o ply_lex.yy.o -c ply_lex.yy.c $(CFLAGS)
# 	$(CC) -o _ply.o -c $(LIB)/ply.c $(CFLAGS)
# 	ar rvs ply.o _ply.o ply_lex.yy.o

ply.o: $(LIB)/ply_lex.l $(LIB)/ply_query.l $(LIB)/ply.c
	flex -o ply_lex.yy.c $(LIB)/ply_lex.l
	$(CC) -o ply_lex.yy.o -c ply_lex.yy.c $(CFLAGS)
	flex -o ply_query.yy.c $(LIB)/ply_query.l
	$(CC) -o ply_query.yy.o -c ply_query.yy.c $(CFLAGS)
	$(CC) -o _ply.o -c $(LIB)/ply.c $(CFLAGS)
	ar rvs ply.o _ply.o ply_lex.yy.o ply_query.yy.o
	ar rvs ply.o _ply.o ply_lex.yy.o

