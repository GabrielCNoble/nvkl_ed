#include <stdio.h>
#include "ed.h"
#include "neighbor/r_draw.h"
#include "neighbor/lib/dstuff/ds_mem.h"
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
uint32_t *ed_picking_memory;
uint32_t ed_frame;

struct stack_list_t ed_objects;

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
    struct ed_world_context_data_t *data = ed_contexts[ED_CONTEXT_WORLD].context_data;
    data->sub_contexts = mem_Calloc(ED_WORLD_CONTEXT_SUB_CONTEXT_LAST, sizeof(struct ed_world_context_sub_context_t));
    struct ed_world_context_sub_context_t *sub_context = data->sub_contexts + ED_WORLD_CONTEXT_SUB_CONTEXT_WORLD;
    sub_context->input_function = ed_WorldContextWorldSubContextInput;
    sub_context->update_function = ed_WorldContextWorldSubContextUpdate;
    data->active_sub_context = sub_context;
    
    
//    data->selections = create_list(sizeof(struct ed_object_h), 512);
    
    ed_windows = mem_Calloc(1, sizeof(struct ed_editor_viewport_t));
    ed_last_window = (struct ed_editor_window_t *)ed_windows;
    ed_active_window = (struct ed_editor_window_t *)ed_windows;
    ed_windows->attached_context = ed_contexts + ED_CONTEXT_WORLD;
    
    struct ed_editor_viewport_t *viewport = (struct ed_editor_viewport_t *)ed_windows;
    viewport->base.type = ED_EDITOR_WINDOW_TYPE_VIEWPORT;
    viewport->width = 800;
    viewport->height = 600;
    viewport->view_pitch = 0.0;
    viewport->view_yaw = 0.0;

    brush = bsh_CreateCubeBrush(&brush_position, &brush_orientation, &brush_scale);
    
    struct r_shader_description_t shader_description = {
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .push_constant_count = 2,
        .push_constants = (struct r_push_constant_t []){
            [0] = {
                .offset = 0,
                .size = sizeof(mat4_t),
            },
            [1] = {
                .offset = sizeof(mat4_t),
                .size = sizeof(int)
            }
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
                .pipeline_descriptions = (struct r_pipeline_description_t []){
                    [0] = {
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
    
    ed_picking_render_pass = r_CreateRenderPass(&render_pass_description);
    
    struct r_framebuffer_description_t framebuffer_description = {
        .width = 1,
        .height = 1,
        .frame_count = 1,
        .render_pass = r_GetRenderPassPointer(ed_picking_render_pass)
    };
    
    ed_picking_framebuffer = r_CreateFramebuffer(&framebuffer_description);
    struct r_framebuffer_t *framebuffer = r_GetFramebufferPointer(ed_picking_framebuffer);
    struct r_texture_t *texture = r_GetTexturePointer(framebuffer->textures[0]);
    struct r_image_t *image = r_GetImagePointer(texture->image);
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
    
//    ed_FlyView();
    ed_UpdateWindows();
    r_ed_DrawBrushes();
    
//    pos_time += 0.01;
//    rot_time += 0.0372;
    
    
//    printf("%d\n", bsh_ClassifyPolygon(brush->polygons, &vec3_t_c(0.0, 0.4999999, 0.0), &vec3_t_c(0.0, 1.0, 0.0)));
    
//    view = r_GetViewPointer();
//    vec3_t normal = vec3_t_c(cos(rot_time), 1.0, sin(rot_time));
//    vec3_t normal = vec3_t_c(-1.0, 1.0, 0.0);
//    vec3_t_normalize(&normal, &normal);
//    if(bsh_SplitPolygon(brush->polygons, &vec3_t_c(0.0, 0.0, 0.0), &normal, &front, &back))
//    {
//        r_i_BeginSubmission(&view->inv_view_matrix, &view->projection_matrix);
//
//        if(!front || !back)
//        {
//            r_ed_DrawPolygon(brush->polygons, &vec3_t_c(0.0, 1.0, 0.0), 0);
//        }
//        
//        if(front)
//        {
//            r_ed_DrawPolygon(front, &vec3_t_c(1.0, 0.0, 0.0), 0);
//            mem_Free(front->vertices);
//            mem_Free(front);
//        }
//        
//        if(back)
//        {
//            r_ed_DrawPolygon(back, &vec3_t_c(0.0, 0.0, 1.0), 0);
//            mem_Free(back->vertices);
//            mem_Free(back); 
//        }
//        
//        r_i_EndSubmission();
//    }
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
        
        if(context->update_frame != ed_frame)
        {
            context->update_frame = ed_frame;
            
            if(window == ed_active_window)
            {
                context->input_function(context->context_data, window);
            }
            
            context->update_function(context->context_data, window);
        }
        
        window = window->next;
    }
    
    ed_frame++;
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

//void ed_DrawObjects(union r_command_buffer_h command_buffer)
//{
//    struct ed_object_t *object;
//    object = ed_objects;
//    
//    while(object)
//    {
//        
//    }
//}

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
        
        view_translation.x += view->view_transform.rows[3].x;
        view_translation.y += view->view_transform.rows[3].y;
        view_translation.z += view->view_transform.rows[3].z;
        
        mat4_t_comp(&view->view_transform, &view_orientation, &view_translation);
        r_RecomputeInvViewMatrix();
    }
    else if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_JUST_RELEASED)
    {
        in_RelativeMode(0);
    }
}

void ed_WorldContextInput(void *context_data, struct ed_editor_window_t *window)
{
    struct ed_world_context_data_t *world_context_data;
    if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_PRESSED)
    {
        ed_FlyView((struct ed_editor_viewport_t *)window);
        printf("ed_FlyView\n");
    }
    else
    {
        world_context_data = (struct ed_world_context_data_t *)context_data;
        world_context_data->active_sub_context->input_function(world_context_data->active_sub_context, (struct ed_editor_viewport_t *)window);
    }
}

void ed_WorldContextUpdate(void *context_data, struct ed_editor_window_t *window)
{
    struct ed_world_context_data_t *world_context_data;
    world_context_data = (struct ed_world_context_data_t *)context_data;
    world_context_data->active_sub_context->update_function(world_context_data->active_sub_context, (struct ed_editor_viewport_t *)window);
}

void ed_WorldContextWorldSubContextInput(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport)
{
    printf("ed_WorldContextWorldSubContextInput\n");
}

void ed_WorldContextWorldSubContextUpdate(struct ed_world_context_sub_context_t *sub_context, struct ed_editor_viewport_t *viewport)
{
    
}









