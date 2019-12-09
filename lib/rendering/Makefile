# This library is split up into different modules for organization purposes.
rendering.o: $(LIB)/rendering.c $(LIB)/shader_bookkeeping.c
	$(CC) -o shader_bookkeeping.o -c $(LIB)/shader_bookkeeping.c
	$(CC) -o ___rendering.o -c $(LIB)/rendering.c
	# Partial linking, also -r, -i incremental link
	ld -relocatable ___rendering.o shader_bookkeeping.o -o rendering.o
