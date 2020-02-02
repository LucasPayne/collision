Engine.o: _Engine.o helper.o gameobjects.o testing.o game_renderer.o
	ld -relocatable -o $@ $^

game_renderer.o: $(LIB)/game_renderer/game_renderer.c
	$(CC) -o $@ -c $^ $(CFLAGS)

Transform.o: $(LIB)/gameobjects/Transform.c
	$(CC) -o $@ -c $^ $(CFLAGS)
Body.o: $(LIB)/gameobjects/Body.c
	$(CC) -o $@ -c $^ $(CFLAGS)
Logic.o: $(LIB)/gameobjects/Logic.c
	$(CC) -o $@ -c $^ $(CFLAGS)
Input.o: $(LIB)/gameobjects/Input.c
	$(CC) -o $@ -c $^ $(CFLAGS)
Lights.o: $(LIB)/gameobjects/Lights.c
	$(CC) -o $@ -c $^ $(CFLAGS)
Camera.o: $(LIB)/gameobjects/Camera.c
	$(CC) -o $@ -c $^ $(CFLAGS)
gameobjects.o: _gameobjects.o Transform.o Body.o Logic.o Input.o Lights.o Camera.o
	ld -relocatable -o $@ $^

_gameobjects.o: $(LIB)/gameobjects/gameobjects.c
	$(CC) -o $@ -c $^ $(CFLAGS)

testing.o: $(LIB)/utils/testing.c
	$(CC) -o $@ -c $^ $(CFLAGS)
helper.o: $(LIB)/utils/helper.c
	$(CC) -o $@ -c $^ $(CFLAGS)

_Engine.o: $(LIB)/Engine.c
	$(CC) -o $@ -c $^ $(CFLAGS)

