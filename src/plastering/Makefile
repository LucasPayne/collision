
plastering: $(SRC)/plastering.c $(SRC)/include/Plastering.h
	$(CC) -o $@ $(SRC)/plastering.c $(LIB_OBJECTS) $(CFLAGS)

$(SRC)/include/Plastering.h: $(SRC)/plastering.shader_blocks
	$(PROJDIR)/tools/gen_shader_blocks/gen_shader_blocks $^ -c $(SRC)/include -g $(SRC)/glsl





