#ifndef ED_W_CTX_H
#define ED_W_CTX_H

#include "ed.h"

enum ED_TRANSFORM_TYPE
{
    ED_TRANSFORM_TYPE_TRANSLATION = 0,
    ED_TRANSFORM_TYPE_ROTATION,
    ED_TRANSFORM_TYPE_SCALE,
    ED_TRANSFORM_TYPE_BOUNDS,
    ED_TRANSFORM_TYPE_LAST
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
    ED_OBJECT_TYPE_FACE,
    ED_OBJECT_TYPE_EDGE,
    ED_OBJECT_TYPE_LAST,
};

union ed_object_ref_t
{
    struct 
    {
        struct bsh_brush_h brush;
        uint32_t face_index;
    };
    struct ent_entity_h entity;
};

struct ed_object_t
{
    mat4_t transform;
    uint32_t type;
    uint32_t start;
    uint32_t count;
    uint32_t vertex_count;
    uint32_t vertex_offset;
    uint32_t topology;
    union ed_object_ref_t object;
};

struct ed_object_h
{
    uint32_t index;
};

#define ED_INVALID_OBJECT_INDEX 0xffffffff
#define ED_OBJECT_HANDLE(index) (struct ed_object_h){index}
#define ED_INVALID_OBJECT_HANDLE ED_OBJECT_HANDLE(ED_INVALID_OBJECT_INDEX)

enum ED_MANIPULATOR_AXIS_ID
{
    ED_MANIPULATOR_AXIS_ID_X_AXIS = 1,
    ED_MANIPULATOR_AXIS_ID_Y_AXIS = 1 << 1,
    ED_MANIPULATOR_AXIS_ID_Z_AXIS = 1 << 2,
    ED_MANIPULATOR_AXIS_ID_ALL_AXIS = ED_MANIPULATOR_AXIS_ID_X_AXIS | ED_MANIPULATOR_AXIS_ID_Y_AXIS | ED_MANIPULATOR_AXIS_ID_Z_AXIS,
//    ED_OBJECT_ID_LAST = ED_OBJECT_ID_ALL_AXIS + 1
};

enum ED_WORLD_CONTEXT_SELECTION_TARGET
{
    ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT = 0,
    ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH,
    ED_WORLD_CONTEXT_SELECTION_TARGET_LAST
};

enum ED_WORLD_CONTEXT_STATE
{
    ED_WORLD_CONTEXT_STATE_IDLE = 0,
    ED_WORLD_CONTEXT_STATE_LEFT_CLICK,
    ED_WORLD_CONTEXT_STATE_RIGHT_CLICK,
    ED_WORLD_CONTEXT_STATE_PICK_OBJECT,
    ED_WORLD_CONTEXT_STATE_PICK_BRUSH_FACE,
    ED_WORLD_CONTEXT_STATE_MANIPULATING,
    ED_WORLD_CONTEXT_STATE_CREATING_BRUSH,
    ED_WORLD_CONTEXT_STATE_EXTRUDING_BRUSH_FACES,
    ED_WORLD_CONTEXT_STATE_COPYING_SELECTIONS,
    ED_WORLD_CONTEXT_STATE_DELETING_SELECTIONS,
    ED_WORLD_CONTEXT_STATE_FLYING,
};

//enum ED_WORLD_CONTEXT_SUB_CONTEXT
//{
//    ED_WORLD_CONTEXT_SUB_CONTEXT_WORLD = 0,
//    ED_WORLD_CONTEXT_SUB_CONTEXT_BRUSH,
//    ED_WORLD_CONTEXT_SUB_CONTEXT_PATH,
//    ED_WORLD_CONTEXT_SUB_CONTEXT_LAST
//};

struct ed_manipulator_component_t
{
    uint32_t id;
    uint32_t vertex_start;
    uint32_t start;
    uint32_t count;
};

struct ed_manipulator_t
{
    uint32_t component_count;
    struct ed_manipulator_component_t *components;
    struct r_i_vertex_t *vertices;
    struct r_chunk_h chunk;
};

struct ed_manipulator_state_t
{
    mat4_t transform;
    uint32_t picked_axis;
    uint32_t axis_contraint;
    uint32_t transform_mode;
    uint32_t transform_type;
    uint32_t just_picked;
};

//struct ed_world_context_sub_context_t
//{
//    struct list_t selections;
//    struct list_t objects;
//    struct ed_manipulator_state_t manipulator_state;
//    vec3_t pick_offset;
//    void (*input_function)(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport);
//    void (*update_function)(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport);
//};

struct ed_world_context_selection_target_data_t
{
    struct list_t selections;
    struct stack_list_t objects;
};

struct ed_world_context_data_t
{
    uint32_t current_selection_target;
    struct ed_world_context_selection_target_data_t target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_LAST];
    struct ed_manipulator_state_t manipulator_state;
    struct ed_object_h selection;
    
    uint32_t selection_area_active;
    
    struct
    {
        uint32_t setup;
    }left_click_state;
    
    struct 
    {
        uint32_t setup;
//        uint32_t axis_constraint;
        vec3_t pick_offset;
        vec3_t pick_normal;
        vec3_t axis_constraint;
        float linear_threshold;
        float angular_threshold;
    }manipulation_state;
    
    struct 
    {
        uint32_t setup;
        vec3_t start_corner;
        vec3_t current_corner;
    }creating_brush_state;
    
    uint32_t context_state;
    uint32_t just_changed_state;
};

void ed_w_ctx_Init();

void ed_w_ctx_Input(void *context_data, struct ed_editor_window_t *window);

void ed_w_ctx_Update(void *context_data, struct ed_editor_window_t *window);

uint32_t ed_w_ctx_Layout(void *context_data, struct ed_editor_window_t *window);

void ed_w_ctx_Reset(void *context_data);

/* 
===========================================================================================================================================
===========================================================================================================================================
===========================================================================================================================================
*/

struct ed_object_h ed_w_ctx_CreateObject(struct stack_list_t *objects, mat4_t *transform, uint32_t type, uint32_t start, uint32_t count, uint32_t vertex_count, uint32_t vertex_offset, uint32_t topology, union ed_object_ref_t object_ref);

void ed_w_ctx_DestroyObject(struct stack_list_t *objects, struct ed_object_h handle);

struct ed_object_h ed_w_ctx_CreateBrushObject(struct stack_list_t *objects, vec3_t *position, vec3_t *size);

struct ed_object_h ed_w_ctx_CreateBrushFaceObject(struct stack_list_t *objects, mat4_t *transform, struct bsh_brush_h handle, uint32_t face_index);

struct ed_object_h ed_w_ctx_CreateBrushEdgeObject(struct stack_list_t *objects, struct bsh_brush_h handle, uint32_t face_index, uint32_t edge_index);

struct ed_object_h ed_w_ctx_CreateBrushObjectFromBrush(struct stack_list_t *objects, struct bsh_brush_h handle);

struct ed_object_t *ed_w_ctx_GetObjectPointer(struct stack_list_t *objects, struct ed_object_h handle);

/*
===========================================================================================================================================
===========================================================================================================================================
===========================================================================================================================================
*/

void ed_w_ctx_SetState(struct ed_world_context_data_t *data, uint32_t state);

void ed_w_ctx_IdleState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

void ed_w_ctx_LeftClickState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

void ed_w_ctx_RightClickState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

void ed_w_ctx_PickObjectState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

void ed_w_ctx_PickBrushFaceState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

void ed_w_ctx_ManipulatingState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

void ed_w_ctx_CreatingBrushState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

void ed_w_ctx_ExtrudingBrushFacesState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

void ed_w_ctx_CopyingSelectionsState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

void ed_w_ctx_DeletingSelectionsState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

void ed_w_ctx_FlyingState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window);

/*
===========================================================================================================================================
===========================================================================================================================================
===========================================================================================================================================
*/

vec3_t ed_w_ctx_WorldSpaceMouseVec(struct ed_editor_viewport_t *viewport);

/*
===========================================================================================================================================
===========================================================================================================================================
===========================================================================================================================================
*/

void ed_w_ctx_UpdateManipulatorTransform(struct ed_world_context_data_t *data);

void ed_w_ctx_ManipulatorDrawTransform(mat4_t *draw_transform, struct ed_manipulator_state_t *manipulator_state, struct ed_editor_viewport_t *viewport);

void ed_w_ctx_ObjectTransform(struct stack_list_t *objects, struct ed_object_h handle, mat4_t *transform);

void ed_w_ctx_TransformObjects(mat4_t *transform, struct stack_list_t *objects, struct list_t *selections, uint32_t transform_type, uint32_t transform_mode);

void ed_w_ctx_TransformBrushElements(mat4_t *transform, struct stack_list_t *objects, struct list_t *selections, uint32_t transform_type, uint32_t transform_mode);

uint32_t ed_w_ctx_PickManipulator(struct ed_manipulator_state_t *manipulator_state, struct ed_editor_viewport_t *viewport);

struct ed_object_h ed_w_ctx_PickObject(struct stack_list_t *objects, struct list_t *handles, struct ed_editor_viewport_t *viewport);

void ed_w_ctx_AppendObjectSelection(struct list_t *selections, struct ed_object_h selection, uint32_t shift_pressed);

void ed_w_ctx_CopySelections(struct stack_list_t *objects, struct list_t *selections);

void ed_w_ctx_DeleteSelections(struct stack_list_t *objects, struct list_t *selections);

void ed_w_ctx_AppendBrushSelection(struct list_t *selections, struct ed_object_h selection, uint32_t shift_pressed);

#endif // ED_W_CTX_H
