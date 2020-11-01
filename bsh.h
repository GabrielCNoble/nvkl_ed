#ifndef BSH_H
#define BSH_H

#include "neighbor/lib/dstuff/ds_matrix.h"
#include "neighbor/lib/dstuff/ds_vector.h"
#include "neighbor/r_defs.h"
#include <stdint.h>

struct ed_brush_header_t
{
    uint32_t brush_count;
};

struct ed_brush_data_t
{
    uint32_t indice_count;
    uint32_t face_count;
    uint32_t vertice_count;
    uint32_t type;
    vec3_t position;
    vec3_t size;
    mat3_t orientation;
};

struct ed_brush_vertice_data_t
{
    uint32_t count;
    vec3_t vertices[];
};

struct ed_brush_face_data_t
{
    char material[64];
    uint32_t tex_coord_mode;
    float u_scale;
    float v_scale;
    float uv_rotation;
    uint32_t indice_count;
    uint32_t indices[];
};


enum ED_TEX_COORD_MODE
{
    ED_TEX_COORD_MODE_WORLD = 0,
    ED_TEX_COORD_MODE_LOCAL,
};

struct bsh_vertex_t
{
    vec4_t position;
    vec4_t normal;
    vec4_t tangent;
    vec2_t tex_coords;
};

struct ed_brush_face_t
{
    vec3_t normal;
    uint32_t tex_coord_mode;
    uint32_t draw_vertices_start;
    uint32_t indice_count;
    uint32_t *indices;
    uint32_t draw_indices_start;
    uint32_t draw_indices_count;
};

struct ed_brush_edge_t
{
    uint32_t indices[2];
    uint32_t face_a;
    uint32_t face_b;
    uint32_t draw_indices_start;
};

enum BSH_BRUSH_TYPE
{
    BSH_BRUSH_TYPE_CUBE = 0,
    BSH_BRUSH_TYPE_CYLINDER,
    BSH_BRUSH_TYPE_NONE,
};

struct ed_brush_t
{    
    uint32_t type;
    uint32_t subtractive;

    uint32_t max_face_count;
    uint32_t face_count;
    struct ed_brush_face_t *faces;
    
    uint32_t max_edge_count;
    uint32_t edge_count;
    struct ed_brush_edge_t *edges;
    
    uint32_t max_vertice_count;
    uint32_t vertice_count;
    vec3_t *vertices;
    
    uint32_t max_indice_count;
    uint32_t indice_count;
    uint32_t *indices;
    
    uint32_t max_draw_vertice_count;
    uint32_t draw_vertice_count;
    struct r_vertex_t *draw_vertices;
    
    uint32_t max_draw_indice_count;
    uint32_t draw_indice_count;
    uint32_t face_draw_indice_count;
    uint32_t edge_draw_indice_count;
    uint32_t *draw_indices;
    
    struct r_chunk_h draw_vertices_chunk;
    struct r_chunk_h draw_indices_chunk;
    uint32_t draw_indice_start;
    uint32_t vertex_offset;
    
    mat3_t orientation;
    vec3_t scale;
    vec3_t position;
};

struct bsh_brush_h
{
    uint32_t index;
};

#define BSH_BRUSH_HANDLE(index) (struct bsh_brush_h){index}
#define BSH_INVALID_BRUSH_INDEX 0xffffffff
#define BSH_INVALID_BRUSH_HANDLE BSH_BRUSH_HANDLE(BSH_INVALID_BRUSH_INDEX)




void bsh_Init();

void bsh_Shutdown();

struct bsh_brush_h bsh_CreateBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t type);

struct bsh_brush_h bsh_CreateCubeBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale);

struct bsh_brush_h bsh_CreateCylinderBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t vert_count);

struct bsh_brush_h ed_CopyBrush(struct bsh_brush_h handle);

void bsh_DestroyBrush(struct bsh_brush_h handle);

void ed_DestroyAllBrushes();

struct ed_brush_t *ed_GetBrushPointer(struct bsh_brush_h brush);

struct ed_brush_face_t *ed_GetBrushFacePointer(struct bsh_brush_h handle, uint32_t index);

void bsh_TranslateBrush(struct bsh_brush_h handle, vec3_t *translation);

void bsh_RotateBrush(struct bsh_brush_h handle, mat3_t *rotation);

void ed_UpdateBrushEdges(struct bsh_brush_h handle);

void bsh_UpdateBrushDrawData(struct bsh_brush_h handle);

void ed_ExtrudeBrushFace(struct bsh_brush_h handle, uint32_t face_index);

void ed_SerializeBrushes(void **buffer, uint32_t *buffer_size);

void ed_UnserializeBrushes(void *buffer);






#endif // BSH_H









