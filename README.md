update 1
--------
Preliminaries to a collision detection engine (or whatever else). Learning OpenGL, structuring a project,
more C.

Currently working on an entity system.

Then, a basic polygon editor using this entity system, UI utilities, and integrate into file explorer
to quickly edit polygon data, so a start to a "game editor".

build/poly_view is the start of this, a rudimentary viewer for polygons. Polygons can also be read from ascii-grid files.

update 2
--------
Before doing collision, I want to do a mesh loading system, with texturing, asset management, ... Then there
may be some interesting stuff to collide.

Entity system - works relatively well, reasonable frame rate at 10000 cubes with multiple aspects and behaviours + 100 new flying spheres a second.
OpenGL facilities wrappers (handles uploading to vram, etc., managing vertex formats)
A "Renderer" encapsulates shader programs and the uniforms that need to be updated when you draw a mesh with this renderer.
A renderer can draw meshes that have a superset of the vertex format bits specified by the renderer (that probably are needed by its shaders).
Mesh loading
PLY file format querying, I underestimated this. Could just use a library but I want to learn how to write things
like this. Facilities for using a PLY file as a generic database file and querying data to be packed into a given format in application memory.
I think this will be very useful, and could be the format I use for numerical "cluster"/collections data.

I want to learn how to write facilities that do lexing, parsing, custom command formats, configuration files, serialization, etc., seems
neccessary. Learning flex/bison.

priorities:
Image loading
Materials system
Model assets, textured mesh objects (render a Stanford bunny, etc.)
Eventually ...
Later, some sort of wrapper around the entity system to make "game objects", possibly from reading a file format
which loads aspects and fills properties and gets resources. Then could put this format in-line and quickly make a game object
with a model (material,images + mesh, ...), a file with object logic, a rigidbody aspect, ...

