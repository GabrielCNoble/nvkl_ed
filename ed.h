#ifndef ED_H
#define ED_H

#include "bsh.h"
#include "neighbor/lib/dstuff/ds_list.h"

enum ED_EDITOR_CONTEXT
{
    ED_EDITOR_CONTEXT_WORLD = 0,
    ED_EDITOR_CONTEXT_BRUSH,
    ED_EDITOR_CONTEXT_LAST
};

enum ED_SELECTION_TYPE
{
    ED_SELECTION_TYPE_BRUSH = 0,
    ED_SELECTION_TYPE_BRUSH_FACE,
    ED_SELECTION_TYPE_BRUSH_EDGE,
    ED_SELECTION_TYPE_BRUSH_VERT,
    
    ED_SELECTION_TYPE_ENTITY,
    ED_SELECTION_TYPE_LIGHT,
};

enum ED_TRANSFORM_TYPE
{
    ED_TRANSFORM_TYPE_TRANSLATION = 0,
    ED_TRANSFORM_TYPE_ROTATION,
    ED_TRANSFORM_TYPE_SCALE
};

enum ED_TRANSFORM_MODE
{
    ED_TRANSFORM_MODE_WORLD = 0,
    ED_TRANSFORM_MODE_LOCAL,
};

union ed_selection_val_t
{
    struct bsh_brush_t *brush;
    struct bsh_polygon_t *face; 
    /* this will be used both for a single vertice and for an edge */
    struct {struct bsh_vertex_t *ends[2];} edge;
};

struct ed_selection_t
{
    uint32_t type;
    union ed_selection_val_t value;
};

struct ed_editor_context_t
{
    struct list_t selections;
    uint32_t transform_mode;
    uint32_t transform_type;
    mat4_t gizmo_transform;
    void (*ed_EditFunction)(void *context_data);
};

void ed_Init();

void ed_Shutdown();

void ed_Main(float delta_time);

void ed_FlyView();

void ed_SetCurrentContext(uint32_t context);

void ed_EditWorld(void *context_data);

void ed_ShowGizmo(uint32_t type, mat4_t *transform, mat4_t *delta_transform);

void ed_ApplyTransform(mat4_t *apply_to, uint32_t apply_to_count, mat4_t *to_apply, uint32_t transform_type, uint32_t transform_mode);


#endif // ED_H
