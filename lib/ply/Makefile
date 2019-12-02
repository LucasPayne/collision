PLY_HEADER=ply_header
PLY_QUERY=ply_query

ply.o: $(LIB)/$(PLY_HEADER).l $(LIB)/$(PLY_QUERY).l $(LIB)/ply.c
	flex -o $(PLY_HEADER).c $(LIB)/$(PLY_HEADER).l
	$(CC) -o $(PLY_HEADER).o -c $(PLY_HEADER).c $(CFLAGS)
	flex -o $(PLY_QUERY).c $(LIB)/$(PLY_QUERY).l
	$(CC) -o $(PLY_QUERY).o -c $(PLY_QUERY).c $(CFLAGS)
	$(CC) -o _ply.o -c $(LIB)/ply.c $(CFLAGS)
	ar rvs ply.o _ply.o $(PLY_HEADER).o $(PLY_QUERY).o
