#include <stdio.h>
#include <limits.h>
#include "ed.h"
#include "ed_w_ctx.h"
#include "neighbor/r_draw.h"
#include "neighbor/lib/dstuff/ds_file.h"
#include "neighbor/lib/dstuff/ds_dir.h"
#include "neighbor/lib/dstuff/ds_mem.h"
#include "neighbor/lib/dstuff/ds_obj.h"
#include "neighbor/ui.h"
#include "bsh.h"
#include "neighbor/in.h"
#include "neighbor/g.h"


/*

    TODO: 
    
        -   implement scaling transform
        
        -   figure out what should happen when a brush face
            gets deleted (or collapsed).
            
        -   figure out how to not allow brushes to be turned
            inside out
        
        -   figure out undo
    
        -   implement brush face and brush face plane rotation.
            Should reestructure brushes to be the intersection
            of several planes? Or should it stay as a list of
            faces, and plane rotation only extends adjacent 
            faces? 
        
        -   figure out how scaling affects brush faces. Should 
            the scaling act only in global space? Brush local 
            space? Should each face have its own "coordinate 
            system" (answer: probably yes)
        
        -   figure out a way of drawing selected brush elements
            that doesn't involve doing a billion draw state
            changes. Maybe sort them or something.
            
        -   material context
        
        -   brush face context (to allow setting face parameters,
            like material, uv multipliers, uv rotation)
        
        -   figure out how to have brushes not added to the bsp
            cast shadows. The best approach so far is to create
            a mdl_model_t and an ent_entity_t for each brush,
            and draw them that way.
            
        -   figure out brush sets
        
        -   csg

*/

float ed_pitch;
float ed_yaw;

//struct list_t ed_selection_contexts;
//struct ed_selection_context_t *ed_current_selection_context;

//uint32_t ed_current_context = ED_EDITOR_CONTEXT_LAST;
//struct ed_editor_context_t *ed_active_context;
struct ed_context_t ed_contexts[ED_CONTEXT_LAST];

struct ed_editor_window_t *ed_windows;
struct ed_editor_window_t *ed_last_window;
struct ed_editor_window_t *ed_active_window;

struct bsh_brush_t *brush;
struct r_framebuffer_h ed_picking_framebuffer;
struct r_render_pass_handle_t ed_picking_render_pass;
uint32_t ed_picking_framebuffer_width;
uint32_t ed_picking_framebuffer_height;
void *ed_picking_memory;
uint32_t ed_frame;


struct
{
    char path[PATH_MAX];
    char file_name[PATH_MAX];
    struct ds_dir_list_t current_dir;
    uint32_t browser_open;
    uint32_t mode;
} ed_browser_state;

struct r_framebuffer_h ed_framebuffer;

//extern struct r_heap_h r_vertex_heap;
//extern struct r_heap_h r_index_heap;
extern struct r_queue_h r_draw_queue;
extern VkFence r_draw_fence;
extern SDL_Window *r_window;

//#define ED_GRID_QUAD_VERTS 5

//#define ED_GRID_VERTS (((ED_GRID_X_WIDTH >> 1) + (ED_GRID_Z_WIDTH >> 1) - 1) * ED_GRID_QUAD_VERTS)

//struct r_i_vertex_t *ed_center_grid;


//struct r_i_vertex_t *ed_translation_manipulator_verts;
//uint32_t *ed_translation_manipulator_indices;
//struct r_chunk_h ed_translation_manipulator_vert_chunk;
//struct r_chunk_h ed_translation_manipulator_index_chunk;
//#define ED_TRANSLATION_MANIPULATOR_SHAFT_VERT_COUNT 13
//#define ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT 60
//#define ED_TRANSLATION_MANIPULATOR_PLANE_VERT_COUNT 4
//#define ED_TRANSLATION_MANIPULATOR_PLANE_INDICE_COUNT 6
//#define ED_TRANSLATION_MANIPULATOR_VERT_COUNT ((ED_TRANSLATION_MANIPULATOR_SHAFT_VERT_COUNT + ED_TRANSLATION_MANIPULATOR_PLANE_VERT_COUNT) * 3)
//#define ED_TRANSLATION_MANIPULATOR_INDICE_COUNT (ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT + ED_TRANSLATION_MANIPULATOR_PLANE_INDICE_COUNT)
//#define ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT (ED_TRANSLATION_MANIPULATOR_SHAFT_VERT_COUNT + ED_TRANSLATION_MANIPULATOR_PLANE_VERT_COUNT)


struct r_i_vertex_t *ed_rotation_manipulator;
struct r_i_vertex_t *ed_scale_manipulator;
struct r_shader_handle_t ed_pick_vertex_shader;
struct r_shader_handle_t ed_pick_fragment_shader;

void (*ed_TranslateObjectFunction[ED_OBJECT_TYPE_LAST])(struct ed_object_t *object, vec3_t *translation) = {NULL};

//struct ed_manipulator_t ed_manipulators[ED_TRANSFORM_TYPE_LAST] = {
//    [ED_TRANSFORM_TYPE_TRANSLATION] = {
//        .component_count = 3,
//        .components = (struct ed_manipulator_component_t []){
//            {
//                .vertex_start = 0, 
//                .index_start = 0, 
//                .count = ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT, 
//                .id = ED_PICKER_OBJECT_ID_X_AXIS,
//            },
//            
//            {
//                .vertex_start = ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT, 
//                .index_start = 0, 
//                .count = ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT, 
//                .id = ED_PICKER_OBJECT_ID_Y_AXIS,
//            },
//            
//            {
//                .vertex_start = ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT * 2, 
//                .index_start = 0, 
//                .count = ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT, 
//                .id = ED_PICKER_OBJECT_ID_Z_AXIS,
//            },
//        }
//    }
//};


struct ed_level_t ed_current_level = {};

void ed_Init()
{
//    printf("%s\n", getenv("APPDATA"));
//    vec3_t brush_position = {0.0, 0.0, 0.0};
//    vec3_t brush_scale;
//    mat3_t brush_orientation;
    struct ed_editor_context_t *context;
    uint32_t edges[2];
    char window_name[512];
    
    in_RegisterKey(SDL_SCANCODE_ESCAPE);
    in_RegisterKey(SDL_SCANCODE_SPACE);
    in_RegisterKey(SDL_SCANCODE_LSHIFT);
    in_RegisterKey(SDL_SCANCODE_TAB);
    in_RegisterKey(SDL_SCANCODE_W);
    in_RegisterKey(SDL_SCANCODE_S);
    in_RegisterKey(SDL_SCANCODE_A);
    in_RegisterKey(SDL_SCANCODE_D);
    in_RegisterKey(SDL_SCANCODE_R);
    in_RegisterKey(SDL_SCANCODE_G);
    in_RegisterKey(SDL_SCANCODE_L);
    in_RegisterKey(SDL_SCANCODE_DELETE);
    in_RegisterKey(SDL_SCANCODE_RETURN);
    
    ed_current_level.level_path[0] = '\0';
    ed_current_level.data.level_name[0] = '\0';
    
    strcpy(ed_browser_state.path, "C:/");
    
    ed_w_ctx_Init();
    
    struct ed_editor_window_t *window = mem_Calloc(1, sizeof(struct ed_editor_viewport_t));
    window->focused = 0;
    ed_windows = window;
    ed_last_window = (struct ed_editor_window_t *)ed_windows;
    ed_active_window = (struct ed_editor_window_t *)ed_windows;
    ed_windows->attached_context = ed_contexts + ED_CONTEXT_WORLD;
    
    struct ed_editor_viewport_t *viewport = (struct ed_editor_viewport_t *)window;
    viewport->base.type = ED_EDITOR_WINDOW_TYPE_VIEWPORT;
    viewport->width = 640;
    viewport->height = 480;
    viewport->view_pitch = 0.0;
    viewport->view_yaw = 0.0;
    viewport->framebuffer = r_CreateDrawableFramebuffer(viewport->width, viewport->height);
    mat4_t_identity(&viewport->view_matrix);
    
    
//    window = mem_Calloc(1, sizeof(struct ed_editor_viewport_t));
//    ed_last_window->next = window;
//    ed_active_window = window;
//    window->attached_context = ed_contexts + ED_CONTEXT_WORLD;
//    
//    viewport = (struct ed_editor_viewport_t *)window;
//    viewport->base.type = ED_EDITOR_WINDOW_TYPE_VIEWPORT;
//    viewport->width = 640;
//    viewport->height = 480;
//    viewport->view_pitch = 0.0;
//    viewport->view_yaw = 0.0;
//    viewport->framebuffer = r_CreateDrawableFramebuffer(viewport->width, viewport->height);
//    mat4_t_identity(&viewport->view_matrix);
    
    
    struct r_shader_description_t shader_description = {
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .vertex_binding_count = 1,
        .vertex_bindings = &(struct r_vertex_binding_t){
            .attrib_count = 1,
            .size = sizeof(struct r_vertex_t),
            .attribs = &(struct r_vertex_attrib_t){
                .offset = offsetof(struct r_vertex_t, position),
                .format = VK_FORMAT_R32G32B32A32_SFLOAT
            }
        },
        .push_constant_count = 1,
        .push_constants = (struct r_push_constant_t []){
            [0] = {
                .offset = 0,
                .size = sizeof(mat4_t) + sizeof(int32_t),
            }, 
        }
    };
    
    FILE *shader_file = fopen("shaders/pick.vert.spv", "rb");
    read_file(shader_file, &shader_description.code, &shader_description.code_size);
    fclose(shader_file);
    ed_pick_vertex_shader = r_CreateShader(&shader_description);
//    struct r_shader_t *vertex_shader = r_GetShaderPointer(r_CreateShader(&shader_description));
    
    
    shader_description = (struct r_shader_description_t){
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    shader_file = fopen("shaders/pick.frag.spv", "rb");
    read_file(shader_file, &shader_description.code, &shader_description.code_size);
    fclose(shader_file);
    ed_pick_fragment_shader = r_CreateShader(&shader_description);
//    struct r_shader_t *fragment_shader = r_GetShaderPointer(r_CreateShader(&shader_description));
    
    struct r_render_pass_description_t render_pass_description = {
        .attachment_count = 2,
        .attachments = (struct r_attachment_d []){
            [0] = {
                .format = VK_FORMAT_R32_UINT,
                .tiling = VK_IMAGE_TILING_LINEAR,
                .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
            },
            [1] = {
                .format = VK_FORMAT_D32_SFLOAT,
                .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
            }
        },
        .subpass_count = 1,
        .subpasses = (struct r_subpass_description_t []){
            [0] = {
                .color_attachments = &(VkAttachmentReference){
                    .attachment = 0,
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                },
                .color_attachment_count = 1,
                .depth_stencil_attachment = &(VkAttachmentReference){
                    .attachment = 1,
                    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                },
                .pipeline_description_count = 1,
                .pipeline_descriptions = (struct r_pipeline_description_t []){
                    [0] = {
                        .shader_count = 2,
                        .shaders = (struct r_shader_handle_t []){ed_pick_vertex_shader, ed_pick_fragment_shader},
                        .vertex_input_state = &(VkPipelineVertexInputStateCreateInfo){
                            .vertexBindingDescriptionCount = 1,
                            .pVertexBindingDescriptions = &(VkVertexInputBindingDescription){
                                .binding = 0,
                                .stride = sizeof(struct bsh_vertex_t),
                            },
                            .vertexAttributeDescriptionCount = 1,
                            .pVertexAttributeDescriptions = &(VkVertexInputAttributeDescription){
                                .binding = 0,
                                .location = 0,
                                .format = VK_FORMAT_R32G32B32_SFLOAT,
                                .offset = offsetof(struct bsh_vertex_t, position)
                            }
                        },
                        .input_assembly_state = &(VkPipelineInputAssemblyStateCreateInfo){
                            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                        },
                        .rasterization_state = &(VkPipelineRasterizationStateCreateInfo) {
                            .cullMode = VK_CULL_MODE_BACK_BIT,
                            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE
                        }
                    }
                }
            }
        }
    };
    
    SDL_Rect display_size;
    SDL_GetDisplayBounds(0, &display_size);
    ed_picking_render_pass = r_CreateRenderPass(&render_pass_description);
    
    struct r_framebuffer_d framebuffer_description = {
        .width = display_size.w,
        .height = display_size.h,
        .frame_count = 1,
        .render_pass = r_GetRenderPassPointer(ed_picking_render_pass)
    };
    
    ed_picking_framebuffer = r_CreateFramebuffer(&framebuffer_description);
    struct r_framebuffer_t *framebuffer = r_GetFramebufferPointer(ed_picking_framebuffer);
    struct r_texture_t *texture = framebuffer->textures[0];
    ed_picking_memory = r_MapImageMemory(texture->image);
}

void ed_Shutdown()
{
     
}

void ed_Main(float delta_time)
{
    struct r_view_t *view;
    
    if(in_GetKeyState(SDL_SCANCODE_ESCAPE) & IN_INPUT_STATE_PRESSED)
    {
        g_Quit();
    }
    
    ed_DrawLayout();
    ed_UpdateWindows();
}

void ed_UpdateLayout()
{
    
}

void ed_UpdateWindows()
{
    struct ed_editor_window_t *window = ed_windows;
    
    while(window)
    {
        struct ed_context_t *context = window->attached_context;
        
        switch(window->type)
        {
            case ED_EDITOR_WINDOW_TYPE_VIEWPORT:
            
            break;
        }
        
//        if(context->update_frame != ed_frame)
//        {
            context->update_frame = ed_frame;
            
            if(window == ed_active_window)
            {
                context->input_function(context->context_data, window);
            }
            
//        }
        context->update_function(context->context_data, window);
        
        window = window->next;
    }
    
    ed_frame++;
}

void ed_DrawLayout()
{
    vec2_t window_size;
    r_GetWindowSize(&window_size);    
    
    igSetNextWindowPos((ImVec2){0.0, 0.0}, 0, (ImVec2){0.0, 0.0});
    igSetNextWindowSize((ImVec2){window_size.x, window_size.y}, 0);
    igPushStyleVarVec2(ImGuiStyleVar_WindowMinSize, (ImVec2){0.0, 0.0});
//    igPushStyleVarVec2(ImGuiStyleVar_WindowPadding, (ImVec2){0.0, 0.0});
    igPushStyleVarFloat(ImGuiStyleVar_WindowRounding, 0.0);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus; 
    igBegin("Main", NULL, window_flags);
    igPopStyleVar(2);
    int32_t mouse_x;
    int32_t mouse_y;
    
    struct ed_editor_window_t *active_window = NULL;
    
    char window_name[512];
//    char path[512];
    
    if(igBeginMenuBar())
    {
        if(igBeginMenu("File", 1))
        {
            if(igMenuItemBool("New", NULL, 0, 1))
            {
                ed_NewLevel();
            }
            if(igMenuItemBool("Save", NULL, 0, ed_current_level.level_path))
            {
                if(ed_current_level.level_path[0] == '\0')
                {
                    ed_OpenBrowser(ED_BROWSER_MODE_SAVE);
                }
                else
                {
                    ed_SaveLevel(ds_path_AppendPath(ed_current_level.level_path, ed_current_level.data.level_name));
                }
            }
            if(igMenuItemBool("Save as...", NULL, 0, 1))
            {
                ed_OpenBrowser(ED_BROWSER_MODE_SAVE);
            }
            if(igMenuItemBool("Load", NULL, 0, 1))
            {
                ed_OpenBrowser(ED_BROWSER_MODE_LOAD);
            }
            igEndMenu();
        }
        igEndMenuBar();
    }
    
    if(ed_browser_state.browser_open)
    {
        igSetNextWindowFocus();
        igSetNextWindowSize((ImVec2){window_size.x, window_size.y}, ImGuiCond_Always);
        igSetNextWindowPos((ImVec2){0.0, 0.0}, 0, (ImVec2){0.0, 0.0});
        if(igBegin("Browser", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if(igBeginChildStr("address_bar", (ImVec2){0.0, 60.0}, 0, 0))
            {
                strcpy(ed_browser_state.path, ed_browser_state.current_dir.path);
                uint32_t edited = igInputText("Path", ed_browser_state.path, PATH_MAX, ImGuiInputTextFlags_EnterReturnsTrue, NULL, NULL);
                edited |= igIsItemDeactivatedAfterEdit();
                
                if(edited)
                {
                    if(ds_dir_IsDir(ds_path_FormatPath(ed_browser_state.path)))
                    {
                        ds_dir_OpenDir(&ed_browser_state.current_dir, ed_browser_state.path);
                    }
                }
                
                igSameLine(0.0, -1.0);
                
                if(igButton("Up", (ImVec2){60.0, 20.0}))
                {
                    if(ds_dir_IsDir(ds_path_DropPath(&ed_browser_state.path)))
                    {
                        ds_dir_GoUp(&ed_browser_state.current_dir);
                    }
                }
                igSameLine(0.0, -1.0);
                if(igButton("Close", (ImVec2){60.0, 20.0}))
                {
                    ed_CloseBrowser();
                }
                
                igInputText("Name", ed_browser_state.file_name, PATH_MAX, ImGuiInputTextFlags_EnterReturnsTrue, NULL, NULL);
                if(!igIsItemActive() && (in_GetKeyState(SDL_SCANCODE_RETURN) & IN_INPUT_STATE_JUST_PRESSED))
                {
                    switch(ed_browser_state.mode)
                    {
                        case ED_BROWSER_MODE_SAVE:
                            ed_SaveLevel(ds_path_AppendPath(ed_browser_state.path, ed_browser_state.file_name));
                            ed_CloseBrowser();
                        break;
                        
                        case ED_BROWSER_MODE_LOAD:
                            ed_LoadLevel(ds_path_AppendPath(ed_browser_state.path, ed_browser_state.file_name));
                            ed_CloseBrowser();
                        break;
                    }
                }
                
                igSameLine(0.0, -1.0);
                
                switch(ed_browser_state.mode)
                {
                    case ED_BROWSER_MODE_SAVE:
                        if(igButton("Save", (ImVec2){60.0, 20.0}))
                        {
                            ed_SaveLevel(ds_path_AppendPath(ed_browser_state.path, ed_browser_state.file_name));
                            ed_CloseBrowser();
                        }
                    break;
                    
                    case ED_BROWSER_MODE_LOAD:
                        if(igButton("Load", (ImVec2){60.0, 20.0}))
                        {
                            ed_LoadLevel(ds_path_AppendPath(ed_browser_state.path, ed_browser_state.file_name));
                            ed_CloseBrowser();
                        }
                    break;
                }
            }
            igEndChild();

            igSeparator();
            
            igColumns(2, "columns", 1);
            igSetColumnWidth(-1, 400);
            if(igBeginChildStr("recent", (ImVec2){0.0, 0.0}, 0, 0))
            {
                
            }
            igEndChild();
            igNextColumn();
            
            ImVec2 content_size;
            igGetContentRegionMax(&content_size);
            
            if(igBeginChildStr("dir_list", (ImVec2){0.0, 0.0}, 0, 0))
            {
                if(igListBoxHeaderVec2("", (ImVec2){-8.0, -8.0}))
                {
                    for(uint32_t entry_index = 0; entry_index < ed_browser_state.current_dir.entry_count; entry_index++)
                    {
                        struct ds_dir_entry_t *entry = ed_browser_state.current_dir.entries + entry_index;
                        if(igSelectableBool(entry->name, 0, 0, (ImVec2){0.0, 0.0}))
                        {
                            switch(entry->type)
                            {
                                case DS_DIR_ENTRY_TYPE_DIR:
                                    ds_dir_GoDown(&ed_browser_state.current_dir, entry->name);
                                break;
                                
                                case DS_DIR_ENTRY_TYPE_PARENT:
                                    ds_dir_GoUp(&ed_browser_state.current_dir);
                                break;
                                
                                case DS_DIR_ENTRY_TYPE_FILE:
                                    strcpy(ed_browser_state.file_name, entry->name);
                                break;
                            }                            
                        }
                        
                    }
                    igListBoxFooter();
                }
            }
            igEndChild();
            
        }
        igEnd();
    }
    

    ImGuiContext *context = igGetCurrentContext();
    struct ed_editor_window_t *window = ed_windows;
    in_GetMousePos(&mouse_x, &mouse_y);
    ed_active_window = NULL;
    while(window)
    {
        uint32_t hovered = window->attached_context->layout_function(window->attached_context->context_data, window);
        
        window->focused = 0;
        
        if(!window->attached_context->hold_focus && hovered)
        {
            ed_active_window = window;
            window->focused = 1;
        }

        window = window->next;
    }
    
    igEnd();
    
//    ed_active_window = active_window;
} 

/*
=============================================================
=============================================================
=============================================================
*/

void ed_OpenBrowser(uint32_t mode)
{
    ed_browser_state.browser_open = 1;
    ed_browser_state.mode = mode;
    ds_dir_OpenDir(&ed_browser_state.current_dir, ed_browser_state.path);
}

void ed_CloseBrowser()
{
    ed_browser_state.browser_open = 0;
}

void ed_LoadLevel(char *file_path)
{
    file_path = ds_path_AppendExt(file_path, ".lvl");
    FILE *file = fopen(file_path, "rb");
    void *level_buffer;
    uint32_t level_buffer_size;
    read_file(file, &level_buffer, &level_buffer_size);
    ed_UnserializeLevel(level_buffer);
    mem_Free(level_buffer);
    
    char *file_path_no_ext = ds_path_GetPathAndFileNameNoExt(file_path);
    
    strcpy(ed_current_level.level_path, ds_path_GetPath(file_path_no_ext));
    strcpy(ed_current_level.data.level_name, ds_path_GetFileName(file_path_no_ext));
    SDL_SetWindowTitle(r_window, ed_current_level.data.level_name);
}

void ed_SaveLevel(char *file_path)
{
    void *level_buffer;
    uint32_t level_buffer_size;
    
    file_path = ds_path_AppendExt(file_path, ".lvl");
    
    ed_SerializeLevel(&level_buffer, &level_buffer_size);
    FILE *file = fopen(file_path, "wb");
    fwrite(level_buffer, 1, level_buffer_size, file);
    fclose(file);
    mem_Free(level_buffer);
    
    char *file_path_no_ext = ds_path_GetPathAndFileNameNoExt(file_path);
    
    strcpy(ed_current_level.level_path, ds_path_GetPath(file_path_no_ext));
    strcpy(ed_current_level.data.level_name, ds_path_GetFileName(file_path_no_ext));
    SDL_SetWindowTitle(r_window, ed_current_level.data.level_name);
}

void ed_NewLevel()
{
    ed_current_level.data.level_name[0] = '\0';
    ed_current_level.level_path[0] = '\0';
    
    for(uint32_t context_index = 0; context_index < ED_CONTEXT_LAST; context_index++)
    {
        ed_contexts[context_index].reset_function(ed_contexts[context_index].context_data);
    }
}

void ed_SerializeLevel(void **buffer, uint32_t *buffer_size)
{
    struct ds_section_t section = {};
    strcpy(section.data.name, "level");
    void *brush_buffer = NULL;
    uint32_t brush_buffer_size = 0;
    
    ed_SerializeBrushes(&brush_buffer, &brush_buffer_size);
    ds_append_record(&section, "brushes", brush_buffer_size, brush_buffer);
    
    
    uint32_t level_buffer_size = sizeof(struct ed_level_data_t);
    void *level_buffer = mem_Calloc(1, level_buffer_size);
    struct ed_level_data_t *level_data = (struct ed_level_data_t *)level_buffer;
    
    strcpy(level_data->level_name, ed_current_level.data.level_name);
    ds_append_record(&section, "level", level_buffer_size, level_buffer);
    mem_Free(level_buffer);
    
    ds_serialize_section(&section, buffer, buffer_size);
    ds_free_section(&section);
}

void ed_UnserializeLevel(void *buffer)
{    
    struct ed_world_context_data_t *data;
    struct ds_section_t section = ds_unserialize_section(buffer);
    
    struct ds_record_t *brush_record = ds_find_record(&section, "brushes");
    ed_UnserializeBrushes(brush_record->data.data);
    
    struct ds_record_t *level_record = ds_find_record(&section, "level");
    struct ed_level_data_t *level_data = (struct ed_level_data_t *)&level_record->data;
    
    memcpy(&ed_current_level.data, level_data, sizeof(struct ed_level_data_t));
    
    struct stack_list_t *brushes = bsh_GetBrushList();
    data = ed_contexts[ED_CONTEXT_WORLD].context_data;
    for(uint32_t brush_index = 0; brush_index < brushes->cursor; brush_index++)
    {
        struct bsh_brush_h handle = BSH_BRUSH_HANDLE(brush_index);
        ed_w_ctx_CreateBrushObjectFromBrush(&data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects, handle);
    }
    
    ds_free_section(&section);
}

/*
=============================================================
=============================================================
=============================================================
*/

union r_command_buffer_h ed_BeginPicking(struct ed_editor_viewport_t *viewport)
{
    VkViewport vk_viewport = {};
    VkRect2D scissor = {};
    union r_command_buffer_h command_buffer;
    struct r_render_pass_t *render_pass;
    struct r_pipeline_t *pipeline;
    struct r_buffer_heap_t *vertex_heap;
    struct r_buffer_heap_t *index_heap;
    struct r_framebuffer_t *framebuffer;
    struct r_framebuffer_description_t *framebuffer_description;
    struct r_render_pass_begin_info_t begin_info = {};
    
    vk_viewport.width = (float)viewport->width;
    vk_viewport.height = (float)viewport->height;
    vk_viewport.minDepth = 0.0;
    vk_viewport.maxDepth = 1.0;
    
    scissor.extent.width = viewport->width;
    scissor.extent.height = viewport->height;
    
    command_buffer = r_AllocateCommandBuffer();
    render_pass = r_GetRenderPassPointer(ed_picking_render_pass);
//    pipeline = r_GetPipelinePointer(render_pass->pipelines[0]);
    vertex_heap = r_GetHeapPointer(R_DEFAULT_VERTEX_HEAP);
    index_heap = r_GetHeapPointer(R_DEFAULT_INDEX_HEAP);
    framebuffer = r_GetFramebufferPointer(ed_picking_framebuffer);
    
    begin_info.render_pass = ed_picking_render_pass;
    begin_info.render_area = scissor;
    begin_info.framebuffer = ed_picking_framebuffer;
    begin_info.clear_value_count = 2;
    begin_info.clear_values = (VkClearValue[]){
        [0] = {
            .color.uint32[0] = 0
        },
        [1] = {
            .depthStencil.depth = 1.0
        }
    };
    
    r_vkBeginCommandBuffer(command_buffer);
    r_vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_heap->buffer, &(VkDeviceSize){0});
    r_vkCmdBindIndexBuffer(command_buffer, index_heap->buffer, 0, VK_INDEX_TYPE_UINT32);
    r_vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_pass->pipelines[0]);
    r_vkCmdSetViewport(command_buffer, 0, 1, &vk_viewport);
    r_vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    r_vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    
    
    return command_buffer;
}

uint32_t ed_EndPicking(union r_command_buffer_h command_buffer, struct ed_editor_viewport_t *viewport)
{
    struct r_submit_info_t submit_info = {};
    
    r_vkCmdEndRenderPass(command_buffer);
    r_vkEndCommandBuffer(command_buffer);
    
    submit_info.s_type = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.command_buffers = &command_buffer;
    submit_info.command_buffer_count = 1;
    
    struct r_fence_h fence = r_AllocFence();
    
//    r_vkResetFences(1, &r_draw_fence);
    r_vkQueueSubmit(r_draw_queue, 1, &submit_info, fence);
    r_vkWaitForFences(1, &fence, VK_TRUE, 0xffffffffffffffff);
    
    VkSubresourceLayout layout;
    VkImageSubresource subresource;
    
    subresource.mipLevel = 0;
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.arrayLayer = 0;
    
    struct r_framebuffer_t *framebuffer = r_GetFramebufferPointer(ed_picking_framebuffer);
    struct r_texture_t *texture = framebuffer->textures[0];
    struct r_image_t *image = texture->image;
    vkGetImageSubresourceLayout(r_GetDevice(), image->image, &subresource, &layout);
    
    uint32_t row = layout.rowPitch * viewport->mouse_y;
    
    return *(uint32_t *)((char *)ed_picking_memory + row + viewport->mouse_x * sizeof(uint32_t));
}

void ed_FlyView(struct ed_editor_viewport_t *viewport)
{
    float mouse_dx;
    float mouse_dy;
    mat3_t view_pitch_matrix;
    mat3_t view_yaw_matrix;
    mat3_t view_orientation;
    vec3_t view_translation;
    struct r_view_t *view;
    
//    if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_PRESSED)
//    {
//        if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_JUST_PRESSED)
//        {
//            in_RelativeMode(1);
//        }
        
        view = r_GetViewPointer();
    
        in_GetMouseDelta(&mouse_dx, &mouse_dy);
        viewport->view_pitch += mouse_dy * 0.6;
        viewport->view_yaw -= mouse_dx * 0.6;
        
        if(viewport->view_pitch > 0.5) viewport->view_pitch = 0.5;
        else if(viewport->view_pitch < -0.5) viewport->view_pitch = -0.5;
        
        if(viewport->view_yaw > 1.0) viewport->view_yaw = -2.0 + viewport->view_yaw;
        else if (viewport->view_yaw < -1.0) viewport->view_yaw = 2.0 + viewport->view_yaw;
        
        mat3_t_identity(&view_pitch_matrix);
        mat3_t_identity(&view_yaw_matrix);
        
        mat3_t_rotate_x(&view_pitch_matrix, viewport->view_pitch);
        mat3_t_rotate_y(&view_yaw_matrix, viewport->view_yaw);
        mat3_t_mul(&view_orientation, &view_pitch_matrix, &view_yaw_matrix);    
        view_translation = vec3_t_c(0.0, 0.0, 0.0);
        
        if(in_GetKeyState(SDL_SCANCODE_W) & IN_INPUT_STATE_PRESSED)
        {
            view_translation.z = -0.1;
        }
        else if(in_GetKeyState(SDL_SCANCODE_S) & IN_INPUT_STATE_PRESSED)
        {
            view_translation.z = 0.1;
        }
        
        if(in_GetKeyState(SDL_SCANCODE_A) & IN_INPUT_STATE_PRESSED)
        {
            view_translation.x = -0.1;
        }
        else if(in_GetKeyState(SDL_SCANCODE_D) & IN_INPUT_STATE_PRESSED)
        {
            view_translation.x = 0.1;
        }
        
        mat3_t_vec3_t_mul(&view_translation, &view_translation, &view_orientation);
        
        view_translation.x += viewport->view_matrix.rows[3].x;
        view_translation.y += viewport->view_matrix.rows[3].y;
        view_translation.z += viewport->view_matrix.rows[3].z;
        
        mat4_t_comp(&viewport->view_matrix, &view_orientation, &view_translation);
//    }
}







