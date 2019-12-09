# This library is split up into different modules for organization purposes.
# rendering.o: $(LIB)/rendering.c $(LIB)/shader_bookkeeping.c $(LIB)/meshes.c
rendering.o: $(LIB)/rendering.c $(LIB)/shader_bookkeeping.c
	# echo "Building $@"
	# echo "with $^"
	# sleep 5
	$(CC) -o shader_bookkeeping.o -c $(LIB)/shader_bookkeeping.c
	$(CC) -o ___rendering.o -c $(LIB)/rendering.c
	ld -relocatable ___rendering.o shader_bookkeeping.o -o rendering.o
	# $(CC) -o meshes.o -c $(LIB)/meshes.c
	# Partial linking, also -r, -i incremental link
	# ld -relocatable ___rendering.o shader_bookkeeping.o meshes.o -o rendering.o
