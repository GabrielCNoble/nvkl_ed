#ifndef ED_H
#define ED_H

#include "bsh.h"
#include "neighbor/lib/dstuff/ds_list.h"
#include "neighbor/ent.h"



struct ed_level_data_t
{
    char level_name[PATH_MAX];
};

struct ed_level_t 
{
    char level_path[PATH_MAX];
    struct ed_level_data_t data;
};



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

struct ed_editor_window_t;
struct ed_editor_viewport_t;

struct ed_context_t
{
    void *context_data;
    uint32_t update_frame;
    uint32_t hold_focus;
    void (*input_function)(void *context_data, struct ed_editor_window_t *window);
    void (*update_function)(void *context_data, struct ed_editor_window_t *window);
    uint32_t (*layout_function)(void *context_data, struct ed_editor_window_t *window);
    void (*reset_function)(void *context_data);
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
    uint32_t focused;
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

enum ED_BROWSER_MODE
{
    ED_BROWSER_MODE_SAVE = 0,
    ED_BROWSER_MODE_LOAD,
};

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

void ed_OpenBrowser(uint32_t mode);

void ed_CloseBrowser();

void ed_LoadLevel(char *file_path);

void ed_SaveLevel(char *file_path);

void ed_NewLevel();

void ed_SerializeLevel(void **buffer, uint32_t *buffer_size);

void ed_UnserializeLevel(void *buffer);

/*
=============================================================
=============================================================
=============================================================
*/

union r_command_buffer_h ed_BeginPicking(struct ed_editor_viewport_t *viewport);

uint32_t ed_EndPicking(union r_command_buffer_h command_buffer, struct ed_editor_viewport_t *viewport);

void ed_FlyView(struct ed_editor_viewport_t *viewport);

#endif // ED_H
