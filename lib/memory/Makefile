
memory.o: $(LIB)/memory.c $(LIB)/small_memory_allocator.c
	$(CC) -o _memory.o -c $(LIB)/memory.c $(CFLAGS)
	$(CC) -o small_memory_allocator.o -c $(LIB)/small_memory_allocator.c $(CFLAGS)
	ld -relocatable _memory.o small_memory_allocator.o -o memory.o

