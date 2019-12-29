rendering.o: $(LIB)/rendering.c $(LIB)/geometry.c $(LIB)/shaders.c $(LIB)/materials.c $(LIB)/textures.c $(LIB)/meshes.c
	$(CC) -o _rendering.o -c $(LIB)/rendering.c $(CFLAGS)
	$(CC) -o geometry.o -c $(LIB)/geometry.c $(CFLAGS)
	$(CC) -o shaders.o -c $(LIB)/shaders.c $(CFLAGS)
	$(CC) -o materials.o -c $(LIB)/materials.c $(CFLAGS)
	$(CC) -o textures.o -c $(LIB)/textures.c $(CFLAGS)
	$(CC) -o meshes.o -c $(LIB)/meshes.c $(CFLAGS)
	ld -relocatable _rendering.o geometry.o shaders.o materials.o textures.o meshes.o -o rendering.o
