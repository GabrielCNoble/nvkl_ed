#include <stdio.h>
#include "ed.h"
#include "neighbor/r_draw.h"
#include "neighbor/lib/dstuff/ds_mem.h"
#include "neighbor/ui.h"
#include "r_ed.h"
#include "bsh.h"
#include "neighbor/in.h"
#include "neighbor/g.h"
#include "ui_ed.h"

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
struct stack_list_t ed_objects;

struct r_framebuffer_h ed_framebuffer;

extern struct r_heap_h r_vertex_heap;
extern VkQueue r_draw_queue;
extern VkFence r_draw_fence;

#define ED_GRID_X_WIDTH 20
#define ED_GRID_Z_WIDTH 20
//#define ED_GRID_QUAD_VERTS 5

//#define ED_GRID_VERTS (((ED_GRID_X_WIDTH >> 1) + (ED_GRID_Z_WIDTH >> 1) - 1) * ED_GRID_QUAD_VERTS)

struct r_i_vertex_t *ed_center_grid;
#define ED_TRANSLATION_MANIPULATOR_VERT_COUNT 24
struct r_i_vertex_t *ed_translation_manipulator_verts;
#define ED_TRANSLATION_MANIPULATOR_INDICE_COUNT 108
uint32_t *ed_translation_manipulator_indices;

struct r_i_vertex_t *ed_rotation_manipulator;
struct r_i_vertex_t *ed_scale_manipulator;

void (*ed_TranslateObjectFunction[ED_OBJECT_TYPE_LAST])(struct ed_object_t *object, vec3_t *translation) = {NULL};

void ed_Init()
{
    vec3_t brush_position = {0.0, 0.0, 0.0};
    vec3_t brush_scale;
    mat3_t brush_orientation;
    struct ed_editor_context_t *context;
    uint32_t edges[2];
    
    in_RegisterKey(SDL_SCANCODE_ESCAPE);
    in_RegisterKey(SDL_SCANCODE_SPACE);
    in_RegisterKey(SDL_SCANCODE_LSHIFT);
    in_RegisterKey(SDL_SCANCODE_W);
    in_RegisterKey(SDL_SCANCODE_S);
    in_RegisterKey(SDL_SCANCODE_A);
    in_RegisterKey(SDL_SCANCODE_D);
    in_RegisterKey(SDL_SCANCODE_R);
    in_RegisterKey(SDL_SCANCODE_G);
    
    ed_objects = create_stack_list(sizeof(struct ed_object_t), 512);
    
    
    ed_contexts[ED_CONTEXT_WORLD].input_function = ed_WorldContextInput;
    ed_contexts[ED_CONTEXT_WORLD].update_function = ed_WorldContextUpdate;
    ed_contexts[ED_CONTEXT_WORLD].context_data = mem_Calloc(1, sizeof(struct ed_world_context_data_t));
    ed_contexts[ED_CONTEXT_WORLD].update_frame = 0xffffffff;
    struct ed_world_context_data_t *data = ed_contexts[ED_CONTEXT_WORLD].context_data;
    data->sub_contexts = mem_Calloc(ED_WORLD_CONTEXT_SUB_CONTEXT_LAST, sizeof(struct ed_world_context_sub_context_t));
    struct ed_world_context_sub_context_t *sub_context = data->sub_contexts + ED_WORLD_CONTEXT_SUB_CONTEXT_WORLD;
    sub_context->input_function = ed_WorldContextWorldSubContextInput;
    sub_context->update_function = ed_WorldContextWorldSubContextUpdate;
    data->active_sub_context = sub_context;
    
    
    struct ed_editor_window_t *window = mem_Calloc(1, sizeof(struct ed_editor_viewport_t));
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
    
    
    window = mem_Calloc(1, sizeof(struct ed_editor_viewport_t));
    ed_last_window->next = window;
    ed_last_window = window;
    window->attached_context = ed_contexts + ED_CONTEXT_WORLD;
    viewport = (struct ed_editor_viewport_t *)window;
    viewport->base.type = ED_EDITOR_WINDOW_TYPE_VIEWPORT;
    viewport->width = 640;
    viewport->height = 480;
    viewport->view_pitch = 0.0;
    viewport->view_yaw = 0.0;
    viewport->framebuffer = r_CreateDrawableFramebuffer(viewport->width, viewport->height);
    mat4_t_identity(&viewport->view_matrix);
    
    bsh_Init();
    
    ed_CreateBrushObject(&vec3_t_c(0.0, 0.0, 0.0));    
    ed_CreateBrushObject(&vec3_t_c(0.0, 0.0, 5.0));
    ed_CreateBrushObject(&vec3_t_c(0.0, 0.0, -5.0));
    
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
    struct r_shader_t *vertex_shader = r_GetShaderPointer(r_CreateShader(&shader_description));
    
    
    shader_description = (struct r_shader_description_t){
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    shader_file = fopen("shaders/pick.frag.spv", "rb");
    read_file(shader_file, &shader_description.code, &shader_description.code_size);
    fclose(shader_file);
    struct r_shader_t *fragment_shader = r_GetShaderPointer(r_CreateShader(&shader_description));
    
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
                        .shaders = (struct r_shader_t *[]){vertex_shader, fragment_shader},
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
                        }
                    }
                }
            }
        }
    };
    
    SDL_Rect display_size;
    SDL_GetDisplayBounds(0, &display_size);
    ed_picking_render_pass = r_CreateRenderPass(&render_pass_description);
    
    struct r_framebuffer_description_t framebuffer_description = {
        .width = display_size.w,
        .height = display_size.h,
        .frame_count = 1,
        .render_pass = r_GetRenderPassPointer(ed_picking_render_pass)
    };
    
    ed_picking_framebuffer = r_CreateFramebuffer(&framebuffer_description);
    struct r_framebuffer_t *framebuffer = r_GetFramebufferPointer(ed_picking_framebuffer);
    struct r_texture_t *texture = r_GetTexturePointer(framebuffer->textures[0]);
    ed_picking_memory = r_MapImageMemory(texture->image);
    
    /* center grid (well, I suppose that was already clear) */
    ed_center_grid = mem_Calloc(((ED_GRID_X_WIDTH >> 1) + (ED_GRID_Z_WIDTH >> 1) + 2) * 5, sizeof(struct r_i_vertex_t));
    struct r_i_vertex_t *verts = ed_center_grid;
    for(int32_t quad_index = ED_GRID_X_WIDTH >> 1; quad_index >= 0; quad_index--)
    {
        struct r_i_vertex_t *vert;
        int32_t vert_index = 0;
        
        vert = verts + vert_index;
        vert->position = vec4_t_c(-(float)quad_index, 0.0, -(float)(ED_GRID_X_WIDTH >> 1), 1.0);
        vert->color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
        vert_index++;
        
        vert = verts + vert_index;
        vert->position = vec4_t_c(-(float)quad_index, 0.0, (float)(ED_GRID_X_WIDTH >> 1), 1.0);
        vert->color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
        vert_index++;
        
        vert = verts + vert_index;
        vert->position = vec4_t_c((float)quad_index, 0.0, (float)(ED_GRID_X_WIDTH >> 1), 1.0);
        vert->color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
        vert_index++;
        
        vert = verts + vert_index;
        vert->position = vec4_t_c((float)quad_index, 0.0, -(float)(ED_GRID_X_WIDTH >> 1), 1.0);
        vert->color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
        vert_index++;
        
        vert = verts + vert_index;
        vert->position = vec4_t_c(-(float)quad_index, 0.0, -(float)(ED_GRID_X_WIDTH >> 1), 1.0);
        vert->color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
        vert_index++;
        
        verts += 5;
    }
    
    
    for(int32_t quad_index = ED_GRID_Z_WIDTH >> 1; quad_index >= 0; quad_index--)
    {
        struct r_i_vertex_t *vert;
        int32_t vert_index = 0;
        
        vert = verts + vert_index;
        vert->position = vec4_t_c(-(float)(ED_GRID_Z_WIDTH >> 1), 0.0, -(float)quad_index, 1.0);
        vert->color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
        vert_index++;
        
        vert = verts + vert_index;
        vert->position = vec4_t_c(-(float)(ED_GRID_Z_WIDTH >> 1), 0.0, (float)quad_index, 1.0);
        vert->color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
        vert_index++;
        
        vert = verts + vert_index;
        vert->position = vec4_t_c((float)(ED_GRID_Z_WIDTH >> 1), 0.0, (float)quad_index, 1.0);
        vert->color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
        vert_index++;
        
        vert = verts + vert_index;
        vert->position = vec4_t_c((float)(ED_GRID_Z_WIDTH >> 1), 0.0, -(float)quad_index, 1.0);
        vert->color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
        vert_index++;
        
        vert = verts + vert_index;
        vert->position = vec4_t_c(-(float)(ED_GRID_Z_WIDTH >> 1), 0.0, -(float)quad_index, 1.0);
        vert->color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
        vert_index++;
        
        verts += 5;
    }
    
    
    /* translation manipulator */
    ed_translation_manipulator_verts = mem_Calloc(24, sizeof(struct r_i_vertex_t));
    ed_translation_manipulator_indices = mem_Calloc(108, sizeof(uint32_t));
    
    /* X axis */
    ed_translation_manipulator_verts[0].position = vec4_t_c(0.5, 0.1,-0.1, 1.0);
    ed_translation_manipulator_verts[0].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[1].position = vec4_t_c(0.5, 0.1, 0.1, 1.0);
    ed_translation_manipulator_verts[1].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[2].position = vec4_t_c(2.5, 0.1, 0.1, 1.0);
    ed_translation_manipulator_verts[2].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[3].position = vec4_t_c(2.5, 0.1,-0.1, 1.0);
    ed_translation_manipulator_verts[3].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[4].position = vec4_t_c(0.5,-0.1,-0.1, 1.0);
    ed_translation_manipulator_verts[4].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[5].position = vec4_t_c(0.5,-0.1, 0.1, 1.0);
    ed_translation_manipulator_verts[5].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[6].position = vec4_t_c(2.5,-0.1, 0.1, 1.0);
    ed_translation_manipulator_verts[6].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[7].position = vec4_t_c(2.5,-0.1,-0.1, 1.0);
    ed_translation_manipulator_verts[7].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_indices[0] = 0;
    ed_translation_manipulator_indices[1] = 1;
    ed_translation_manipulator_indices[2] = 2;
    ed_translation_manipulator_indices[3] = 2;
    ed_translation_manipulator_indices[4] = 3;
    ed_translation_manipulator_indices[5] = 0;
    
    ed_translation_manipulator_indices[6]  = 1;
    ed_translation_manipulator_indices[7]  = 5;
    ed_translation_manipulator_indices[8]  = 6;
    ed_translation_manipulator_indices[9]  = 6;
    ed_translation_manipulator_indices[10] = 2;
    ed_translation_manipulator_indices[11] = 1;
    
    ed_translation_manipulator_indices[12] = 4;
    ed_translation_manipulator_indices[13] = 7;
    ed_translation_manipulator_indices[14] = 6;
    ed_translation_manipulator_indices[15] = 6;
    ed_translation_manipulator_indices[16] = 5;
    ed_translation_manipulator_indices[17] = 4;
    
    ed_translation_manipulator_indices[18] = 0;
    ed_translation_manipulator_indices[19] = 3;
    ed_translation_manipulator_indices[20] = 7;
    ed_translation_manipulator_indices[21] = 7;
    ed_translation_manipulator_indices[22] = 4;
    ed_translation_manipulator_indices[23] = 0;
    
    /* Y axis */
    for(uint32_t index = 0; index < 8; index++)
    {
        struct r_i_vertex_t *vert = ed_translation_manipulator_verts + index;
        ed_translation_manipulator_verts[index + 8].position.x = vert->position.y;
        ed_translation_manipulator_verts[index + 8].position.y = vert->position.x;
        ed_translation_manipulator_verts[index + 8].position.z = vert->position.z;
        ed_translation_manipulator_verts[index + 8].position.w = 1.0;
        ed_translation_manipulator_verts[index + 8].color = vec4_t_c(0.0, 1.0, 0.0, 1.0);
    }
    
    for(uint32_t index = 0; index < 36; index++)
    {
        ed_translation_manipulator_indices[index + 36] = ed_translation_manipulator_indices[index] + 8;
    }
    
    /* Z axis */
    for(uint32_t index = 0; index < 8; index++)
    {
        struct r_i_vertex_t *vert = ed_translation_manipulator_verts + index;
        ed_translation_manipulator_verts[index + 16].position.x = vert->position.z;
        ed_translation_manipulator_verts[index + 16].position.y = vert->position.y;
        ed_translation_manipulator_verts[index + 16].position.z = vert->position.x;
        ed_translation_manipulator_verts[index + 16].position.w = 1.0;
        ed_translation_manipulator_verts[index + 16].color = vec4_t_c(0.0, 0.0, 1.0, 1.0);
    }
    
    for(uint32_t index = 0; index < 36; index++)
    {
        ed_translation_manipulator_indices[index + 72] = ed_translation_manipulator_indices[index] + 16;
    }
    
    
//    ed_framebuffer = r_CreateDrawableFramebuffer(200, 200);
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
    
    if(in_GetKeyState(SDL_SCANCODE_SPACE) & IN_INPUT_STATE_JUST_PRESSED)
    {
        r_Fullscreen(1);
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

struct ed_object_h ed_CreateObject(mat4_t *transform, uint32_t type, uint32_t start, uint32_t count, union ed_object_ref_t object_ref)
{
    struct ed_object_h handle;
    struct ed_object_t *object;
    
    handle.index = add_stack_list_element(&ed_objects, NULL);
    object = get_stack_list_element(&ed_objects, handle.index);
    
    object->type = type;
    object->start = start;
    object->count = count;
    object->transform = *transform;
    object->object = object_ref;
    
    return handle;
}

struct ed_object_h ed_CreateBrushObject(vec3_t *position)
{
    struct ed_object_h handle;
    struct bsh_brush_h brush_handle;
    struct bsh_brush_t *brush;
    mat3_t orientation;
    mat4_t transform;
    
    mat3_t_identity(&orientation);
    brush_handle = bsh_CreateCubeBrush(position, &orientation, &vec3_t_c(1.0, 1.0, 1.0));
    brush = bsh_GetBrushPointer(brush_handle);
    
    mat4_t_identity(&transform);
    
    handle = ed_CreateObject(&transform, ED_OBJECT_TYPE_BRUSH, brush->start, brush->count, (union ed_object_ref_t){.brush = brush_handle});
    return handle;
}

struct ed_object_t *ed_GetObjectPointer(struct ed_object_h handle)
{
    struct ed_object_t *object;
    object = get_stack_list_element(&ed_objects, handle.index);
    if(object && object->type == ED_OBJECT_TYPE_LAST)
    {
        object = NULL;
    }
    return object;
}

//void ed_ApplyTransform(mat4_t *apply_to, uint32_t apply_to_count, mat4_t *to_apply, uint32_t transform_type, uint32_t transform_mode)
//{
//    switch(transform_type)
//    {
//        case ED_TRANSFORM_TYPE_TRANSLATION:
//            for(uint32_t transform_index = 0; transform_index < apply_to_count; transform_index++)
//            {
//                mat4_t *transform = apply_to + transform_index;
//                vec4_t_add(&transform->rows[3], &transform->rows[3], &to_apply->rows[3]);
//            }
//        break;
//        
//        case ED_TRANSFORM_TYPE_ROTATION:
//            
//        break;
//        
//        case ED_TRANSFORM_TYPE_SCALE:
//            
//        break;
//    }
//}

void ed_DrawLayout()
{    
    igSetNextWindowPos((ImVec2){0.0, 0.0}, 0, (ImVec2){0.0, 0.0});
    igSetNextWindowSize((ImVec2){1360, 760}, 0);
    igPushStyleVarVec2(ImGuiStyleVar_WindowMinSize, (ImVec2){0.0, 0.0});
    igPushStyleVarFloat(ImGuiStyleVar_WindowRounding, 0.0);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus; 
    igBegin("Main", NULL, window_flags);
    igPopStyleVar(2);
    
    char window_name[512];
    
    if(igBeginMenuBar())
    {
        if(igBeginMenu("FUCK", 1))
        {
            igMenuItemBool("ASS", NULL, 0, 1);
            igMenuItemBool("ASS", NULL, 0, 1);
            igMenuItemBool("ASS", NULL, 0, 1);
            igEndMenu();
        }
        if(igBeginMenu("SHIT", 1))
        {
            igMenuItemBool("ASS", NULL, 0, 1);
            igMenuItemBool("ASS", NULL, 0, 1);
            igMenuItemBool("ASS", NULL, 0, 1);
            igEndMenu();
        }
        igEndMenuBar();
    }

    ImGuiContext *context = igGetCurrentContext();
    struct ed_editor_window_t *window = ed_windows;
    
    if(!(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_PRESSED))
    {
        ed_active_window = NULL;
    }
    
    while(window)
    {
        switch(window->type)
        {
            case ED_EDITOR_WINDOW_TYPE_VIEWPORT:
            {
                sprintf(window_name, "Viewport%p", window);
//                ImGuiID node_id = igDockContextGenNodeID(context);
//                node_id = igDockBuilderAddNode(node_id, ImGuiDockNodeFlags_DockSpace);
//                igDockBuilderSetNodePos(node_id, (ImVec2){0.0, 0.0});
//                igDockBuilderSetNodeSize(node_id, (ImVec2){200.0, 200.0});
//                ImGuiID another_id = igDockBuilderSplitNode(node_id, ImGuiDir_Down, 0.5, NULL, NULL);
//                igDockBuilderDockWindow("Child0", node_id);
//                igDockBuilderFinish(node_id);
//                
//                igDockSpace(node_id, (ImVec2){0.0, 0.0}, ImGuiDockNodeFlags_PassthruCentralNode, NULL);
//                igSetNextWindowSize((ImVec2){200.0, 200.0}, 0);
                struct ed_editor_viewport_t *viewport = (struct ed_editor_viewport_t *)window;
                igSetNextWindowSize((ImVec2){(float)viewport->width, (float)viewport->height}, ImGuiCond_Once);
                if(igBegin(window_name, NULL, ImGuiWindowFlags_NoScrollbar))
                {
                    struct r_framebuffer_t *framebuffer = r_GetFramebufferPointer(viewport->framebuffer);
                    struct r_texture_t *texture = r_GetTexturePointer(framebuffer->textures[0]);
                    ImTextureID texture_id = (void *)framebuffer->textures[0].index;
                    igImage(texture_id, (ImVec2){(float) viewport->width, (float) viewport->height}, 
                            (ImVec2){0.0, 0.0}, (ImVec2){1.0, 1.0}, (ImVec4){1.0, 1.0, 1.0, 1.0}, (ImVec4){0.0, 0.0, 0.0, 0.0});
                            
                    ImVec2 content_min;
                    ImVec2 content_max;
                    igGetWindowContentRegionMax(&content_max);
                    igGetWindowContentRegionMin(&content_min);
                    
                    content_max.x -= content_min.x;
                    content_max.y -= content_min.y;
                    ImVec2 position;
                    igGetItemRectMin(&position);
                    
                    if(viewport->width != (uint32_t)content_max.x || (uint32_t)viewport->height != content_max.y)
                    {
                        viewport->width = (uint32_t)content_max.x;
                        viewport->height = (uint32_t)content_max.y;
                        r_ResizeFramebuffer(viewport->framebuffer, viewport->width, viewport->height);
                    }
                    
                    viewport->x = position.x;
                    viewport->y = position.y;
                    
                    if(igIsItemHovered(0) && !ed_active_window)
                    {
                        ed_active_window = window;
                    }
                }
                igEnd();
            }
            break;
        }
        
        window = window->next;
    }
    
    igEnd();
} 

/*
=============================================================
=============================================================
=============================================================
*/

void ed_FlyView(struct ed_editor_viewport_t *viewport)
{
    float mouse_dx;
    float mouse_dy;
    mat3_t view_pitch_matrix;
    mat3_t view_yaw_matrix;
    mat3_t view_orientation;
    vec3_t view_translation;
    struct r_view_t *view;
    
//    printf("%d\n", in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE));
    
    if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_PRESSED)
    {
        if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_JUST_PRESSED)
        {
            in_RelativeMode(1);
        }
        
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
    }
}

void ed_WorldContextInput(void *context_data, struct ed_editor_window_t *window)
{
    struct ed_world_context_data_t *world_context_data;
    if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_PRESSED)
    {
        ed_FlyView((struct ed_editor_viewport_t *)window);
    }
    else
    {
        if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_JUST_RELEASED)
        {
            in_RelativeMode(0);
        }
        
        world_context_data = (struct ed_world_context_data_t *)context_data;
        world_context_data->active_sub_context->input_function(world_context_data->active_sub_context, (struct ed_editor_viewport_t *)window);
    }
}

void ed_WorldContextUpdate(void *context_data, struct ed_editor_window_t *window)
{
    struct ed_world_context_data_t *world_context_data;
    struct r_i_vertex_t *grid_verts = ed_center_grid;
    struct r_begin_submission_info_t begin_info = {};
    struct ed_editor_viewport_t *viewport = (struct ed_editor_viewport_t *)window;
    
    mat4_t_invvm(&viewport->inv_view_matrix, &viewport->view_matrix);
    mat4_t_persp(&viewport->projection_matrix, 0.68, (float)viewport->width / (float)viewport->height, 0.1, 500.0);
    
    begin_info.inv_view_matrix = viewport->inv_view_matrix;
    begin_info.projection_matrix = viewport->projection_matrix;
    begin_info.framebuffer = viewport->framebuffer;
    begin_info.viewport.minDepth = 0.0;
    begin_info.viewport.maxDepth = 1.0;
    begin_info.viewport.width = viewport->width;
    begin_info.viewport.height = viewport->height;
    begin_info.scissor.extent.width = viewport->width;
    begin_info.scissor.extent.height = viewport->height;
    begin_info.clear_framebuffer = 1;
    
    r_BeginSubmission(&begin_info);
    for(uint32_t object_index = 0; object_index < ed_objects.cursor; object_index++)
    {
        struct ed_object_t *object = ed_GetObjectPointer((struct ed_object_h){object_index});
        
        if(object)
        {
            r_Draw(object->start, object->count, r_GetDefaultMaterialPointer(), &object->transform);
        }
    }
    r_EndSubmission();
    begin_info.clear_framebuffer = 0;
    
    r_i_BeginSubmission(&begin_info);
    r_i_SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
    r_i_SetScissor(0, 0, viewport->width, viewport->height);
    for(int32_t quad_index = ED_GRID_X_WIDTH >> 1; quad_index >= 0; quad_index--)
    {
        r_i_DrawImmediate(grid_verts, 5, NULL, 0);
        grid_verts += 5;
    }
    for(int32_t quad_index = ED_GRID_Z_WIDTH >> 1; quad_index >= 0; quad_index--)
    {
        r_i_DrawImmediate(grid_verts, 5, NULL, 0);
        grid_verts += 5;
    }
    
    
    r_i_SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    r_i_DrawImmediate(ed_translation_manipulator_verts, ED_TRANSLATION_MANIPULATOR_VERT_COUNT, 
                      ed_translation_manipulator_indices, ED_TRANSLATION_MANIPULATOR_INDICE_COUNT);
    r_i_EndSubmission();
    
    world_context_data = (struct ed_world_context_data_t *)context_data;
    world_context_data->active_sub_context->update_function(world_context_data->active_sub_context, (struct ed_editor_viewport_t *)window);
}

void ed_WorldContextWorldSubContextInput(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport)
{
    if(in_GetKeyState(SDL_SCANCODE_LSHIFT) & IN_INPUT_STATE_PRESSED)
    {
        if(in_GetKeyState(SDL_SCANCODE_A) & IN_INPUT_STATE_JUST_PRESSED)
        {
            igSetNextWindowPos((ImVec2){300.0, 300.0}, 0, (ImVec2){0.0, 0.0});
            igOpenPopup("AddToWorldMenu", 0);
        }
    }
    
//    else if(in_GetMouseState(IN_MOUSE_BUTTON_LEFT) & IN_INPUT_STATE_JUST_PRESSED)
    {
        ed_WorldContextPickObject(viewport);
    }
    
    if(igBeginPopup("AddToWorldMenu", 0))
    {
        igMenuItemBool("BALLS", NULL, 0, 1);
        igMenuItemBool("SHIT", NULL, 0, 1);
        igEndPopup();
    }
}

void ed_WorldContextWorldSubContextUpdate(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport)
{
    
}

struct ed_object_h ed_WorldContextPickObject(struct ed_editor_viewport_t *viewport)
{
    VkViewport vk_viewport = {};
    VkRect2D scissor = {};
    int32_t mouse_x;
    int32_t mouse_y;
    union r_command_buffer_h command_buffer;
    struct r_submit_info_t submit_info = {};
    struct r_render_pass_t *render_pass;
    struct r_pipeline_t *pipeline;
    struct r_buffer_heap_t *vertex_heap;
    struct r_framebuffer_t *framebuffer;
    struct r_framebuffer_description_t *framebuffer_description;
    struct r_render_pass_begin_info_t begin_info = {};
    mat4_t model_view_projection_matrix;
    mat4_t view_projection_matrix;
    struct r_view_t *view;
    
    struct
    {
        mat4_t model_view_projection_matrix;
        uint32_t object_index;
    }push_constant;
    
    
    in_GetMousePos(&mouse_x, &mouse_y);

    mouse_x -= viewport->x;
    mouse_y -= viewport->y;
    
//    r_RecomputeInvViewMatrix();
    mat4_t_mul(&view_projection_matrix, &viewport->inv_view_matrix, &viewport->projection_matrix);
    
    vk_viewport.width = (float)viewport->width;
    vk_viewport.height = (float)viewport->height;
    vk_viewport.minDepth = 0.0;
    vk_viewport.maxDepth = 1.0;
    
    scissor.extent.width = viewport->width;
    scissor.extent.height = viewport->height;
    
    command_buffer = r_AllocateCommandBuffer();
    render_pass = r_GetRenderPassPointer(ed_picking_render_pass);
    pipeline = r_GetPipelinePointer(render_pass->pipelines[0]);
    vertex_heap = r_GetHeapPointer(r_vertex_heap);
    framebuffer = r_GetFramebufferPointer(ed_picking_framebuffer);
    
    begin_info.render_pass = ed_picking_render_pass;
    begin_info.render_area = scissor;
//    begin_info.render_area = (VkRect2D){
//        .extent.width = 1,
//        .extent.height = 600,
//        .offset.x = 0,
//        .offset.y = 0,
//    };
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
    r_vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
    r_vkCmdSetViewport(command_buffer, 0, 1, &vk_viewport);
    r_vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    r_vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    for(uint32_t object_index = 0; object_index < ed_objects.cursor; object_index++)
    {
        struct ed_object_t *object = ed_GetObjectPointer((struct ed_object_h){object_index});
        if(object)
        {
            mat4_t_identity(&push_constant.model_view_projection_matrix);
            mat4_t_mul(&push_constant.model_view_projection_matrix, &object->transform, &view_projection_matrix);
            push_constant.object_index = (object_index + 1);
            r_vkCmdPushConstants(command_buffer, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constant), &push_constant);
            r_vkCmdDraw(command_buffer, object->count, 1, object->start, 0);
        }
    }

    r_vkCmdEndRenderPass(command_buffer);
    r_vkEndCommandBuffer(command_buffer);
    
    submit_info.s_type = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.command_buffers = &command_buffer;
    submit_info.command_buffer_count = 1;
    
    r_vkResetFences(1, &r_draw_fence);
    r_vkQueueSubmit(r_draw_queue, 1, &submit_info, r_draw_fence);
    r_vkWaitForFences(1, &r_draw_fence, VK_TRUE, 0xffffffffffffffff);
    
    VkSubresourceLayout layout;
    VkImageSubresource subresource;
    
    subresource.mipLevel = 0;
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.arrayLayer = 0;
    
    struct r_texture_t *texture = r_GetTexturePointer(framebuffer->textures[0]);
    struct r_image_t *image = r_GetImagePointer(texture->image);
    vkGetImageSubresourceLayout(r_GetDevice(), image->image, &subresource, &layout);
    
    uint32_t row = layout.rowPitch * mouse_y;
    printf("col: %d, row: %d   %d\n", mouse_x, mouse_y, *(uint32_t *)((char *)ed_picking_memory + row + mouse_x * sizeof(uint32_t)));
}









