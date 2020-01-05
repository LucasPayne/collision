ply.o: $(LIB)/ply_header.l $(LIB)/ply_query.l $(LIB)/ply.c
	flex -o ply_header.c $(LIB)/ply_header.l
	$(CC) -o ply_header.o -c ply_header.c $(CFLAGS)
	flex -o ply_query.c $(LIB)/ply_query.l
	$(CC) -o ply_query.o -c ply_query.c $(CFLAGS)
	$(CC) -o _ply.o -c $(LIB)/ply.c $(CFLAGS)
	ar rvs ply.o _ply.o ply_header.o ply_query.o
