#ifndef BSH_H
#define BSH_H

#include "neighbor/lib/dstuff/ds_matrix.h"
#include "neighbor/lib/dstuff/ds_vector.h"
#include "neighbor/r_common.h"
#include <stdint.h>

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
//    uint32_t vert_count;
    vec3_t face_normal;
    uint32_t draw_vertices_start;
    uint32_t indice_count;
    uint32_t *indices;
    uint32_t draw_indices_start;
    uint32_t draw_indices_count;
    struct ed_brush_face_t *next;
    
    uint32_t tex_coord_mode;
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
    uint32_t polygon_count;
    struct ed_brush_face_t *polygons;
    struct ed_brush_face_t *last_polygon;
    
    uint32_t vertice_count;
    vec3_t *vertices;
    uint32_t face_indice_count;
    uint32_t *face_indices;
    
    uint32_t draw_vertice_count;
    struct r_vertex_t *draw_vertices;
    uint32_t draw_indice_count;
    uint32_t *draw_indices;
    
    struct r_chunk_h draw_vertices_chunk;
    struct r_chunk_h draw_indices_chunk;
    uint32_t start;
    uint32_t count;
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



enum BSH_BSP_TYPE
{
    BSH_BSP_TYPE_SPLITTER = 0,
    BSH_BSP_TYPE_SOLID,
    BSH_BSP_TYPE_EMPTY,
};

enum BSH_POINT_CLASSIFICATION
{
    BSH_POINT_FRONT,
    BSH_POINT_BACK,
    BSH_POINT_COPLANAR = 4,
};

enum BSH_POLYGON_CLASSIFICATION
{
    BSH_POLYGON_FRONT = 0,
    BSH_POLYGON_BACK,
    BSH_POLYGON_STRADDLING,
    BSH_POLYGON_COPLANAR
};

enum BSH_SET_OP
{
    BSH_SET_OP_UNION = 0,
    BSH_SET_OP_INTERSECTION,
    BSH_SET_OP_SUBTRACTION,
};

struct bsh_bsp_t
{
//    struct bsh_bsp_t *back;
//    struct bsh_bsp_t *front;
//    uint32_t polygon_count;
//    uint32_t type;
//    struct bsh_polygon_t *splitter;
//    struct bsh_polygon_t *anti_splitter;
};

void bsh_Init();

void bsh_Shutdown();

struct bsh_brush_h bsh_CreateBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t type);

struct bsh_brush_h bsh_CreateCubeBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale);

struct bsh_brush_h bsh_CreateCylinderBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t vert_count);

void bsh_DestroyBrush(struct bsh_brush_h handle);

struct ed_brush_t *ed_GetBrushPointer(struct bsh_brush_h brush);

struct ed_brush_face_t *ed_GetBrushFacePointer(struct bsh_brush_h handle, uint32_t index);

void bsh_TranslateBrush(struct bsh_brush_h handle, vec3_t *translation);

void bsh_UpdateDrawTriangles(struct bsh_brush_h handle);

void ed_ExtrudeBrushFace(struct bsh_brush_h handle, uint32_t face_index);


//struct bsh_polygon_t *bsh_CopyPolygons(struct bsh_polygon_t *polygons);
//
//void bsh_DestroyPolygons(struct bsh_polygon_t *polygons);
//
//struct bsh_polygon_t *bsh_FlipPolygons(struct bsh_polygon_t *polygons);
//
//uint32_t bsh_SplitEdges(struct bsh_polygon_t *polygon, vec3_t *point, vec3_t *normal, uint32_t *edges, float *times);
//
//uint32_t bsh_SplitPolygon(struct bsh_polygon_t *polygon, vec3_t *point, vec3_t *normal, struct bsh_polygon_t **front, struct bsh_polygon_t **back);
//
//void bsh_SplitPolygons(struct bsh_polygon_t *polygons, vec3_t *point, vec3_t *normal, struct bsh_polygon_t **front, struct bsh_polygon_t **back, struct bsh_polygon_t **coplanar);
//
//struct bsh_polygon_t *bsh_BestSplitter(struct bsh_polygon_t **polygons);
//
//uint32_t bsh_ClassifyPoint(vec3_t *point, vec3_t *plane_point, vec3_t *plane_normal);
//
//uint32_t bsh_ClassifyPolygon(struct bsh_polygon_t *polygon, vec3_t *point, vec3_t *normal);
//
//struct bsh_bsp_t *bsh_BspFromPolygons(struct bsh_polygon_t *polygons);
//
//void bsh_DestroyBsp(struct bsh_bsp_t *bsp);
//
//struct bsh_bsp_t *bsh_IncrementalSetOp(struct bsh_bsp_t *bsp, struct bsh_polygon_t *polygons, uint32_t op);
//
//struct stack_list_t *bsh_GetBrushList();



#endif // BSH_H









