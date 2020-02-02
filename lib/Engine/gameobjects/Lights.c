/*================================================================================
    Lights
        DirectionalLight
        PointLight
================================================================================*/
#include "Engine.h"

AspectType DirectionalLight_TYPE_ID;
void DirectionalLight_init(DirectionalLight *directional_light, float cr, float cg, float cb, float ca, float shadow_width, float shadow_height, float shadow_depth)
{
    directional_light->color = new_vec4(cr, cg, cb, ca);
    directional_light->shadow_width = shadow_width;
    directional_light->shadow_height = shadow_height;
    directional_light->shadow_depth = shadow_depth;
}
vec3 DirectionalLight_direction(DirectionalLight *directional_light)
{
    // The direction of the light is in the light entity's local z direction.
    return Transform_relative_direction(get_sibling_aspect(directional_light, Transform), new_vec3(0,0,1));
}

AspectType PointLight_TYPE_ID;
void PointLight_init(PointLight *point_light, float linear_attenuation, float quadratic_attenuation, float cubic_attenuation, float cr, float cg, float cb, float ca)
{
    point_light->color = new_vec4(cr, cg, cb, ca);
    point_light->linear_attenuation = linear_attenuation;
    point_light->quadratic_attenuation = quadratic_attenuation;
    point_light->cubic_attenuation = cubic_attenuation;
}

