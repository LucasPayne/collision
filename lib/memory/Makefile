
memory.o: $(LIB)/memory.c $(LIB)/small_memory_allocator.c
    $(CC) -o _memory.o $(LIB)/memory.c $(CFLAGS)
    $(CC) -o small_memory_allocator.o $(LIB)/memory.c $(CFLAGS)
    ld -relocatable $@ _memory.o small_memory_allocator.o

