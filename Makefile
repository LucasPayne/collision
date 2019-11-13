# Project structure
BUILD_DIR=build
LIB_DIR=lib
INCLUDE_DIR=include
SRC_DIR=src
SCRIPTS_DIR=scripts
SCHEMATICS_DIR=utils/schematics
MAKEFILE=Makefile

# Commands and flags
CC=gcc -rdynamic -Iinclude
CFLAGS=-lglfw3 -lm -lrt -lm -ldl -lX11 -lpthread -lGL

# Lists
FILES=lib/glad.c lib/helper_gl.c lib/helper_input.c

# General Makefile options
default_target: list

# Scripts
.PHONY: list
.SILENT: list
list:
	echo "make <name> to build, make do_<name> to build and run."
	sh $(SCRIPTS_DIR)/list.sh $(SRC_DIR)

.PHONY: new
.SILENT: new
new:
	$(SCRIPTS_DIR)/make_new.sh $(SRC_DIR) $(SCHEMATICS_DIR) $(MAKEFILE)

# Application make rules
grid_test: $(SRC_DIR)/grid_test/grid_test.c lib/grid.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_grid_test: grid_test
	$(BUILD_DIR)/$<

triangles: $(SRC_DIR)/triangles/triangles.cpp $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_triangles: triangles
	$(BUILD_DIR)/$<

regular: $(SRC_DIR)/regular/regular.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_regular: regular
	$(BUILD_DIR)/$<

convex_collision: $(SRC_DIR)/convex_collision/convex_collision.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_convex_collision: convex_collision
	$(BUILD_DIR)/$<

poly_view: $(SRC_DIR)/poly_view/poly_view.c $(LIB_DIR)/data.c $(LIB_DIR)/geometry/shapes.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_poly_view: poly_view
	$(BUILD_DIR)/$<

ascii_polygon_to_regular: $(SRC_DIR)/ascii_polygon_to_regular/ascii_polygon_to_regular.c $(LIB_DIR)/geometry/shapes.c $(LIB_DIR)/data.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_ascii_polygon_to_regular: ascii_polygon_to_regular
	$(BUILD_DIR)/$<
        
entity_test: $(SRC_DIR)/entity_test/entity_test.c $(LIB_DIR)/entity.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_entity_test: entity_test
	$(BUILD_DIR)/$<

gl_entity_test: $(SRC_DIR)/gl_entity_test/gl_entity_test.c $(LIB_DIR)/geometry/shapes.c $(LIB_DIR)/entity.c $(LIB_DIR)/data.c $(LIB_DIR)/collision/collision.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_gl_entity_test: gl_entity_test
	$(BUILD_DIR)/$<

new_entity_test: $(SRC_DIR)/new_entity_test/new_entity_test.c $(LIB_DIR)/entity.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_new_entity_test: new_entity_test
	$(BUILD_DIR)/$<
        
collision: $(SRC_DIR)/collision/collision.c $(LIB_DIR)/entity.c $(LIB_DIR)/geometry/shapes.c $(LIB_DIR)/data.c $(LIB_DIR)/iterator.c $(LIB_DIR)/grid.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_collision: collision
	$(BUILD_DIR)/$<


poly_editor: $(SRC_DIR)/poly_editor/poly_editor.c $(LIB_DIR)/entity.c $(LIB_DIR)/geometry/shapes.c $(LIB_DIR)/data.c $(LIB_DIR)/iterator.c $(LIB_DIR)/grid.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_poly_editor: poly_editor
	$(BUILD_DIR)/$<
