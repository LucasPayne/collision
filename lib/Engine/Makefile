Engine.o: _Engine.o helper.o gameobjects.o testing.o game_renderer.o
	ld -relocatable -o $@ $^

game_renderer.o: $(LIB)/game_renderer/game_renderer.c
	$(CC) -o $@ -c $^ $(CFLAGS)
gameobjects.o: $(LIB)/gameobjects/gameobjects.c
	$(CC) -o $@ -c $^ $(CFLAGS)

testing.o: $(LIB)/utils/testing.c
	$(CC) -o $@ -c $^ $(CFLAGS)
helper.o: $(LIB)/utils/helper.c
	$(CC) -o $@ -c $^ $(CFLAGS)

_Engine.o: $(LIB)/Engine.c
	$(CC) -o $@ -c $^ $(CFLAGS)
