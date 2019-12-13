# This library is split up into different modules for organization purposes.
# rendering.o: $(LIB)/rendering.c $(LIB)/shader_bookkeeping.c $(LIB)/meshes.c
	# $(CC) -o meshes.o -c $(LIB)/meshes.c
	# Partial linking, also -r, -i incremental link
	# ld -relocatable ___rendering.o shader_bookkeeping.o meshes.o -o rendering.o
	#
	# echo "Building $@"
	# echo "with $^"
	# sleep 5


rendering.o: $(LIB)/rendering.c $(LIB)/shader_bookkeeping.c $(LIB)/textures.c
	$(CC) -o shader_bookkeeping.o -c $(LIB)/shader_bookkeeping.c $(CFLAGS)
	$(CC) -o ___rendering.o -c $(LIB)/rendering.c $(CFLAGS)
	$(CC) -o textures.o -c $(LIB)/textures.c $(CFLAGS)
	ld -relocatable ___rendering.o shader_bookkeeping.o textures.o -o rendering.o
