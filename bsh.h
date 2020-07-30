#ifndef BSH_H
#define BSH_H

#include "neighbor/lib/dstuff/ds_matrix.h"
#include "neighbor/lib/dstuff/ds_vector.h"
#include "neighbor/r_common.h"
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
    vec3_t face_normal;
    struct bsh_vertex_t *vertices;
    struct bsh_polygon_t *next;
};

enum BSH_BRUSH_TYPE
{
    BSH_BRUSH_TYPE_CUBE = 0,
    BSH_BRUSH_TYPE_CYLINDER,
    BSH_BRUSH_TYPE_NONE,
};

struct bsh_brush_t
{    
    uint32_t type;
    uint32_t subtractive;
    uint32_t polygon_count;
    struct bsh_polygon_t *polygons;
    struct bsh_polygon_t *last_polygon;
    
    struct r_chunk_h vertices;
    struct r_chunk_h indices;
    uint32_t start;
    uint32_t count;
    
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
    struct bsh_bsp_t *back;
    struct bsh_bsp_t *front;
    uint32_t polygon_count;
    uint32_t type;
    struct bsh_polygon_t *splitter;
    struct bsh_polygon_t *anti_splitter;
};

void bsh_Init();

void bsh_Shutdown();

struct bsh_brush_h bsh_CreateBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t type);

struct bsh_brush_h bsh_CreateCubeBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale);

struct bsh_brush_h bsh_CreateCylinderBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t vert_count);

void bsh_DestroyBrush(struct bsh_brush_h handle);

struct bsh_brush_t *bsh_GetBrushPointer(struct bsh_brush_h brush);

//void bsh_TranslateBrush(struct bsh_brush_t *brush, vec3_t *translation);

//void bsh_SetBrushPosition(struct bsh_brush_t *brush, vec3_t *position);

void bsh_TriangulatePolygons(struct bsh_brush_h handle);


struct bsh_polygon_t *bsh_CopyPolygons(struct bsh_polygon_t *polygons);

void bsh_DestroyPolygons(struct bsh_polygon_t *polygons);

struct bsh_polygon_t *bsh_FlipPolygons(struct bsh_polygon_t *polygons);

uint32_t bsh_SplitEdges(struct bsh_polygon_t *polygon, vec3_t *point, vec3_t *normal, uint32_t *edges, float *times);

uint32_t bsh_SplitPolygon(struct bsh_polygon_t *polygon, vec3_t *point, vec3_t *normal, struct bsh_polygon_t **front, struct bsh_polygon_t **back);

void bsh_SplitPolygons(struct bsh_polygon_t *polygons, vec3_t *point, vec3_t *normal, struct bsh_polygon_t **front, struct bsh_polygon_t **back, struct bsh_polygon_t **coplanar);

struct bsh_polygon_t *bsh_BestSplitter(struct bsh_polygon_t **polygons);

uint32_t bsh_ClassifyPoint(vec3_t *point, vec3_t *plane_point, vec3_t *plane_normal);

uint32_t bsh_ClassifyPolygon(struct bsh_polygon_t *polygon, vec3_t *point, vec3_t *normal);

struct bsh_bsp_t *bsh_BspFromPolygons(struct bsh_polygon_t *polygons);

void bsh_DestroyBsp(struct bsh_bsp_t *bsp);

struct bsh_bsp_t *bsh_IncrementalSetOp(struct bsh_bsp_t *bsp, struct bsh_polygon_t *polygons, uint32_t op);

struct stack_list_t *bsh_GetBrushList();



#endif // BSH_H









