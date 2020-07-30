#ifndef ED_H
#define ED_H

#include "bsh.h"
#include "neighbor/lib/dstuff/ds_list.h"
#include "neighbor/ent.h"

enum ED_EDITOR_CONTEXT
{
    ED_CONTEXT_WORLD = 0,
//    ED_EDITOR_CONTEXT_BRUSH,
    ED_CONTEXT_LAST
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


enum ED_OBJECT_TYPES
{
    ED_OBJECT_TYPE_ENTITY,
    ED_OBJECT_TYPE_LIGHT,
    ED_OBJECT_TYPE_BRUSH,
    ED_OBJECT_TYPE_LAST,
};

union ed_object_ref_t
{
    struct bsh_brush_h brush;
    struct ent_entity_h entity;
};

struct ed_object_t
{
    mat4_t transform;
    uint32_t type;
    uint32_t start;
    uint32_t count;
    uint32_t topology;
    union ed_object_ref_t object;
};

struct ed_object_h
{
    uint32_t index;
};

struct ed_editor_window_t;
struct ed_editor_viewport_t;

struct ed_context_t
{
    void *context_data;
    uint32_t update_frame;
    void (*input_function)(void *context_data, struct ed_editor_window_t *window);
    void (*update_function)(void *context_data, struct ed_editor_window_t *window);
};

enum ED_WORLD_CONTEXT_SUB_CONTEXT
{
    ED_WORLD_CONTEXT_SUB_CONTEXT_WORLD = 0,
    ED_WORLD_CONTEXT_SUB_CONTEXT_BRUSH,
    ED_WORLD_CONTEXT_SUB_CONTEXT_PATH,
    ED_WORLD_CONTEXT_SUB_CONTEXT_LAST
};

struct ed_world_context_sub_context_t
{
    struct list_t selections;
    mat4_t manipulator_transform;
    uint32_t transform_mode;
    uint32_t transform_type;
    void (*input_function)(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport);
    void (*update_function)(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport);
};

struct ed_world_context_data_t
{
    struct ed_world_context_sub_context_t *sub_contexts;
    struct ed_world_context_sub_context_t *active_sub_context;
};

enum ED_EDITOR_WINDOW_TYPE
{
    ED_EDITOR_WINDOW_TYPE_VIEWPORT = 0,
};

struct ed_editor_window_t
{
    struct ed_editor_window_t *next;
    struct ed_editor_window_t *prev;
    struct ed_context_t *attached_context;
    uint32_t type;
};

struct ed_editor_viewport_t
{
    struct ed_editor_window_t base;
    struct r_framebuffer_h framebuffer;
    uint32_t width;
    uint32_t height;
    int32_t x;
    int32_t y;
    float view_pitch;
    float view_yaw;
    mat4_t view_matrix;
    mat4_t inv_view_matrix;
    mat4_t projection_matrix;
};

enum ED_PICKER_OBJECT_ID
{
    ED_PICKER_OBJECT_ID_X_AXIS = 1,
    ED_PICKER_OBJECT_ID_Y_AXIS = 1 << 1,
    ED_PICKER_OBJECT_ID_Z_AXIS = 1 << 2,
    ED_PICKER_OBJECT_ID_LAST = ED_PICKER_OBJECT_ID_X_AXIS |
                               ED_PICKER_OBJECT_ID_Y_AXIS |
                               ED_PICKER_OBJECT_ID_Z_AXIS
};



void ed_Init();

void ed_Shutdown();

void ed_Main(float delta_time);

void ed_UpdateLayout();

void ed_UpdateWindows();

void ed_ApplyTransform(mat4_t *apply_to, uint32_t apply_to_count, mat4_t *to_apply, uint32_t transform_type, uint32_t transform_mode);

struct ed_object_h ed_CreateObject(mat4_t *transform, uint32_t type, uint32_t start, uint32_t count, union ed_object_ref_t object_ref);

void ed_DestroyObject(struct ed_object_h handle);

struct ed_object_h ed_CreateBrushObject(vec3_t *position);

struct ed_object_t *ed_GetObjectPointer(struct ed_object_h handle);

void ed_DrawLayout();

/*
=============================================================
=============================================================
=============================================================
*/

void ed_FlyView(struct ed_editor_viewport_t *viewport);

void ed_WorldContextInput(void *context_data, struct ed_editor_window_t *window);

void ed_WorldContextUpdate(void *context_data, struct ed_editor_window_t *window);

void ed_WorldContextDrawObjects(void *context_data, struct ed_editor_window_t *window);

void ed_WorldContextWorldSubContextInput(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport);

void ed_WorldContextWorldSubContextUpdate(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport);

struct ed_object_h ed_WorldContextPickObject(struct ed_editor_viewport_t *viewport);


#endif // ED_H
