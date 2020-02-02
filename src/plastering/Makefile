
plastering: $(SRC)/plastering.c
	$(PROJDIR)/tools/gen_shader_blocks/gen_shader_blocks $(SRC)/plastering.shader_blocks -c $(SRC)/include -g $(SRC)/glsl
	$(CC) -o $@ $(SRC)/plastering.c $(LIB_OBJECTS) $(CFLAGS)
