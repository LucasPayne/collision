rendering.o: _rendering.o geometry.o shaders.o materials.o textures.o meshes.o
	ld -relocatable -o $@ $^
meshes.o: $(LIB)/meshes.c
	$(CC) -o $@ -c $^ $(CFLAGS)
textures.o: $(LIB)/textures.c
	$(CC) -o $@ -c $^ $(CFLAGS)
materials.o: $(LIB)/materials.c
	$(CC) -o $@ -c $^ $(CFLAGS)
shaders.o: $(LIB)/shaders.c
	$(CC) -o $@ -c $^ $(CFLAGS)
geometry.o: $(LIB)/geometry.c
	$(CC) -o $@ -c $^ $(CFLAGS)
_rendering.o: $(LIB)/rendering.c
	$(CC) -o $@ -c $^ $(CFLAGS)
