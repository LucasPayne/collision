
ply.o: ply_header.o ply_query.o _ply.o
	ar rvs $@ $^

ply_header.c: $(LIB)/ply_header.l
	flex -o $@ $^
ply_header.o: ply_header.c
	$(CC) -o $@ -c $^ $(CFLAGS)

ply_query.c: $(LIB)/ply_query.l
	flex -o  $@ $^
ply_query.o: ply_query.c
	$(CC) -o $@ -c $^ $(CFLAGS)
	
_ply.o: $(LIB)/ply.c
	$(CC) -o $@ -c $^ $(CFLAGS)
	
