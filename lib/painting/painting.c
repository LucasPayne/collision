/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
#include "data_dictionary.h"
#include "resources.h"
#include "rendering.h"
#include "painting.h"

static DataDictionary *g_painting_resources = NULL;

// Initialize the painting module. This opens the global dictionary with painting resources.
void painting_init(void)
{
    // need to do this in a better way, allowing a library to reference its own location, as a compile option defining a symbol.
    g_painting_resources = dd_fopen("/home/lucas/collision/lib/painting/painting_resources.dd");
}

/*--------------------------------------------------------------------------------
Painting function variants:
*   : scalar parameters
*_v : vector parameters
*_c : colors given by string code
Variants can be combined.
--------------------------------------------------------------------------------*/
void paint_line_v(vec3 a, vec3 b, vec4 color)
{
    gm_lines(VERTEX_FORMAT_3);
    attribute_3f(Position, a.vals[0],a.vals[1],a.vals[2]);
    attribute_3f(Position, b.vals[0],b.vals[1],b.vals[2]);
    Geometry g = gm_done();
    ResourceHandle mat = Material_create("Materials/flat_color");
    material_set_property_vec4(resource_data(Material, mat), "flat_color", color);
    gm_draw(g, resource_data(Material, mat));
    gm_free(g);
    destroy_resource_handle(&mat);
}
void paint_line(float ax, float ay, float az, float bx, float by, float bz, float cr, float cg, float cb, float ca)
{
    paint_line_v(new_vec3(ax, ay, az), new_vec3(bx, by, bz), new_vec4(cr, cg, cb, ca));
}
void paint_line_c(float ax, float ay, float az, float bx, float by, float bz, char *color_str)
{
    paint_line_v(new_vec3(ax, ay, az), new_vec3(bx, by, bz), str_to_color_key(color_str));
}
void paint_line_cv(vec3 a, vec3 b, vec4 color)
{

}
