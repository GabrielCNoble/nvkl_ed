#ifndef BSH_H
#define BSH_H

#include "neighbor/lib/dstuff/ds_matrix.h"
#include "neighbor/lib/dstuff/ds_vector.h"
#include <stdint.h>

struct bsh_vertex_t
{
    vec3_t position;
    vec3_t normal;
    vec3_t tangent;
    vec2_t tex_coords;
};

struct bsh_polygon_t
{
    uint32_t vert_count;
    struct bsh_vertex_t *vertices;
    struct bsh_polygon_t *next;
};

struct bsh_brush_t
{
    struct bsh_brush_t *next;
    struct bsh_brush_t *prev;
    
    uint32_t type;
    uint32_t subtractive;
    uint32_t polygon_count;
    struct bsh_polygon_t *polygons;
    struct bsh_polygon_t *last_polygon;
    
    mat3_t orientation;
    vec3_t scale;
    vec3_t position;
};


enum BSH_BRUSH_TYPE
{
    BSH_BRUSH_TYPE_CUBE = 0,
    BSH_BRUSH_TYPE_CYLINDER,
};


struct bsh_brush_t *bsh_CreateBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t type);

struct bsh_brush_t *bsh_CreateCubeBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale);

struct bsh_brush_t *bsh_CreateCylinderBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t vert_count);

void bsh_DestroyBrush(struct bsh_brush_t *brush);

struct bsh_brush_t *bsh_GetBrushList();

#endif // BSH_H









