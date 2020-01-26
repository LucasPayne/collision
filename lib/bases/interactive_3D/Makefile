interactive_3D.o: _interactive_3D.o helper.o gameobjects.o testing.o
	ld -relocatable -o $@ $^

testing.o: $(LIB)/testing.c
	$(CC) -o $@ -c $^ $(CFLAGS)
helper.o: $(LIB)/helper.c
	$(CC) -o $@ -c $^ $(CFLAGS)
gameobjects.o: $(LIB)/gameobjects.c
	$(CC) -o $@ -c $^ $(CFLAGS)
_interactive_3D.o: $(LIB)/interactive_3D.c
	$(CC) -o $@ -c $^ $(CFLAGS)
