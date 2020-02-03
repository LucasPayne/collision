/*--------------------------------------------------------------------------------
    Fonts module
----------------
    Monospace, ASCII, raster bitmap fonts. This module defines the Font resource.
--------------------------------------------------------------------------------*/

ResourceType Font_RTID;

typedef struct /* Resource */ Texture_s {
    GLuint texture_id;
} Texture;
void Texture_load(void *resource, char *path);




