
small_memory_allocator.o: $(LIB)/small_memory_allocator.c
	$(CC) -o $@ -c $^ $(CFLAGS)
_memory.o: $(LIB)/memory.c
	$(CC) -o $@ -c $^ $(CFLAGS)
memory.o: _memory.o small_memory_allocator.o
	ld -relocatable -o $@ $^

