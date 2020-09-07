#ifndef ED_H
#define ED_H

#include "bsh.h"
#include "neighbor/lib/dstuff/ds_list.h"
#include "neighbor/ent.h"

enum ED_EDITOR_CONTEXT
{
    ED_CONTEXT_WORLD = 0,
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
    ED_TRANSFORM_TYPE_SCALE,
    ED_TRANSFORM_TYPE_BOUNDS,
    ED_TRANSFORM_TYPE_LAST
};

enum ED_TRANSFORM_MODE
{
    ED_TRANSFORM_MODE_WORLD = 0,
    ED_TRANSFORM_MODE_LOCAL,
};

struct ed_editor_window_t;
struct ed_editor_viewport_t;

/*
    The basic editor functionality is comprised of windows and contexts. Windows serve
    to determine which context is receiving inputs. Contexts serve to limit which 
    commands can be executed in a given situation with the given input.

    For instance, the level editor has a context to deal with world editing, one for
    dealing with brush editing, one for dealing with material editing, etc.

    The world editing context will allow moving things around, and will respond to
    commands to add/delete objects, or to change the transformation mode. The brush
    editing context won't respond to the same comands, however. It'll allow to grab
    vertices/edges/faces, drag them around, but won't allow adding new brushes to the
    world. Nor will it allow select objects that are not brushes. It'll allow editing
    only brushes that were "brought in" the brush editing context.

    Each context will hold its own blob of data, which they use however they see fit.
    For example, the world editing context will hold a list of selections, while the
    brush editing context will hold a "map" of vertices/edges/faces selected, that maps
    between those primitives and the original brush. The material editing context will
    hold the current material being edited.

    A window is just an abstraction, and it represents anything that can receive inputs
    from the user. So, a simple widget, with a few sliders and checkboxes, is a window, and
    there will be a context attached to it, consuming the inputs. Viewports are also windows.
    The only difference is that they render to the surface of the window.

    All contexts are "alive" all the time, but only one will be active. The active context
    is the one consuming the inputs. 

    A window allows only a single context to be attached to it at a time.
*/

struct ed_context_t
{
    void *context_data;
    uint32_t update_frame;
    void (*input_function)(void *context_data, struct ed_editor_window_t *window);
    void (*update_function)(void *context_data, struct ed_editor_window_t *window);
};

//enum ED_WORLD_CONTEXT_SUB_CONTEXT
//{
//    ED_WORLD_CONTEXT_SUB_CONTEXT_WORLD = 0,
//    ED_WORLD_CONTEXT_SUB_CONTEXT_BRUSH,
//    ED_WORLD_CONTEXT_SUB_CONTEXT_PATH,
//    ED_WORLD_CONTEXT_SUB_CONTEXT_LAST
//};
//
//struct ed_manipulator_component_t
//{
//    uint32_t id;
//    uint32_t index_start;
//    uint32_t vertex_start;
//    uint32_t count;
//};
//
//struct ed_manipulator_t
//{
//    uint32_t component_count;
//    struct ed_manipulator_component_t *components;
//};
//
//struct ed_manipulator_state_t
//{
//    mat4_t transform;
//    uint32_t picked_axis;
//    uint32_t transform_mode;
//    uint32_t transform_type;
//    uint32_t just_picked;
//};

//struct ed_world_context_sub_context_t
//{
//    struct list_t selections;
//    struct list_t objects;
//    struct ed_manipulator_state_t manipulator_state;
////    uint32_t manipulator_axis;
////    mat4_t manipulator_transform;
//    vec3_t pick_offset;
////    uint32_t transform_mode;
////    uint32_t transform_type;
//    void (*input_function)(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport);
//    void (*update_function)(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport);
//};
//
//struct ed_world_context_data_t
//{
//    struct ed_world_context_sub_context_t *sub_contexts;
//    struct ed_world_context_sub_context_t *active_sub_context;
//};

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
    int32_t mouse_x;
    int32_t mouse_y;
    float view_pitch;
    float view_yaw;
    mat4_t view_matrix;
    mat4_t inv_view_matrix;
    mat4_t projection_matrix;
};

//enum ED_PICKER_OBJECT_ID
//{
//    ED_PICKER_OBJECT_ID_X_AXIS = 1,
//    ED_PICKER_OBJECT_ID_Y_AXIS = 1 << 1,
//    ED_PICKER_OBJECT_ID_Z_AXIS = 1 << 2,
//    ED_PICKER_OBJECT_ID_ALL_AXIS = ED_PICKER_OBJECT_ID_X_AXIS | ED_PICKER_OBJECT_ID_Y_AXIS | ED_PICKER_OBJECT_ID_Z_AXIS,
//    ED_PICKER_OBJECT_ID_LAST = ED_PICKER_OBJECT_ID_ALL_AXIS + 1
//};

void ed_Init();

void ed_Shutdown();

void ed_Main(float delta_time);

void ed_UpdateLayout();

void ed_UpdateWindows();

void ed_DrawLayout();

/*
=============================================================
=============================================================
=============================================================
*/


union r_command_buffer_h ed_BeginPicking(struct ed_editor_viewport_t *viewport);

uint32_t ed_EndPicking(union r_command_buffer_h command_buffer, struct ed_editor_viewport_t *viewport);

void ed_FlyView(struct ed_editor_viewport_t *viewport);

#endif // ED_H
