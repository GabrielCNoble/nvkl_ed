#include "bsh.h"
#include "neighbor/lib/dstuff/ds_mem.h"
#include <string.h>
#include <stdlib.h>

struct bsh_brush_t *bsh_brushes = NULL;
struct bsh_brush_t *bsh_last_brush = NULL;


struct bsh_vertex_t bsh_cube_brush_verts[] = 
{
    /* +X */
    (struct bsh_vertex_t){.position = { 0.5, 0.5, 0.5}, .normal = { 1.0, 0.0, 0.0}, .tangent = { 0.0, 0.0, 1.0}, .tex_coords = {0.0, 0.0}},
    (struct bsh_vertex_t){.position = { 0.5,-0.5, 0.5}, .normal = { 1.0, 0.0, 0.0}, .tangent = { 0.0, 0.0, 1.0}, .tex_coords = {0.0, 1.0}},
    (struct bsh_vertex_t){.position = { 0.5,-0.5,-0.5}, .normal = { 1.0, 0.0, 0.0}, .tangent = { 0.0, 0.0, 1.0}, .tex_coords = {1.0, 1.0}},
    (struct bsh_vertex_t){.position = { 0.5, 0.5,-0.5}, .normal = { 1.0, 0.0, 0.0}, .tangent = { 0.0, 0.0, 1.0}, .tex_coords = {1.0, 0.0}},
    
    /* -X */
    (struct bsh_vertex_t){.position = {-0.5, 0.5,-0.5}, .normal = {-1.0, 0.0, 0.0}, .tangent = { 0.0, 0.0,-1.0}, .tex_coords = {0.0, 0.0}},
    (struct bsh_vertex_t){.position = {-0.5,-0.5,-0.5}, .normal = {-1.0, 0.0, 0.0}, .tangent = { 0.0, 0.0,-1.0}, .tex_coords = {0.0, 1.0}},
    (struct bsh_vertex_t){.position = {-0.5,-0.5, 0.5}, .normal = {-1.0, 0.0, 0.0}, .tangent = { 0.0, 0.0,-1.0}, .tex_coords = {1.0, 1.0}},
    (struct bsh_vertex_t){.position = {-0.5, 0.5, 0.5}, .normal = {-1.0, 0.0, 0.0}, .tangent = { 0.0, 0.0,-1.0}, .tex_coords = {1.0, 0.0}},
    
    /* +Y */
    (struct bsh_vertex_t){.position = {-0.5, 0.5,-0.5}, .normal = { 0.0,-1.0, 0.0}, .tangent = {-1.0, 0.0, 1.0}, .tex_coords = {0.0, 0.0}},
    (struct bsh_vertex_t){.position = {-0.5, 0.5, 0.5}, .normal = { 0.0,-1.0, 0.0}, .tangent = {-1.0, 0.0, 1.0}, .tex_coords = {0.0, 1.0}},
    (struct bsh_vertex_t){.position = { 0.5, 0.5, 0.5}, .normal = { 0.0,-1.0, 0.0}, .tangent = {-1.0, 0.0, 1.0}, .tex_coords = {1.0, 1.0}},
    (struct bsh_vertex_t){.position = { 0.5, 0.5,-0.5}, .normal = { 0.0,-1.0, 0.0}, .tangent = {-1.0, 0.0, 1.0}, .tex_coords = {1.0, 0.0}},
    
    /* -Y */
    (struct bsh_vertex_t){.position = {-0.5,-0.5, 0.5}, .normal = { 0.0, 1.0, 0.0}, .tangent = { 1.0, 0.0, 1.0}, .tex_coords = {0.0, 0.0}},
    (struct bsh_vertex_t){.position = {-0.5,-0.5,-0.5}, .normal = { 0.0, 1.0, 0.0}, .tangent = { 1.0, 0.0, 1.0}, .tex_coords = {0.0, 1.0}},
    (struct bsh_vertex_t){.position = { 0.5,-0.5,-0.5}, .normal = { 0.0, 1.0, 0.0}, .tangent = { 1.0, 0.0, 1.0}, .tex_coords = {1.0, 1.0}},
    (struct bsh_vertex_t){.position = { 0.5,-0.5, 0.5}, .normal = { 0.0, 1.0, 0.0}, .tangent = { 1.0, 0.0, 1.0}, .tex_coords = {1.0, 0.0}},
    
    /* +Z */
    (struct bsh_vertex_t){.position = {-0.5, 0.5, 0.5}, .normal = { 0.0, 0.0, 1.0}, .tangent = { 1.0, 0.0, 0.0}, .tex_coords = {0.0, 0.0}},
    (struct bsh_vertex_t){.position = {-0.5,-0.5, 0.5}, .normal = { 0.0, 0.0, 1.0}, .tangent = { 1.0, 0.0, 0.0}, .tex_coords = {0.0, 1.0}},
    (struct bsh_vertex_t){.position = { 0.5,-0.5, 0.5}, .normal = { 0.0, 0.0, 1.0}, .tangent = { 1.0, 0.0, 0.0}, .tex_coords = {1.0, 1.0}},
    (struct bsh_vertex_t){.position = { 0.5, 0.5, 0.5}, .normal = { 0.0, 0.0, 1.0}, .tangent = { 1.0, 0.0, 0.0}, .tex_coords = {1.0, 0.0}},
    
    /* -Z */
    (struct bsh_vertex_t){.position = {-0.5,-0.5,-0.5}, .normal = { 0.0, 0.0,-1.0}, .tangent = {-1.0, 0.0, 0.0}, .tex_coords = {0.0, 0.0}},
    (struct bsh_vertex_t){.position = {-0.5, 0.5,-0.5}, .normal = { 0.0, 0.0,-1.0}, .tangent = {-1.0, 0.0, 0.0}, .tex_coords = {0.0, 1.0}},
    (struct bsh_vertex_t){.position = { 0.5, 0.5,-0.5}, .normal = { 0.0, 0.0,-1.0}, .tangent = {-1.0, 0.0, 0.0}, .tex_coords = {1.0, 1.0}},
    (struct bsh_vertex_t){.position = { 0.5,-0.5,-0.5}, .normal = { 0.0, 0.0,-1.0}, .tangent = {-1.0, 0.0, 0.0}, .tex_coords = {1.0, 0.0}},
};

struct bsh_brush_t *bsh_CreateBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t type)
{
    struct bsh_brush_t *brush;
    brush = mem_Calloc(1, sizeof(struct bsh_brush_t ));
    
    brush->type = type;
    brush->subtractive = 0;
    brush->position = *position;
    brush->scale = *scale;
    brush->orientation = *orientation;
    brush->polygons = NULL;
    
    if(!bsh_brushes)
    {
        bsh_brushes = brush;
    }
    else
    {
        bsh_last_brush->next = brush;
        brush->prev = bsh_last_brush;
    }
    
    bsh_last_brush = brush;
    
    return brush;
}

struct bsh_brush_t *bsh_CreateCubeBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale)
{
    struct bsh_brush_t *brush;
    struct bsh_polygon_t *polygon;
    struct bsh_vertex_t *vertices = bsh_cube_brush_verts;
    
    brush = bsh_CreateBrush(position, orientation, scale, BSH_BRUSH_TYPE_CUBE);
    
    for(uint32_t polygon_index = 0; polygon_index < 6; polygon_index++)
    {
//        vertices = bsh_cube_brush_verts + polygon_index * sizeof(struct bsh_vertex_t) * 4;
        
        polygon = mem_Calloc(1, sizeof(struct bsh_polygon_t));
        polygon->next = NULL;
        polygon->vert_count = 4;
        polygon->vertices = mem_Calloc(4, sizeof(struct bsh_vertex_t));
        
        for(uint32_t vert_index = 0; vert_index < polygon->vert_count; vert_index++)
        {
            vec3_t_add(&polygon->vertices[vert_index].position, &brush->position, &vertices[vert_index].position);
        }
        
        if(!brush->polygons)
        {
            brush->polygons = polygon;
        }
        else
        {
            brush->last_polygon->next = polygon;
        }
        brush->last_polygon = polygon;
        
        vertices += 4;
    }
    
    return brush;
}

struct bsh_brush_t *bsh_CreateCylinderBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t vert_count)
{
    return NULL;
}

void bsh_DestroyBrush(struct bsh_brush_t *brush)
{
    struct bsh_polygon_t *next_polygon;
    
    if(brush)
    {        
        while(brush->polygons)
        {
            next_polygon = brush->polygons->next;
            mem_Free(brush->polygons->vertices);
            mem_Free(brush->polygons);
            brush->polygons = next_polygon;
        }
        
        if(brush->prev)
        {
            brush->prev->next = brush->next;
        }
        else
        {
            bsh_brushes = brush->next;
            bsh_brushes->prev = NULL;
        }
        
        if(brush->next)
        {
            brush->next->prev = brush->prev;
        }
        else
        {
            bsh_last_brush = brush->prev;
            bsh_last_brush->next = NULL;
        }
        
        mem_Free(brush);
    }
}

struct bsh_brush_t *bsh_GetBrushList()
{
    return bsh_brushes;
}





