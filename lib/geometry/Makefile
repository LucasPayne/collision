geometry.o: _geometry.o polyhedra.o testing.o
	ld -relocatable -o $@ $^

_geometry.o: $(LIB)/geometry.c
	$(CC) -o $@ -c $^ $(CFLAGS)
polyhedra.o: $(LIB)/polyhedra.c
	$(CC) -o $@ -c $^ $(CFLAGS)
testing.o: $(LIB)/testing.c
	$(CC) -o $@ -c $^ $(CFLAGS)

