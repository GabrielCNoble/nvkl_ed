#ifndef ED_BSP_H
#define ED_BSP_H

#include "neighbor/lib/dstuff/ds_vector.h"

enum ED_BSP_TYPE
{
    BSH_BSP_TYPE_SPLITTER = 0,
    BSH_BSP_TYPE_SOLID,
    BSH_BSP_TYPE_EMPTY,
};

enum ED_POINT_CLASSIFICATION
{
    BSH_POINT_FRONT,
    BSH_POINT_BACK,
    BSH_POINT_COPLANAR = 4,
};

enum ED_POLYGON_CLASSIFICATION
{
    BSH_POLYGON_FRONT = 0,
    BSH_POLYGON_BACK,
    BSH_POLYGON_STRADDLING,
    BSH_POLYGON_COPLANAR
};

enum ED_SET_OP
{
    BSH_SET_OP_UNION = 0,
    BSH_SET_OP_INTERSECTION,
    BSH_SET_OP_SUBTRACTION,
};

struct ed_bsp_polygon_t
{
    vec3_t *verts;
};

struct ed_bsp_t
{
//    struct bsh_bsp_t *back;
//    struct bsh_bsp_t *front;
//    uint32_t polygon_count;
//    uint32_t type;
//    struct bsh_polygon_t *splitter;
//    struct bsh_polygon_t *anti_splitter;
};

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

#endif // ED_BSP_H
