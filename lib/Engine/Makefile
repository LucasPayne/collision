Engine.o: _Engine.o helper.o gameobjects.o testing.o game_renderer.o scenes.o collision.o
	ld -relocatable -o $@ $^
_Engine.o: $(LIB)/Engine.c
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
Text.o: $(LIB)/gameobjects/Text.c
	$(CC) -o $@ -c $^ $(CFLAGS)
RigidBody.o: $(LIB)/gameobjects/RigidBody.c
	$(CC) -o $@ -c $^ $(CFLAGS)
_gameobjects.o: $(LIB)/gameobjects/gameobjects.c
	$(CC) -o $@ -c $^ $(CFLAGS)
gameobjects.o: _gameobjects.o Transform.o Body.o Logic.o Input.o Lights.o Camera.o Text.o RigidBody.o
	ld -relocatable -o $@ $^

shadows.o: $(LIB)/game_renderer/shadows.c
	$(CC) -o $@ -c $^ $(CFLAGS)
_game_renderer.o: $(LIB)/game_renderer/game_renderer.c
	$(CC) -o $@ -c $^ $(CFLAGS)
game_renderer.o: _game_renderer.o shadows.o
	ld -relocatable -o $@ $^

_collision.o: $(LIB)/collision/collision.c
	$(CC) -o $@ -c $^ $(CFLAGS)
dynamics.o: $(LIB)/collision/dynamics.c
	$(CC) -o $@ -c $^ $(CFLAGS)
collision.o: _collision.o dynamics.o
	ld -relocatable -o $@ $^

testing.o: $(LIB)/utils/testing.c
	$(CC) -o $@ -c $^ $(CFLAGS)
helper.o: $(LIB)/utils/helper.c
	$(CC) -o $@ -c $^ $(CFLAGS)

scenes.o: $(LIB)/scenes/scenes.c
	$(CC) -o $@ -c $^ $(CFLAGS)

