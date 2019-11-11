
BUILD_DIR=build
LIB_DIR=lib
INCLUDE_DIR=include
SRC_DIR=src
SCRIPTS_DIR=scripts
SCHEMATICS_DIR=utils/schematics
# Macro for this?
MAKEFILE=Makefile

CC=gcc -rdynamic -Iinclude
CFLAGS=-lglfw3 -lm -lrt -lm -ldl -lX11 -lpthread -lGL

FILES=lib/glad.c lib/helper_gl.c lib/helper_input.c

# relocate:
# 	cat "$(INCLUDE_DIR)/project_definitions.h" | sed 's/^#define PROJECT_DIR.*/#define PROJECT_DIR thingy'

default_target: list

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

# .PHONY .SILENT: list // works?
.PHONY: list
.SILENT: list
list:
	echo "make <name> to build, make do_<name> to build and run."
	sh $(SCRIPTS_DIR)/list.sh $(SRC_DIR)

.PHONY: new
.SILENT: new
new:
	# Macro for this Makefile path?
	$(SCRIPTS_DIR)/make_new.sh $(SRC_DIR) $(SCHEMATICS_DIR) $(MAKEFILE)

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
        
collision: $(SRC_DIR)/collision/collision.c $(LIB_DIR)/entity.c $(LIB_DIR)/geometry/shapes.c $(LIB_DIR)/data.c $(LIB_DIR)/iterator.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_collision: collision
	$(BUILD_DIR)/$<
        
