
BUILD_DIR=build
LIB_DIR=lib
INCLUDE_DIR=include
SRC_DIR=src
SCRIPTS_DIR=scripts
SCHEMATICS_DIR=utils/schematics
# Macro for this?
MAKEFILE=Makefile

CC=g++ -rdynamic -Iinclude
CFLAGS=-lglfw3 -lm -lrt -lm -ldl -lX11 -lpthread -lGL

FILES=lib/glad.c lib/helper_gl.cpp lib/helper_input.cpp

# relocate:
# 	cat "$(INCLUDE_DIR)/project_definitions.h" | sed 's/^#define PROJECT_DIR.*/#define PROJECT_DIR thingy'

default_target: list

grid_test: $(SRC_DIR)/grid_test/grid_test.cpp lib/grid.cpp $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_grid_test: grid_test
	$(BUILD_DIR)/$<

triangles: $(SRC_DIR)/triangles/triangles.cpp $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_triangles: triangles
	$(BUILD_DIR)/$<

regular: $(SRC_DIR)/regular/regular.cpp $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_regular: regular
	$(BUILD_DIR)/$<

convex_collision: $(SRC_DIR)/convex_collision/convex_collision.cpp $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_convex_collision: convex_collision
	$(BUILD_DIR)/$<

shapes_test: $(SRC_DIR)/shapes_test/shapes_test.cpp $(LIB_DIR)/data.c $(LIB_DIR)/shapes/shapes.cpp $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_shapes_test: shapes_test
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

poly_view: $(SRC_DIR)/poly_view/poly_view.cpp $(LIB_DIR)/data.c $(LIB_DIR)/shapes/shapes.cpp $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_poly_view: poly_view
	$(BUILD_DIR)/$<

ascii_polygon_to_regular: $(SRC_DIR)/ascii_polygon_to_regular/ascii_polygon_to_regular.cpp $(LIB_DIR)/shapes/shapes.cpp $(LIB_DIR)/data.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_ascii_polygon_to_regular: ascii_polygon_to_regular
	$(BUILD_DIR)/$<
        
