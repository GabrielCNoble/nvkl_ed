#include "ed_w_ctx.h"
#include "neighbor/in.h"
#include "neighbor/r_draw.h"
#include "neighbor/lib/dstuff/ds_mem.h"

extern struct ed_context_t ed_contexts[ED_CONTEXT_LAST];
extern struct r_render_pass_handle_t ed_picking_render_pass;
struct list_t ed_object_handles;

uint32_t ed_max_outline_verts;
struct r_i_vertex_t *ed_outline_verts;
uint32_t ed_max_outline_indices;
uint32_t *ed_outline_indices;

extern struct r_heap_h r_vertex_heap;
extern struct r_heap_h r_index_heap;
extern VkQueue r_draw_queue;
extern VkFence r_draw_fence;

#define ED_GRID_X_WIDTH 20
#define ED_GRID_Z_WIDTH 20
struct r_i_vertex_t *ed_center_grid;
struct r_i_vertex_t *ed_creating_brush_verts;
struct r_i_vertex_t *ed_translation_manipulator_verts;
uint32_t *ed_translation_manipulator_indices;
struct r_chunk_h ed_translation_manipulator_vert_chunk;
struct r_chunk_h ed_translation_manipulator_index_chunk;
#define ED_TRANSLATION_MANIPULATOR_SHAFT_VERT_COUNT 13
#define ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT 60
#define ED_TRANSLATION_MANIPULATOR_PLANE_VERT_COUNT 4
#define ED_TRANSLATION_MANIPULATOR_PLANE_INDICE_COUNT 6
#define ED_TRANSLATION_MANIPULATOR_VERT_COUNT ((ED_TRANSLATION_MANIPULATOR_SHAFT_VERT_COUNT + ED_TRANSLATION_MANIPULATOR_PLANE_VERT_COUNT) * 3)
#define ED_TRANSLATION_MANIPULATOR_INDICE_COUNT (ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT + ED_TRANSLATION_MANIPULATOR_PLANE_INDICE_COUNT)
#define ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT (ED_TRANSLATION_MANIPULATOR_SHAFT_VERT_COUNT + ED_TRANSLATION_MANIPULATOR_PLANE_VERT_COUNT)

struct r_i_draw_state_t ed_selected_face_fill_draw_state = {
    .line_width = 1.0,
    .texture = R_INVALID_TEXTURE_HANDLE,
    .pipeline_state.input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .pipeline_state.rasterizer_state.polygon_mode = VK_POLYGON_MODE_FILL,
    .pipeline_state.rasterizer_state.front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .pipeline_state.rasterizer_state.cull_mode = VK_CULL_MODE_BACK_BIT,
    .pipeline_state.depth_state.compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
    .pipeline_state.depth_state.test_enable = VK_TRUE,
    .pipeline_state.depth_state.write_enable = VK_TRUE,
    .pipeline_state.color_blend_state.test_enable = VK_TRUE,
    .pipeline_state.color_blend_state.blend_constants = (float []){1.0, 1.0, 1.0, 1.0},
    .pipeline_state.color_blend_state.src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
    .pipeline_state.color_blend_state.dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .pipeline_state.color_blend_state.src_alpha_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
    .pipeline_state.color_blend_state.dst_alpha_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .pipeline_state.color_blend_state.color_blend_op = VK_BLEND_OP_ADD,
    .pipeline_state.color_blend_state.alpha_blend_op = VK_BLEND_OP_ADD,
    .pipeline_state.color_blend_state.color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
};

struct r_i_draw_state_t ed_selected_face_ouline_draw_state = {
    .line_width = 3.0,
    .texture = R_INVALID_TEXTURE_HANDLE,
    .pipeline_state.input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
    .pipeline_state.rasterizer_state.polygon_mode = VK_POLYGON_MODE_LINE,
    .pipeline_state.rasterizer_state.front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .pipeline_state.rasterizer_state.cull_mode = VK_CULL_MODE_NONE,
    .pipeline_state.depth_state.compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
    .pipeline_state.depth_state.test_enable = VK_TRUE,
    .pipeline_state.depth_state.write_enable = VK_TRUE,
    .pipeline_state.color_blend_state.test_enable = VK_TRUE,
    .pipeline_state.color_blend_state.blend_constants = (float []){1.0, 1.0, 1.0, 1.0},
    .pipeline_state.color_blend_state.src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
    .pipeline_state.color_blend_state.dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .pipeline_state.color_blend_state.src_alpha_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
    .pipeline_state.color_blend_state.dst_alpha_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .pipeline_state.color_blend_state.color_blend_op = VK_BLEND_OP_ADD,
    .pipeline_state.color_blend_state.alpha_blend_op = VK_BLEND_OP_ADD,
    .pipeline_state.color_blend_state.color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
};

struct r_i_draw_state_t ed_selected_object_outline_draw_state = {
    .line_width = 3.0,
    .texture = R_INVALID_TEXTURE_HANDLE,
    .pipeline_state.input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .pipeline_state.rasterizer_state.polygon_mode = VK_POLYGON_MODE_LINE,
    .pipeline_state.rasterizer_state.front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .pipeline_state.rasterizer_state.cull_mode = VK_CULL_MODE_FRONT_BIT,
    .pipeline_state.depth_state.compare_op = VK_COMPARE_OP_LESS,
    .pipeline_state.depth_state.test_enable = VK_TRUE,
    .pipeline_state.depth_state.write_enable = VK_TRUE,
    .pipeline_state.color_blend_state.test_enable = VK_FALSE,
};

struct r_i_draw_state_t ed_center_grid_draw_state = {
    .line_width = 1.0,
    .texture = R_INVALID_TEXTURE_HANDLE,
    .pipeline_state.input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
    .pipeline_state.depth_state.compare_op = VK_COMPARE_OP_LESS,
    .pipeline_state.depth_state.test_enable = VK_TRUE,
    .pipeline_state.depth_state.write_enable = VK_TRUE,
};

struct r_i_draw_state_t ed_creating_brush_draw_state = {
    .line_width = 2.0,
    .texture = R_INVALID_TEXTURE_HANDLE,
    .pipeline_state.input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
    .pipeline_state.depth_state.compare_op = VK_COMPARE_OP_LESS,
    .pipeline_state.depth_state.test_enable = VK_TRUE,
    .pipeline_state.depth_state.write_enable = VK_TRUE,
    .pipeline_state.color_blend_state.test_enable = VK_TRUE,
    .pipeline_state.color_blend_state.blend_constants = (float []){1.0, 1.0, 1.0, 1.0},
    .pipeline_state.color_blend_state.src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
    .pipeline_state.color_blend_state.dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .pipeline_state.color_blend_state.src_alpha_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
    .pipeline_state.color_blend_state.dst_alpha_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .pipeline_state.color_blend_state.color_blend_op = VK_BLEND_OP_ADD,
    .pipeline_state.color_blend_state.alpha_blend_op = VK_BLEND_OP_ADD,
    .pipeline_state.color_blend_state.color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
};

struct ed_manipulator_t ed_manipulators[ED_TRANSFORM_TYPE_LAST] = {
    [ED_TRANSFORM_TYPE_TRANSLATION] = {
        .component_count = 3,
        .components = (struct ed_manipulator_component_t []){
            {
                .vertex_start = 0, 
                .index_start = 0, 
                .count = ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT, 
                .id = ED_MANIPULATOR_AXIS_ID_X_AXIS,
            },
            
            {
                .vertex_start = ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT, 
                .index_start = 0, 
                .count = ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT, 
                .id = ED_MANIPULATOR_AXIS_ID_Y_AXIS,
            },
            
            {
                .vertex_start = ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT * 2, 
                .index_start = 0, 
                .count = ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT, 
                .id = ED_MANIPULATOR_AXIS_ID_Z_AXIS,
            },
        }
    }
};

vec3_t ed_manipulator_plane_normal[] = {
    [ED_MANIPULATOR_AXIS_ID_X_AXIS] = vec3_t_c(1.0, 0.0, 0.0),
    [ED_MANIPULATOR_AXIS_ID_Y_AXIS] = vec3_t_c(0.0, 1.0, 0.0),
    [ED_MANIPULATOR_AXIS_ID_Z_AXIS] = vec3_t_c(0.0, 0.0, 1.0),
};

void (*ed_w_ctx_StateFunction[])(struct ed_world_context_data_t *data, struct ed_editor_window_t *window) = {
    [ED_WORLD_CONTEXT_STATE_IDLE] = ed_w_ctx_IdleState,
    [ED_WORLD_CONTEXT_STATE_LEFT_CLICK] = ed_w_ctx_LeftClickState,
    [ED_WORLD_CONTEXT_STATE_RIGHT_CLICK] = ed_w_ctx_RightClickState,
    [ED_WORLD_CONTEXT_STATE_PICK_OBJECT] = ed_w_ctx_PickObjectState,
    [ED_WORLD_CONTEXT_STATE_PICK_BRUSH_FACE] = ed_w_ctx_PickBrushFaceState,
    [ED_WORLD_CONTEXT_STATE_MANIPULATING] = ed_w_ctx_ManipulatingState,
    [ED_WORLD_CONTEXT_STATE_CREATING_BRUSH] = ed_w_ctx_CreatingBrushState,
    [ED_WORLD_CONTEXT_STATE_FLYING] = ed_w_ctx_FlyingState,
};

void ed_w_ctx_Init()
{
//    ed_objects = create_stack_list(sizeof(struct ed_object_t), 512);
//    ed_temp_objects = create_stack_list(sizeof(struct ed_object_t), 512);
    ed_object_handles = create_list(sizeof(struct ed_object_h), 512);
    
    ed_contexts[ED_CONTEXT_WORLD].input_function = ed_w_ctx_Input;
    ed_contexts[ED_CONTEXT_WORLD].update_function = ed_w_ctx_Update;
    ed_contexts[ED_CONTEXT_WORLD].context_data = mem_Calloc(1, sizeof(struct ed_world_context_data_t));
    ed_contexts[ED_CONTEXT_WORLD].update_frame = 0xffffffff;
    
    struct ed_world_context_data_t *data;
    struct ed_world_context_sub_context_t *sub_context;
    
    data = ed_contexts[ED_CONTEXT_WORLD].context_data;
    
//    data->selections = mem_Calloc(ED_WORLD_CONTEXT_MODE_LAST, sizeof(struct list_t));
//    data->selections[0] = create_list(sizeof(struct ed_object_h), 512);
//    data->selections[1] = create_list(sizeof(struct ed_object_h), 512);
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].selections = create_list(sizeof(struct ed_object_h), 512);
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects = create_stack_list(sizeof(struct ed_object_t), 512);
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].selections = create_list(sizeof(struct ed_object_h), 512);
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].objects = create_stack_list(sizeof(struct ed_object_t), 512);
    data->manipulation_state.linear_threshold = 1.0;
    
    data->current_selection_target = ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT;
    mat4_t_identity(&data->manipulator_state.transform);
    
    
    
    
//    data->sub_contexts = mem_Calloc(ED_WORLD_CONTEXT_SUB_CONTEXT_LAST, sizeof(struct ed_world_context_sub_context_t));
    
//    sub_context = data->sub_contexts + ED_WORLD_CONTEXT_SUB_CONTEXT_WORLD;
//    sub_context->input_function = ed_WorldContextWorldSubContextInput;
//    sub_context->update_function = ed_WorldContextWorldSubContextUpdate;
//    sub_context->selections = create_list(sizeof(struct ed_object_h), 512);
//    sub_context->objects = create_list(sizeof(struct ed_object_h), 512);
//    mat4_t_identity(&sub_context->manipulator_state.transform);
//    sub_context->manipulator_state.picked_axis = 0;
//    data->active_sub_context = sub_context;
    
//    sub_context = data->sub_contexts + ED_WORLD_CONTEXT_SUB_CONTEXT_BRUSH;
//    sub_context->input_function = ed_WorldContextBrushSubContextInput;
//    sub_context->update_function = ed_WorldContextBrushSubContextUpdate;
//    sub_context->selections = create_list(sizeof(struct ed_object_h), 512);
//    sub_context->objects = create_list(sizeof(struct ed_object_h), 512);
//    mat4_t_identity(&sub_context->manipulator_state.transform);
//    sub_context->manipulator_state.picked_axis = 0;
    
    
    /* 4 line loops (which for now needs 5 verts) */
    ed_creating_brush_verts = mem_Calloc(20, sizeof(struct r_i_vertex_t));
    
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
    ed_translation_manipulator_verts = mem_Calloc(ED_TRANSLATION_MANIPULATOR_VERT_COUNT, sizeof(struct r_i_vertex_t));
    ed_translation_manipulator_indices = mem_Calloc(ED_TRANSLATION_MANIPULATOR_INDICE_COUNT, sizeof(uint32_t));
    
    #define MANIPULATOR_SHAFT_THICKNESS 0.05
    
    /* X axis */
    ed_translation_manipulator_verts[0].position = vec4_t_c(0.5, MANIPULATOR_SHAFT_THICKNESS,-MANIPULATOR_SHAFT_THICKNESS, 1.0);
    ed_translation_manipulator_verts[0].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[1].position = vec4_t_c(0.5, MANIPULATOR_SHAFT_THICKNESS, MANIPULATOR_SHAFT_THICKNESS, 1.0);
    ed_translation_manipulator_verts[1].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[2].position = vec4_t_c(2.5, MANIPULATOR_SHAFT_THICKNESS, MANIPULATOR_SHAFT_THICKNESS, 1.0);
    ed_translation_manipulator_verts[2].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[3].position = vec4_t_c(2.5, MANIPULATOR_SHAFT_THICKNESS,-MANIPULATOR_SHAFT_THICKNESS, 1.0);
    ed_translation_manipulator_verts[3].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[4].position = vec4_t_c(0.5,-MANIPULATOR_SHAFT_THICKNESS,-MANIPULATOR_SHAFT_THICKNESS, 1.0);
    ed_translation_manipulator_verts[4].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[5].position = vec4_t_c(0.5,-MANIPULATOR_SHAFT_THICKNESS, MANIPULATOR_SHAFT_THICKNESS, 1.0);
    ed_translation_manipulator_verts[5].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[6].position = vec4_t_c(2.5,-MANIPULATOR_SHAFT_THICKNESS, MANIPULATOR_SHAFT_THICKNESS, 1.0);
    ed_translation_manipulator_verts[6].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[7].position = vec4_t_c(2.5,-MANIPULATOR_SHAFT_THICKNESS,-MANIPULATOR_SHAFT_THICKNESS, 1.0);
    ed_translation_manipulator_verts[7].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    
    ed_translation_manipulator_verts[8].position = vec4_t_c(2.5, 0.2,-0.2, 1.0);
    ed_translation_manipulator_verts[8].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[9].position = vec4_t_c(2.5,-0.2,-0.2, 1.0);
    ed_translation_manipulator_verts[9].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[10].position = vec4_t_c(2.5,-0.2, 0.2, 1.0);
    ed_translation_manipulator_verts[10].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[11].position = vec4_t_c(2.5, 0.2, 0.2, 1.0);
    ed_translation_manipulator_verts[11].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[12].position = vec4_t_c(3.0, 0.0, 0.0, 1.0);
    ed_translation_manipulator_verts[12].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    
    ed_translation_manipulator_verts[13].position = vec4_t_c(0.4, 0.0, 0.4, 1.0);
    ed_translation_manipulator_verts[13].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[14].position = vec4_t_c(0.4, 0.0, 0.8, 1.0);
    ed_translation_manipulator_verts[14].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[15].position = vec4_t_c(0.8, 0.0, 0.8, 1.0);
    ed_translation_manipulator_verts[15].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    ed_translation_manipulator_verts[16].position = vec4_t_c(0.8, 0.0, 0.4, 1.0);
    ed_translation_manipulator_verts[16].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    
    
    
    /* manipulator shaft */
    ed_translation_manipulator_indices[0]  = 0;
    ed_translation_manipulator_indices[1]  = 1;
    ed_translation_manipulator_indices[2]  = 2;
    ed_translation_manipulator_indices[3]  = 2;
    ed_translation_manipulator_indices[4]  = 3;
    ed_translation_manipulator_indices[5]  = 0;
    
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
    
    
    /* manipulator tip */
    ed_translation_manipulator_indices[24] = 8;
    ed_translation_manipulator_indices[25] = 11;
    ed_translation_manipulator_indices[26] = 12;
    ed_translation_manipulator_indices[27] = 11;
    ed_translation_manipulator_indices[28] = 10;
    ed_translation_manipulator_indices[29] = 12;
    
    ed_translation_manipulator_indices[30] = 8;
    ed_translation_manipulator_indices[31] = 12;
    ed_translation_manipulator_indices[32] = 9;
    ed_translation_manipulator_indices[33] = 9;
    ed_translation_manipulator_indices[34] = 12;
    ed_translation_manipulator_indices[35] = 10;
    
    ed_translation_manipulator_indices[36] = 8;
    ed_translation_manipulator_indices[37] = 3;
    ed_translation_manipulator_indices[38] = 2;
    ed_translation_manipulator_indices[39] = 2;
    ed_translation_manipulator_indices[40] = 11;
    ed_translation_manipulator_indices[41] = 8;
    
    ed_translation_manipulator_indices[42] = 11;
    ed_translation_manipulator_indices[43] = 2;
    ed_translation_manipulator_indices[44] = 6;
    ed_translation_manipulator_indices[45] = 6;
    ed_translation_manipulator_indices[46] = 10;
    ed_translation_manipulator_indices[47] = 11;
    
    ed_translation_manipulator_indices[48] = 7;
    ed_translation_manipulator_indices[49] = 9;
    ed_translation_manipulator_indices[50] = 10;
    ed_translation_manipulator_indices[51] = 10;
    ed_translation_manipulator_indices[52] = 6;
    ed_translation_manipulator_indices[53] = 7;
    
    ed_translation_manipulator_indices[54] = 8;
    ed_translation_manipulator_indices[55] = 9;
    ed_translation_manipulator_indices[56] = 7;
    ed_translation_manipulator_indices[57] = 7;
    ed_translation_manipulator_indices[58] = 3;
    ed_translation_manipulator_indices[59] = 8;
    
    
    /* planes */
    ed_translation_manipulator_indices[ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT    ] = 13;
    ed_translation_manipulator_indices[ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT + 1] = 14;
    ed_translation_manipulator_indices[ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT + 2] = 15;
    ed_translation_manipulator_indices[ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT + 3] = 15;
    ed_translation_manipulator_indices[ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT + 4] = 16;
    ed_translation_manipulator_indices[ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT + 5] = 13;

    /* Y axis */
    for(uint32_t index = 0; index < ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT; index++)
    {
        struct r_i_vertex_t *vert = ed_translation_manipulator_verts + index;
        uint32_t offset = index + ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT;
        ed_translation_manipulator_verts[offset].position.x = vert->position.z;
        ed_translation_manipulator_verts[offset].position.y = vert->position.x;
        ed_translation_manipulator_verts[offset].position.z = vert->position.y;
        ed_translation_manipulator_verts[offset].position.w = 1.0;
        ed_translation_manipulator_verts[offset].color = vec4_t_c(0.0, 1.0, 0.0, 1.0);
    }
    
    /* Z axis */
    for(uint32_t index = 0; index < ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT; index++)
    {
        struct r_i_vertex_t *vert = ed_translation_manipulator_verts + index;
        uint32_t offset = index + ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT * 2;
        ed_translation_manipulator_verts[offset].position.y = vert->position.z;
        ed_translation_manipulator_verts[offset].position.x = vert->position.y;
        ed_translation_manipulator_verts[offset].position.z = vert->position.x;
        ed_translation_manipulator_verts[offset].position.w = 1.0;
        ed_translation_manipulator_verts[offset].color = vec4_t_c(0.0, 0.0, 1.0, 1.0);
    }
    
//    for(uint32_t index = 0; index < ED_TRANSLATION_MANIPULATOR_PLANE_VERT_COUNT; index++)
//    {
//        ed_translation_manipulator_verts[index + ED_TRANSLATION_MANIPULATOR_SHAFT_VERT_COUNT].color = vec4_t_c(0.0, 1.0, 0.0, 1.0);
//        ed_translation_manipulator_verts[index + ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT + ED_TRANSLATION_MANIPULATOR_SHAFT_VERT_COUNT].color = vec4_t_c(0.0, 0.0, 1.0, 1.0);
//        ed_translation_manipulator_verts[index + ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT * 2 + ED_TRANSLATION_MANIPULATOR_SHAFT_VERT_COUNT].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
//    }
    
    uint32_t size = ED_TRANSLATION_MANIPULATOR_VERT_COUNT * sizeof(struct r_i_vertex_t);
    ed_translation_manipulator_vert_chunk = r_AllocChunk(r_vertex_heap, size, sizeof(struct r_i_vertex_t));
    r_FillBufferChunk(ed_translation_manipulator_vert_chunk, ed_translation_manipulator_verts, size, 0);
    
    size = ED_TRANSLATION_MANIPULATOR_INDICE_COUNT * sizeof(uint32_t);
    ed_translation_manipulator_index_chunk = r_AllocChunk(r_index_heap, size, sizeof(uint32_t));
    r_FillBufferChunk(ed_translation_manipulator_index_chunk, ed_translation_manipulator_indices, size, 0);
    
    
    
//    ed_w_ctx_CreateBrushObject(&data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects, &vec3_t_c(0.0, 0.0, 0.0), );    
//    ed_WorldContextCreateBrushObject(&data->mode_data[ED_WORLD_CONTEXT_MODE_OBJECT].objects, &vec3_t_c(0.0, 0.0, 5.0));
//    ed_WorldContextCreateBrushObject(&data->mode_data[ED_WORLD_CONTEXT_MODE_OBJECT].objects, &vec3_t_c(0.0, 0.0,-5.0));
}

void ed_w_ctx_Input(void *context_data, struct ed_editor_window_t *window)
{
    struct ed_world_context_data_t *data = (struct ed_world_context_data_t *)context_data;
//    struct ed_editor_viewport_t *viewport = (struct ed_editor_viewport_t *)window;
    
    
    do
    {
        data->just_changed_state = 0;
        ed_w_ctx_StateFunction[data->context_state](data, window);
    }
    while(data->just_changed_state);
//    if(data->state_change_counter)
//    {
//        data->state_change_counter--;
//    }
    
//    struct stack_list_t *objects = NULL;
//    struct list_t *selections = NULL;
    
//    if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_PRESSED)
//    {
//        ed_FlyView((struct ed_editor_viewport_t *)window);
//    }
//    else
//    {
//        if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_JUST_RELEASED)
//        {
//            in_RelativeMode(0);
//        }
//
//        uint32_t shift_pressed = in_GetKeyState(SDL_SCANCODE_LSHIFT) & IN_INPUT_STATE_PRESSED;
//        uint32_t left_mouse_state = in_GetMouseState(IN_MOUSE_BUTTON_LEFT) & (IN_INPUT_STATE_JUST_RELEASED | IN_INPUT_STATE_JUST_PRESSED);
//        uint32_t right_mouse_state = in_GetMouseState(IN_MOUSE_BUTTON_RIGHT) & IN_INPUT_STATE_JUST_RELEASED;
//        
//        if((left_mouse_state || right_mouse_state) && !(data->manipulating))
//        {
//            /* object picking happens if the mouse button is released, and we weren't previously
//            manipulating something. Manipulator picking happens when the left button is pressed, and
//            we weren't previously manipulating something */
//            
//            mat4_t transform;
//            mat4_t_identity(&transform);
//            uint32_t picked_axis = ed_w_ctx_PickManipulator(&data->manipulator_state.transform, data->manipulator_state.transform_type, viewport);
//            
//            data->axis_constraint = 0;
//            
//            if((left_mouse_state & IN_INPUT_STATE_JUST_PRESSED) && picked_axis)
//            {
//                /* manipulator picking happens when the left button is pressed */
//                data->manipulator_state.picked_axis = picked_axis;
//                data->axis_constraint = picked_axis;
//            }
//            else
//            {
//                ed_object_handles.cursor = 0;
//                
//                if(left_mouse_state & IN_INPUT_STATE_JUST_RELEASED)
//                {
//                    /* object picking happens when the left button is released */
//                    data->current_selection_target = ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT;
//                    objects = &data->target_data[data->current_selection_target].objects;
//                    for(uint32_t object_index = 0; object_index < objects->cursor; object_index++)
//                    {
//                        if(ed_w_ctx_GetObjectPointer(objects, (struct ed_object_h){object_index}))
//                        {
//                            add_list_element(&ed_object_handles, &(struct ed_object_h){object_index});
//                        }
//                    }
//                }
//                else if(right_mouse_state & IN_INPUT_STATE_JUST_RELEASED)
//                {
//                    /* brush face picking happens when the right button is released */
//                    data->current_selection_target = ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH;
//                    objects = &data->target_data[data->current_selection_target].objects;
//                    
//                    objects->cursor = 0;
//                    objects->free_stack_top = 0xffffffff;
//                    
//                    for(uint32_t object_index = 0; object_index < data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects.cursor; object_index++)
//                    {
//                        struct ed_object_t *object = ed_w_ctx_GetObjectPointer(&data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects, (struct ed_object_h){object_index});
//                        if(object && object->type == ED_OBJECT_TYPE_BRUSH)
//                        {
//                            struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
//                            struct ed_brush_face_t *polygon = brush->polygons;
//                            uint32_t polygon_index = 0;
//                            while(polygon)
//                            {
//                                struct ed_object_h face_handle = ed_w_ctx_CreateObject(objects, 
//                                                    &object->transform, ED_OBJECT_TYPE_FACE, 0, 0, 0, 0, 
//                                                        (union ed_object_ref_t){.brush = object->object.brush, .face_index = polygon_index});
//                                struct ed_object_t *face_object = ed_w_ctx_GetObjectPointer(objects, face_handle);
//                                face_object->start = polygon->draw_indices_start + brush->start;
//                                face_object->count = polygon->draw_indices_count;
//                                face_object->vertex_count = polygon->indice_count;
//                                face_object->vertex_offset = object->vertex_offset;
//                                add_list_element(&ed_object_handles, &face_handle);
//                                polygon = polygon->next;
//                                polygon_index++;
//                            }
//                        }
//                    }
//                }
//                selections = &data->target_data[data->current_selection_target].selections;
//                struct ed_object_h selection = ed_w_ctx_PickObject(objects, &ed_object_handles, viewport);
//                
//                switch(data->current_selection_target)
//                {
//                    case ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT:
//                        ed_w_ctx_AppendObjectSelection(selections, selection, shift_pressed);
//                    break;
//                    
//                    case ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH:
//                        ed_w_ctx_AppendBrushSelection(selections, selection, shift_pressed);
//                    break;
//                }
//            }
//        }
//        
//        selections = &data->target_data[data->current_selection_target].selections;
//        objects = &data->target_data[data->current_selection_target].objects;
//        
//        
//        if(selections->cursor)
//        {
//            data->manipulator_state.transform.rows[3] = vec4_t_c(0.0, 0.0, 0.0, 0.0);   
//            
//            switch(data->current_selection_target)
//            {
//                case ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT:
//                    for(uint32_t object_index = 0; object_index < selections->cursor; object_index++)
//                    {
//                        struct ed_object_h handle = *(struct ed_object_h *)get_list_element(selections, object_index);
//                        struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
//                        vec4_t_add(&data->manipulator_state.transform.rows[3], &data->manipulator_state.transform.rows[3], &object->transform.rows[3]);
//                    }
//                break;
//                
//                case ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH:
//                    for(uint32_t object_index = 0; object_index < selections->cursor; object_index++)
//                    {
//                        struct ed_object_h handle = *(struct ed_object_h *)get_list_element(selections, object_index);
//                        struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
//                        struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
//                        struct ed_brush_face_t *face = ed_GetBrushFacePointer(object->object.brush, object->object.face_index);
//                        
//                        vec3_t face_position = vec3_t_c(0.0, 0.0, 0.0);
//                        for(uint32_t vert_index = 0; vert_index < face->indice_count; vert_index++)
//                        {
//                            vec3_t_add(&face_position, &face_position, brush->vertices + face->indices[vert_index]);
//                        }
//                        face_position.x /= (float)face->indice_count;
//                        face_position.y /= (float)face->indice_count;
//                        face_position.z /= (float)face->indice_count;  
//                        
//                        vec4_t_add(&data->manipulator_state.transform.rows[3], &data->manipulator_state.transform.rows[3], &object->transform.rows[3]);
//                        data->manipulator_state.transform.rows[3].x += face_position.x;
//                        data->manipulator_state.transform.rows[3].y += face_position.y;
//                        data->manipulator_state.transform.rows[3].z += face_position.z;
//                    }
//                break;
//            }
//            
//            data->manipulator_state.transform.rows[3].x /= (float)selections->cursor;
//            data->manipulator_state.transform.rows[3].y /= (float)selections->cursor;
//            data->manipulator_state.transform.rows[3].z /= (float)selections->cursor;
//            data->manipulator_state.transform.rows[3].w = 1.0;
//                
//            if(in_GetMouseState(IN_MOUSE_BUTTON_LEFT) & IN_INPUT_STATE_PRESSED)
//            {
//                vec3_t manipulator_pos = vec3_t_c_vec4_t(&data->manipulator_state.transform.rows[3]);
//                vec3_t plane_point;
//                vec3_t camera_pos = vec3_t_c_vec4_t(&viewport->view_matrix.rows[3]);
//                vec3_t camera_plane_vec;
//                vec3_t drag_delta;
//                
//                float fovy = 0.68;
//                float aspect = (float)viewport->width / (float)viewport->height;
//                float top = tanf(fovy) * 0.1;
//                float right = top * aspect;
//                
//                vec3_t right_vec = vec3_t_c_vec4_t(&viewport->view_matrix.rows[0]);
//                vec3_t up_vec = vec3_t_c_vec4_t(&viewport->view_matrix.rows[1]);
//                vec3_t forward_vec = vec3_t_c_vec4_t(&viewport->view_matrix.rows[2]);
//                vec3_t mouse_vec = vec3_t_c(0.0, 0.0, 0.0);
//
//                float mouse_x = ((float)viewport->mouse_x / (float)viewport->width) * 2.0 - 1.0;
//                float mouse_y = 1.0 - ((float)viewport->mouse_y / (float)viewport->height) * 2.0;
//                
//                vec3_t_mul(&right_vec, &right_vec, right * mouse_x);
//                vec3_t_mul(&up_vec, &up_vec, top * mouse_y);
//                vec3_t_mul(&forward_vec, &forward_vec, -0.1);
//                
//                vec3_t_add(&mouse_vec, &right_vec, &up_vec);
//                vec3_t_add(&mouse_vec, &mouse_vec, &forward_vec);
//                vec3_t_normalize(&mouse_vec, &mouse_vec);
//                vec3_t_sub(&camera_plane_vec, &manipulator_pos, &camera_pos);
//                
//                if(left_mouse_state & IN_INPUT_STATE_JUST_PRESSED)
//                {
//                    vec3_t plane_normal;
//                    if(data->axis_constraint)
//                    {
//                        data->axis_clamp = ed_manipulator_plane_normal[data->axis_constraint];
//                        plane_normal = ed_manipulator_plane_normal[data->axis_constraint];
//                        
//                        float dist = vec3_t_dot(&camera_plane_vec, &plane_normal);
//                        vec3_t_fmadd(&plane_point, &manipulator_pos, &plane_normal, -dist);
//                        vec3_t_sub(&plane_normal, &camera_pos, &plane_point);
//                        vec3_t_normalize(&data->pick_normal, &plane_normal);
//                    }
//                    else
//                    {
//                        data->axis_clamp = vec3_t_c(1.0, 1.0, 1.0);
//                        data->pick_normal = forward_vec;
//                    }
//                }
//                
//                
//                
//                float d = vec3_t_dot(&mouse_vec, &data->pick_normal);
//                float t = 0.0;
//                
//                if(d)
//                {
//                    t = vec3_t_dot(&camera_plane_vec, &data->pick_normal) / d;                
//                }
//                
//                vec3_t_fmadd(&plane_point, &camera_pos, &mouse_vec, t);
//                
//                if(left_mouse_state & IN_INPUT_STATE_JUST_PRESSED)
//                {
//                    vec3_t_sub(&data->pick_offset, &manipulator_pos, &plane_point);
//                }
//                
//                vec3_t_add(&plane_point, &plane_point, &data->pick_offset);
//                vec3_t_sub(&drag_delta, &plane_point, &manipulator_pos);
//                drag_delta.x *= data->axis_clamp.x;
//                drag_delta.y *= data->axis_clamp.y;
//                drag_delta.z *= data->axis_clamp.z;
//                
//                if(in_GetDragState() & IN_DRAG_STATE_JUST_STARTED_DRAGGING)
//                {
//                    /* to avoid having objects being unselected once the user releases the button, 
//                    we keep track of whether the release happens while we're manipulating */
//                    data->manipulating = 1;
//                }
//                
//                mat4_t transform;
//                mat4_t_identity(&transform);
//                
//                transform.rows[3] = vec4_t_c(drag_delta.x, drag_delta.y, drag_delta.z, 1.0);
//                
//                ed_w_ctx_ApplyTransform(&transform, objects, selections, ED_TRANSFORM_TYPE_TRANSLATION, ED_TRANSFORM_MODE_WORLD);
//            }
//            else
//            {
//                data->manipulating = 0;
//            }
//        }
//    }
}

void ed_w_ctx_Update(void *context_data, struct ed_editor_window_t *window)
{
    struct r_i_vertex_t *grid_verts = ed_center_grid;
    struct r_begin_submission_info_t begin_info = {};
    struct r_i_draw_state_t draw_state = {};
    struct ed_editor_viewport_t *viewport = (struct ed_editor_viewport_t *)window;
    struct ed_world_context_data_t *data = (struct ed_world_context_data_t *)context_data;
    
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
    
    draw_state.scissor.extent.width = viewport->width;
    draw_state.scissor.extent.height = viewport->height;
    draw_state.line_width = ed_center_grid_draw_state.line_width;
    draw_state.texture = ed_center_grid_draw_state.texture;
    draw_state.pipeline_state = ed_center_grid_draw_state.pipeline_state;
    
    
    struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects;
    struct list_t *selections = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].selections;
    
    r_BeginSubmission(&begin_info);
    for(uint32_t object_index = 0; object_index < objects->cursor; object_index++)
    {
        struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, (struct ed_object_h){object_index});
        
        if(object)
        {
            if(object->vertex_offset == 0xffffffff)
            {
                r_Draw(object->start, object->count, r_GetDefaultMaterialPointer(), &object->transform);
            }
            else
            {
                r_DrawIndexed(object->start, object->count, object->vertex_offset, r_GetDefaultMaterialPointer(), &object->transform);
            }
        }
    }
    r_EndSubmission();
    begin_info.clear_framebuffer = 0;
    
    r_i_BeginSubmission(&begin_info);
    r_i_SetDrawState(&draw_state);
    for(int32_t quad_index = ED_GRID_X_WIDTH >> 1; quad_index >= 0; quad_index--)
    {
        r_i_DrawImmediate(grid_verts, 5, NULL, 0, NULL);
        grid_verts += 5;
    }
    for(int32_t quad_index = ED_GRID_Z_WIDTH >> 1; quad_index >= 0; quad_index--)
    {
        r_i_DrawImmediate(grid_verts, 5, NULL, 0, NULL);
        grid_verts += 5;
    }
    
    if(selections->cursor)
    {
        struct stack_list_t *objects = &data->target_data[data->current_selection_target].objects;
        struct list_t *selections = &data->target_data[data->current_selection_target].selections;
    
        if(data->current_selection_target == ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT)
        {
            draw_state.line_width = ed_selected_object_outline_draw_state.line_width;
            draw_state.texture = ed_selected_object_outline_draw_state.texture;
            draw_state.pipeline_state = ed_selected_object_outline_draw_state.pipeline_state;
            r_i_SetDrawState(&draw_state);
                        
            for(uint32_t object_index = 0; object_index < selections->cursor; object_index++)
            {
                struct ed_object_h handle = *(struct ed_object_h *)get_list_element(selections, object_index);
                struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
                struct r_i_vertex_t vertice;
                vec4_t color;
                if(object_index < selections->cursor - 1)
                {
                    color = vec4_t_c(1.0, 0.3, 0.0, 1.0);
                }
                else
                {
                    color = vec4_t_c(1.0, 0.8, 0.0, 1.0); 
                }
                
                if(object->vertex_count > ed_max_outline_verts)
                {
                    ed_max_outline_verts = object->vertex_count;
                    ed_outline_verts = mem_Realloc(ed_outline_verts, sizeof(struct r_i_vertex_t) * ed_max_outline_verts);
                }
                
                if(object->count > ed_max_outline_indices)
                {
                    ed_max_outline_indices = object->count;
                    ed_outline_indices = mem_Realloc(ed_outline_indices, sizeof(uint32_t) * ed_max_outline_indices);
                }
            
                
                switch(object->type)
                {
                    case ED_OBJECT_TYPE_BRUSH:
                    {
                        struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
                        for(uint32_t vertice_index = 0; vertice_index < brush->draw_vertice_count; vertice_index++)
                        {
                            ed_outline_verts[vertice_index].position = brush->draw_vertices[vertice_index].position;
                            ed_outline_verts[vertice_index].color = color;
                        }
                        
                        for(uint32_t indice_index = 0; indice_index < brush->draw_indice_count; indice_index++)
                        {
                            ed_outline_indices[indice_index] = brush->draw_indices[indice_index];
                        }
                        
                        r_i_DrawImmediate(ed_outline_verts, brush->draw_vertice_count, ed_outline_indices, brush->draw_indice_count, &object->transform);
                    }
                    break;
                }
            }
        }
        else
        {                               
            for(uint32_t object_index = 0; object_index < selections->cursor; object_index++)
            {
                struct ed_object_h handle = *(struct ed_object_h *)get_list_element(selections, object_index);
                struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
                
                draw_state.line_width = ed_selected_face_fill_draw_state.line_width;
                draw_state.texture = ed_selected_face_fill_draw_state.texture;
                draw_state.pipeline_state = ed_selected_face_fill_draw_state.pipeline_state;    
                r_i_SetDrawState(&draw_state);
                
                vec4_t color = vec4_t_c(0.0, 0.5, 1.0, 0.2);
                
                if(object->vertex_count > ed_max_outline_verts)
                {
                    ed_max_outline_verts = object->vertex_count;
                    ed_outline_verts = mem_Realloc(ed_outline_verts, sizeof(struct r_i_vertex_t) * ed_max_outline_verts);
                }
                
                if(object->count > ed_max_outline_indices)
                {
                    ed_max_outline_indices = object->count;
                    ed_outline_indices = mem_Realloc(ed_outline_indices, sizeof(uint32_t) * ed_max_outline_indices);
                }
                
                struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
                struct ed_brush_face_t *polygon = ed_GetBrushFacePointer(object->object.brush, object->object.face_index);
                for(uint32_t vertice_index = 0; vertice_index < polygon->indice_count; vertice_index++)
                {
                    ed_outline_verts[vertice_index].position = brush->draw_vertices[polygon->draw_vertices_start + vertice_index].position;
                    ed_outline_verts[vertice_index].color = color;
                }
                
                for(uint32_t indice_index = 0; indice_index < polygon->draw_indices_count; indice_index++)
                {
                    ed_outline_indices[indice_index] = brush->draw_indices[polygon->draw_indices_start + indice_index] - 
                        polygon->draw_vertices_start;
                }
                
                r_i_DrawImmediate(ed_outline_verts, polygon->indice_count, ed_outline_indices, polygon->draw_indices_count, &object->transform);
                
                color = vec4_t_c(0.0, 1.0, 0.2, 0.8);
                draw_state.line_width = ed_selected_face_ouline_draw_state.line_width;
                draw_state.texture = ed_selected_face_ouline_draw_state.texture;
                draw_state.pipeline_state = ed_selected_face_ouline_draw_state.pipeline_state;
                r_i_SetDrawState(&draw_state);
                
                for(uint32_t vertice_index = 0; vertice_index < polygon->indice_count; vertice_index++)
                {
                    ed_outline_verts[vertice_index].position = brush->draw_vertices[polygon->draw_vertices_start + vertice_index].position;
                    ed_outline_verts[vertice_index].color = color;                    
                    ed_outline_indices[vertice_index] = vertice_index;
                }
                
                ed_outline_indices[polygon->indice_count] = 0;                
                r_i_DrawImmediate(ed_outline_verts, polygon->indice_count, ed_outline_indices, polygon->indice_count + 1, &object->transform);
            }
        }
        
        
        draw_state.pipeline_state.input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        draw_state.pipeline_state.rasterizer_state.polygon_mode = VK_POLYGON_MODE_FILL;
        draw_state.pipeline_state.rasterizer_state.front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        draw_state.pipeline_state.rasterizer_state.cull_mode = VK_CULL_MODE_NONE;
        draw_state.pipeline_state.depth_state.test_enable = VK_FALSE;
        r_i_SetDrawState(&draw_state);
        
        mat4_t draw_transform;
        ed_w_ctx_ManipulatorDrawTransform(&draw_transform, &data->manipulator_state.transform, viewport);
        
        struct ed_manipulator_t *manipulator = ed_manipulators + ED_TRANSFORM_TYPE_TRANSLATION;
        for(uint32_t component_index = 0; component_index < manipulator->component_count; component_index++)
        {
            struct ed_manipulator_component_t *component = manipulator->components + component_index;
            r_i_DrawImmediate(ed_translation_manipulator_verts + component->vertex_start, ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT,
                              ed_translation_manipulator_indices, component->count, &draw_transform);
        } 
    }
    
    
    if(data->context_state == ED_WORLD_CONTEXT_STATE_CREATING_BRUSH)
    {
        uint32_t vert_index = 0;
        vec3_t first_corner = data->creating_brush_state.start_corner;
        vec3_t second_corner = data->creating_brush_state.current_corner;
        vec4_t color = vec4_t_c(0.0, 1.0, 0.0, 1.0);
        
        ed_creating_brush_verts[vert_index].position = vec4_t_c(first_corner.x, 0.0, first_corner.z, 1.0);
        ed_creating_brush_verts[vert_index].color = color;
        vert_index++;
        ed_creating_brush_verts[vert_index].position = vec4_t_c(first_corner.x, 0.0, second_corner.z, 1.0);
        ed_creating_brush_verts[vert_index].color = color;
        vert_index++;
        ed_creating_brush_verts[vert_index].position = vec4_t_c(second_corner.x, 0.0, second_corner.z, 1.0);
        ed_creating_brush_verts[vert_index].color = color;
        vert_index++;
        ed_creating_brush_verts[vert_index].position = vec4_t_c(second_corner.x, 0.0, first_corner.z, 1.0);
        ed_creating_brush_verts[vert_index].color = color;
        vert_index++;
        ed_creating_brush_verts[vert_index] = ed_creating_brush_verts[0];
        
        draw_state.line_width = ed_creating_brush_draw_state.line_width;
        draw_state.pipeline_state = ed_creating_brush_draw_state.pipeline_state;
        draw_state.texture = ed_creating_brush_draw_state.texture;
        
        r_i_SetDrawState(&draw_state);
        r_i_DrawImmediate(ed_creating_brush_verts, 5, NULL, 0, NULL);
    }
    
    r_i_EndSubmission();
}

/*
===========================================================================================================================================
===========================================================================================================================================
===========================================================================================================================================
*/

struct ed_object_h ed_w_ctx_CreateObject(struct stack_list_t *objects, mat4_t *transform, uint32_t type, uint32_t start, uint32_t count, uint32_t vertex_count, uint32_t vertex_offset, union ed_object_ref_t object_ref)
{
    struct ed_object_h handle;
    struct ed_object_t *object;
    
    handle.index = add_stack_list_element(objects, NULL);
    object = get_stack_list_element(objects, handle.index);
    
    object->type = type;
    object->start = start;
    object->count = count;
    object->vertex_count = vertex_count;
    object->vertex_offset = vertex_offset;
    object->transform = *transform;
    object->object = object_ref;
    
//    handle.index += ED_OBJECT_ID_LAST;
    
//    struct ed_world_context_data_t *data = (struct ed_world_context_data_t *)ed_contexts[ED_CONTEXT_WORLD].context_data;
//    add_list_element(&ed_objects, &handle);
    
    return handle;
}

void ed_w_ctx_DestroyObject(struct stack_list_t *objects, struct ed_object_h handle)
{
    
}

struct ed_object_h ed_w_ctx_CreateBrushObject(struct stack_list_t *objects, vec3_t *position, vec3_t *size)
{
    struct ed_object_h handle;
    struct bsh_brush_h brush_handle;
    struct ed_brush_t *brush;
    mat3_t orientation;
    mat4_t transform;
    mat4_t scale;
    
    mat3_t_identity(&orientation);
    brush_handle = bsh_CreateCubeBrush(position, &orientation, size);
    brush = ed_GetBrushPointer(brush_handle);
    
//    mat4_t_identity(&scale);
//    scale.rows[0].x = brush->scale.x;
//    scale.rows[1].y = brush->scale.y;
//    scale.rows[2].z = brush->scale.z;
    mat4_t_comp(&transform, &brush->orientation, &brush->position);
//    mat4_t_mul(&transform, &scale, &transform);
    
    return ed_w_ctx_CreateObject(objects, &transform, ED_OBJECT_TYPE_BRUSH, brush->start, brush->count, brush->draw_vertice_count, brush->vertex_offset, (union ed_object_ref_t){.brush = brush_handle});
}

struct ed_object_t *ed_w_ctx_GetObjectPointer(struct stack_list_t *objects, struct ed_object_h handle)
{
    struct ed_object_t *object = NULL;
    struct ed_world_context_data_t *data = ed_contexts[ED_CONTEXT_WORLD].context_data;
        
    object = get_stack_list_element(objects, handle.index);
    if(object && object->type == ED_OBJECT_TYPE_LAST)
    {
        object = NULL;
    }
    
    return object;
}


/*
===========================================================================================================================================
===========================================================================================================================================
===========================================================================================================================================
*/

void ed_w_ctx_SetState(struct ed_world_context_data_t *data, uint32_t state)
{
    data->context_state = state;
    data->just_changed_state = 1;
}

void ed_w_ctx_IdleState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    data->selection = ED_INVALID_OBJECT_HANDLE;
    data->manipulation_state.setup = 0;
    data->manipulation_state.axis_constraint = 0;
    data->creating_brush_state.setup = 0;

    if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_PRESSED)
    {
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_FLYING);
    }
    else if(in_GetMouseState(IN_MOUSE_BUTTON_LEFT) & IN_INPUT_STATE_PRESSED)
    {
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_LEFT_CLICK);
    }
    else if(in_GetMouseState(IN_MOUSE_BUTTON_RIGHT) & IN_INPUT_STATE_PRESSED)
    {
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_RIGHT_CLICK);
    }
}

void ed_w_ctx_LeftClickState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    /* object picking happens when the left button is released */
    uint32_t mouse_state = in_GetMouseState(IN_MOUSE_BUTTON_LEFT) & (IN_INPUT_STATE_JUST_PRESSED | IN_INPUT_STATE_PRESSED | IN_INPUT_STATE_JUST_RELEASED);

    if(mouse_state & IN_INPUT_STATE_JUST_PRESSED)
    {
        uint32_t picked_axis = 0;
        
        if(data->target_data[data->current_selection_target].selections.cursor)
        {
            picked_axis = ed_w_ctx_PickManipulator(&data->manipulator_state.transform, data->manipulator_state.transform_type, (struct ed_editor_viewport_t *)window);
        }
        
        if(picked_axis)
        {
            data->manipulation_state.axis_constraint = picked_axis;
            ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_MANIPULATING);
        }
        else
        {
            struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects;
            ed_object_handles.cursor = 0;
            
            for(uint32_t object_index = 0; object_index < objects->cursor; object_index++)
            {
                if(ed_w_ctx_GetObjectPointer(objects, (struct ed_object_h){object_index}))
                {
                    add_list_element(&ed_object_handles, &(struct ed_object_h){object_index});
                }
            }
            
            data->selection = ed_w_ctx_PickObject(objects, &ed_object_handles, (struct ed_editor_viewport_t *)window);
        }
    }
    else if(mouse_state & IN_INPUT_STATE_PRESSED)
    {
        if(in_GetDragState() & IN_DRAG_STATE_DRAGGING)
        {
            if(data->target_data[data->current_selection_target].selections.cursor)
            {
                ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_MANIPULATING);
            }
            else
            {
                ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_CREATING_BRUSH);
            }
        }
    }
    else if(mouse_state & IN_INPUT_STATE_JUST_RELEASED)
    {
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_PICK_OBJECT);
    }
}

void ed_w_ctx_RightClickState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    /* brush face picking happens when the right button is released */
    uint32_t mouse_state = in_GetMouseState(IN_MOUSE_BUTTON_RIGHT) & (IN_INPUT_STATE_JUST_PRESSED | IN_INPUT_STATE_PRESSED | IN_INPUT_STATE_JUST_RELEASED);
    if(mouse_state & IN_INPUT_STATE_JUST_PRESSED)
    {
        /* brush face picking happens when the right button is released */
        struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].objects;
        
        objects->cursor = 0;
        objects->free_stack_top = 0xffffffff;
        ed_object_handles.cursor = 0;
        
        for(uint32_t object_index = 0; object_index < data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects.cursor; object_index++)
        {
            struct ed_object_t *object = ed_w_ctx_GetObjectPointer(&data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects, (struct ed_object_h){object_index});
            if(object && object->type == ED_OBJECT_TYPE_BRUSH)
            {
                struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
                struct ed_brush_face_t *polygon = brush->polygons;
                uint32_t polygon_index = 0;
                while(polygon)
                {
                    struct ed_object_h face_handle = ed_w_ctx_CreateObject(objects, 
                                        &object->transform, ED_OBJECT_TYPE_FACE, 0, 0, 0, 0, 
                                            (union ed_object_ref_t){.brush = object->object.brush, .face_index = polygon_index});
                    struct ed_object_t *face_object = ed_w_ctx_GetObjectPointer(objects, face_handle);
                    face_object->start = polygon->draw_indices_start + brush->start;
                    face_object->count = polygon->draw_indices_count;
                    face_object->vertex_count = polygon->indice_count;
                    face_object->vertex_offset = object->vertex_offset;
                    add_list_element(&ed_object_handles, &face_handle);
                    polygon = polygon->next;
                    polygon_index++;
                }
            }
        }
        
        data->selection = ed_w_ctx_PickObject(objects, &ed_object_handles, (struct ed_editor_viewport_t *)window);
    }
    else if(mouse_state & IN_INPUT_STATE_JUST_RELEASED)
    {
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_PICK_BRUSH_FACE);
    }
}

void ed_w_ctx_PickObjectState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects;
    struct list_t *selections = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].selections;
    
    ed_object_handles.cursor = 0;
    add_list_element(&ed_object_handles, &data->selection);
    struct ed_object_h selection = ed_w_ctx_PickObject(objects, &ed_object_handles, (struct ed_editor_viewport_t *)window);
    
    if(selection.index == data->selection.index)
    {
        ed_w_ctx_AppendObjectSelection(selections, selection, in_GetKeyState(SDL_SCANCODE_LSHIFT) & IN_INPUT_STATE_PRESSED);
        data->current_selection_target = ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT;
        ed_w_ctx_UpdateManipulatorTransform(data);
    }
    
    ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_IDLE);
}

void ed_w_ctx_PickBrushFaceState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].objects;
    struct list_t *selections = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].selections;
    
    ed_object_handles.cursor = 0;
    add_list_element(&ed_object_handles, &data->selection);
    struct ed_object_h selection = ed_w_ctx_PickObject(objects, &ed_object_handles, (struct ed_editor_viewport_t *)window);
    
    if(selection.index == data->selection.index)
    {
        ed_w_ctx_AppendObjectSelection(selections, selection, in_GetKeyState(SDL_SCANCODE_LSHIFT) & IN_INPUT_STATE_PRESSED);
        data->current_selection_target = ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH;
        ed_w_ctx_UpdateManipulatorTransform(data);
    }
    
    ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_IDLE);
}

void ed_w_ctx_ManipulatingState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    uint32_t mouse_state = in_GetMouseState(IN_MOUSE_BUTTON_LEFT) & (IN_INPUT_STATE_PRESSED | IN_INPUT_STATE_JUST_PRESSED);
    
    if(mouse_state & IN_INPUT_STATE_PRESSED)
    {
        struct ed_editor_viewport_t *viewport = (struct ed_editor_viewport_t *)window;
        struct stack_list_t *objects = &data->target_data[data->current_selection_target].objects;
        struct list_t *selections = &data->target_data[data->current_selection_target].selections;
        
        vec3_t manipulator_pos = vec3_t_c_vec4_t(&data->manipulator_state.transform.rows[3]);
        vec3_t plane_point;
        vec3_t camera_pos = vec3_t_c_vec4_t(&viewport->view_matrix.rows[3]);
        vec3_t camera_plane_vec;
        vec3_t drag_delta;
        
        float fovy = 0.68;
        float aspect = (float)viewport->width / (float)viewport->height;
        float top = tanf(fovy) * 0.1;
        float right = top * aspect;
        
        vec3_t right_vec = vec3_t_c_vec4_t(&viewport->view_matrix.rows[0]);
        vec3_t up_vec = vec3_t_c_vec4_t(&viewport->view_matrix.rows[1]);
        vec3_t forward_vec = vec3_t_c_vec4_t(&viewport->view_matrix.rows[2]);
        vec3_t mouse_vec = vec3_t_c(0.0, 0.0, 0.0);

        float mouse_x = ((float)viewport->mouse_x / (float)viewport->width) * 2.0 - 1.0;
        float mouse_y = 1.0 - ((float)viewport->mouse_y / (float)viewport->height) * 2.0;
        
        vec3_t_mul(&right_vec, &right_vec, right * mouse_x);
        vec3_t_mul(&up_vec, &up_vec, top * mouse_y);
        vec3_t_mul(&forward_vec, &forward_vec, -0.1);
        
        vec3_t_add(&mouse_vec, &right_vec, &up_vec);
        vec3_t_add(&mouse_vec, &mouse_vec, &forward_vec);
        vec3_t_normalize(&mouse_vec, &mouse_vec);
        vec3_t_sub(&camera_plane_vec, &manipulator_pos, &camera_pos);
        
        if(!data->manipulation_state.setup)
        {
            vec3_t plane_normal;
            if(data->manipulation_state.axis_constraint)
            {
                data->manipulation_state.axis_clamp = ed_manipulator_plane_normal[data->manipulation_state.axis_constraint];
                plane_normal = ed_manipulator_plane_normal[data->manipulation_state.axis_constraint];
                
                float dist = vec3_t_dot(&camera_plane_vec, &plane_normal);
                vec3_t_fmadd(&plane_point, &manipulator_pos, &plane_normal, -dist);
                vec3_t_sub(&plane_normal, &camera_pos, &plane_point);
                vec3_t_normalize(&data->manipulation_state.pick_normal, &plane_normal);
            }
            else
            {
                data->manipulation_state.axis_clamp = vec3_t_c(1.0, 1.0, 1.0);
                data->manipulation_state.pick_normal = forward_vec;
            }
        }
        
        float d = vec3_t_dot(&mouse_vec, &data->manipulation_state.pick_normal);
        float t = 0.0;
        
        if(d)
        {
            t = vec3_t_dot(&camera_plane_vec, &data->manipulation_state.pick_normal) / d;                
        }
        
        vec3_t_fmadd(&plane_point, &camera_pos, &mouse_vec, t);
        
        if(!data->manipulation_state.setup)
        {
            vec3_t_sub(&data->manipulation_state.pick_offset, &manipulator_pos, &plane_point);
        }
        
        vec3_t_add(&plane_point, &plane_point, &data->manipulation_state.pick_offset);
        vec3_t_sub(&drag_delta, &plane_point, &manipulator_pos);
        
        if(data->manipulation_state.linear_threshold)
        {
            drag_delta.x -= fmodf(drag_delta.x, data->manipulation_state.linear_threshold);
            drag_delta.y -= fmodf(drag_delta.y, data->manipulation_state.linear_threshold);
            drag_delta.z -= fmodf(drag_delta.z, data->manipulation_state.linear_threshold);
        }
        
        drag_delta.x *= data->manipulation_state.axis_clamp.x;
        drag_delta.y *= data->manipulation_state.axis_clamp.y;
        drag_delta.z *= data->manipulation_state.axis_clamp.z;
                
        mat4_t transform;
        mat4_t_identity(&transform);
        
        data->manipulation_state.setup = 1;
        
        transform.rows[3] = vec4_t_c(drag_delta.x, drag_delta.y, drag_delta.z, 1.0);
        
        ed_w_ctx_ApplyTransform(&transform, objects, selections, ED_TRANSFORM_TYPE_TRANSLATION, ED_TRANSFORM_MODE_WORLD);
        ed_w_ctx_UpdateManipulatorTransform(data);
    }
    else
    {
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_IDLE);
    }
}

void ed_w_ctx_CreatingBrushState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    if(in_GetMouseState(IN_MOUSE_BUTTON_LEFT) & IN_INPUT_STATE_PRESSED)
    {
        struct ed_editor_viewport_t *viewport = (struct ed_editor_viewport_t *)window;
        vec3_t right_vec = vec3_t_c_vec4_t(&viewport->view_matrix.rows[0]);
        vec3_t up_vec = vec3_t_c_vec4_t(&viewport->view_matrix.rows[1]);
        vec3_t forward_vec = vec3_t_c_vec4_t(&viewport->view_matrix.rows[2]);
        
        vec3_t camera_pos = vec3_t_c_vec4_t(&viewport->view_matrix.rows[3]);
        vec3_t plane_point = camera_pos;
        plane_point.y = 0.0;
        
        vec3_t plane_normal = vec3_t_c(0.0, 1.0, 0.0);
        
        float fovy = 0.68;
        float aspect = (float)viewport->width / (float)viewport->height;
        float top = tanf(fovy) * 0.1;
        float right = top * aspect;
        
        float mouse_x = ((float)viewport->mouse_x / (float)viewport->width) * 2.0 - 1.0;
        float mouse_y = 1.0 - ((float)viewport->mouse_y / (float)viewport->height) * 2.0;
        
        vec3_t_mul(&right_vec, &right_vec, right * mouse_x);
        vec3_t_mul(&up_vec, &up_vec, top * mouse_y);
        vec3_t_mul(&forward_vec, &forward_vec, -0.1);
        
        vec3_t mouse_vec = vec3_t_c(0.0, 0.0, 0.0);
        vec3_t_add(&mouse_vec, &right_vec, &up_vec);
        vec3_t_add(&mouse_vec, &mouse_vec, &forward_vec);
        vec3_t_normalize(&mouse_vec, &mouse_vec);
        
        float d = vec3_t_dot(&mouse_vec, &plane_normal);
        float t = 0.0;
        
        if(d)
        {
            t = -camera_pos.y / d;                
        }
        
        vec3_t_fmadd(&plane_point, &camera_pos, &mouse_vec, t);
        
        if(data->manipulation_state.linear_threshold)
        {
            if(data->creating_brush_state.setup)
            {
                vec3_t direction;
                vec3_t_sub(&direction, &plane_point, &data->creating_brush_state.start_corner);
                
                plane_point = data->creating_brush_state.start_corner;
                
                if(direction.x < 0.0)
                {
                    plane_point.x += floorf(direction.x / data->manipulation_state.linear_threshold);
                }
                else
                {
                    plane_point.x += ceilf(direction.x / data->manipulation_state.linear_threshold);
                }
                
                
                if(direction.z < 0.0)
                {
                    plane_point.z += floorf(direction.z / data->manipulation_state.linear_threshold);
                }
                else
                {
                    plane_point.z += ceilf(direction.z / data->manipulation_state.linear_threshold);
                }
            }
            
            plane_point.x -= fmodf(plane_point.x, data->manipulation_state.linear_threshold);
            plane_point.z -= fmodf(plane_point.z, data->manipulation_state.linear_threshold);
        }
        
        if(!data->creating_brush_state.setup)
        {
            data->creating_brush_state.start_corner = plane_point;
        }
        
        data->creating_brush_state.current_corner = plane_point;
        data->creating_brush_state.setup = 1;
    }
    else
    {
        if(in_GetMouseState(IN_MOUSE_BUTTON_LEFT) & IN_INPUT_STATE_JUST_RELEASED)
        {
            vec3_t brush_position;
            vec3_t brush_size;
            
            vec3_t_sub(&brush_size, &data->creating_brush_state.start_corner, &data->creating_brush_state.current_corner);
            vec3_t_fabs(&brush_size, &brush_size);
            brush_size.y = 1.0;
            
            if(brush_size.x > 0.0 || brush_size.z > 0.0)
            {
                vec3_t_add(&brush_position, &data->creating_brush_state.start_corner, &data->creating_brush_state.current_corner);
                vec3_t_mul(&brush_position, &brush_position, 0.5);
                brush_position.y += brush_size.y * 0.5;
                ed_w_ctx_CreateBrushObject(&data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects, &brush_position, &brush_size);
            }
        }
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_IDLE);
    }
}

void ed_w_ctx_FlyingState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_JUST_PRESSED)
    {
        in_RelativeMode(1);
    }
    
    if(in_GetMouseState(IN_MOUSE_BUTTON_MIDDLE) & IN_INPUT_STATE_PRESSED)
    {
        ed_FlyView((struct ed_editor_viewport_t *)window);
    }
    else
    {
        in_RelativeMode(0);
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_IDLE);
    }
}

/*
===========================================================================================================================================
===========================================================================================================================================
===========================================================================================================================================
*/

void ed_w_ctx_UpdateManipulatorTransform(struct ed_world_context_data_t *data)
{
    data->manipulator_state.transform.rows[3] = vec4_t_c(0.0, 0.0, 0.0, 0.0); 
    struct stack_list_t *objects = &data->target_data[data->current_selection_target].objects;
    struct list_t *selections = &data->target_data[data->current_selection_target].selections;

    switch(data->current_selection_target)
    {
        case ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT:
            for(uint32_t object_index = 0; object_index < selections->cursor; object_index++)
            {
                struct ed_object_h handle = *(struct ed_object_h *)get_list_element(selections, object_index);
                struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
                vec4_t_add(&data->manipulator_state.transform.rows[3], &data->manipulator_state.transform.rows[3], &object->transform.rows[3]);
            }
        break;
        
        case ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH:
            for(uint32_t object_index = 0; object_index < selections->cursor; object_index++)
            {
                struct ed_object_h handle = *(struct ed_object_h *)get_list_element(selections, object_index);
                struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
                struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
                struct ed_brush_face_t *face = ed_GetBrushFacePointer(object->object.brush, object->object.face_index);
                
                vec3_t face_position = vec3_t_c(0.0, 0.0, 0.0);
                for(uint32_t vert_index = 0; vert_index < face->indice_count; vert_index++)
                {
                    vec3_t_add(&face_position, &face_position, brush->vertices + face->indices[vert_index]);
                }
                face_position.x /= (float)face->indice_count;
                face_position.y /= (float)face->indice_count;
                face_position.z /= (float)face->indice_count;  
                
                vec4_t_add(&data->manipulator_state.transform.rows[3], &data->manipulator_state.transform.rows[3], &object->transform.rows[3]);
                data->manipulator_state.transform.rows[3].x += face_position.x;
                data->manipulator_state.transform.rows[3].y += face_position.y;
                data->manipulator_state.transform.rows[3].z += face_position.z;
            }
        break;
    }
    
    data->manipulator_state.transform.rows[3].x /= (float)selections->cursor;
    data->manipulator_state.transform.rows[3].y /= (float)selections->cursor;
    data->manipulator_state.transform.rows[3].z /= (float)selections->cursor;
    data->manipulator_state.transform.rows[3].w = 1.0;
}

void ed_w_ctx_ManipulatorDrawTransform(mat4_t *draw_transform, mat4_t *manipulator_transform, struct ed_editor_viewport_t *viewport)
{
    vec3_t view_pos = vec3_t_c_vec4_t(&viewport->view_matrix.rows[3]);
    vec3_t manipulator_pos = vec3_t_c_vec4_t(&manipulator_transform->rows[3]);
    vec3_t view_manipulator_vec;
    
    vec3_t_sub(&view_manipulator_vec, &manipulator_pos, &view_pos);
    
    float scale = vec3_t_length(&view_manipulator_vec) * 0.1;
    *draw_transform = *manipulator_transform;
    draw_transform->rows[0].x *= scale;
    draw_transform->rows[1].y *= scale;
    draw_transform->rows[2].z *= scale;
}

void ed_w_ctx_ObjectTransform(struct stack_list_t *objects, struct ed_object_h handle, mat4_t *transform)
{
    struct ed_object_t *object;
    mat4_t temp;
    object = ed_w_ctx_GetObjectPointer(objects, handle);
    if(object)
    {
        switch(object->type)
        {
            case ED_OBJECT_TYPE_BRUSH:
            {
                struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
                
            }
            break;
        }
    }
}

void ed_w_ctx_ApplyTransform(mat4_t *transform, struct stack_list_t *objects, struct list_t *selections, uint32_t transform_type, uint32_t transform_mode)
{
    switch(transform_type)
    {
        case ED_TRANSFORM_TYPE_TRANSLATION:
        {
            vec3_t translation;
            translation.x = transform->rows[3].x;
            translation.y = transform->rows[3].y;
            translation.z = transform->rows[3].z;
            
            for(uint32_t selection_index = 0; selection_index < selections->cursor; selection_index++)
            {
                struct ed_object_h handle = *(struct ed_object_h *)get_list_element(selections, selection_index);
                struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
                
                switch(object->type)
                {
                    case ED_OBJECT_TYPE_BRUSH:
                    {
                        bsh_TranslateBrush(object->object.brush, &translation);
                        struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
                        mat4_t_identity(&object->transform);
                        mat4_t_comp(&object->transform, &brush->orientation, &brush->position);
                    }
                    break;
                    
                    case ED_OBJECT_TYPE_FACE:
                    {
                        struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
                        struct ed_brush_face_t *face = ed_GetBrushFacePointer(object->object.brush, object->object.face_index);
                        
                        for(uint32_t vert_index = 0; vert_index < face->indice_count; vert_index++)
                        {
                            vec3_t *vert = brush->vertices + face->indices[vert_index];
                            vec3_t_add(vert, vert, &translation);
                        }
                        
                        bsh_UpdateDrawTriangles(object->object.brush);
                    }
                    break;
                }
            }
        }
        break;
    }
}


uint32_t ed_w_ctx_PickManipulator(mat4_t *manipulator_transform, uint32_t transform_type, struct ed_editor_viewport_t *viewport)
{
    union r_command_buffer_h command_buffer;
    struct r_render_pass_t *render_pass;
    struct r_pipeline_t *pipeline;
    mat4_t draw_matrix;
    mat4_t model_view_projection_matrix;
    mat4_t view_projection_matrix;
    struct r_chunk_t *vertex_chunk;
    struct r_chunk_t *index_chunk;
    struct ed_manipulator_t *manipulator;
    
    struct
    {
        mat4_t model_view_projection_matrix;
        uint32_t object_index;
    }push_constant;
      
    mat4_t_mul(&view_projection_matrix, &viewport->inv_view_matrix, &viewport->projection_matrix);
    render_pass = r_GetRenderPassPointer(ed_picking_render_pass);
    pipeline = r_GetPipelinePointer(render_pass->pipelines[0]);
    command_buffer = ed_BeginPicking(viewport);
    
    vertex_chunk = r_GetChunkPointer(ed_translation_manipulator_vert_chunk);
    index_chunk = r_GetChunkPointer(ed_translation_manipulator_index_chunk);
    
    ed_w_ctx_ManipulatorDrawTransform(&push_constant.model_view_projection_matrix, manipulator_transform, viewport);
    mat4_t_mul(&push_constant.model_view_projection_matrix, &push_constant.model_view_projection_matrix, &view_projection_matrix);
    
    manipulator = ed_manipulators + ED_TRANSFORM_TYPE_TRANSLATION;
    uint32_t index_start = index_chunk->start / sizeof(uint32_t);
    uint32_t vertex_start = vertex_chunk->start / sizeof(struct r_i_vertex_t);
    
    for(uint32_t component_index = 0; component_index < manipulator->component_count; component_index++)
    {
        struct ed_manipulator_component_t *component = manipulator->components + component_index;
        push_constant.object_index = component->id;
        r_vkCmdPushConstants(command_buffer, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constant), &push_constant);
        r_vkCmdDrawIndexed(command_buffer, component->count, 1, index_start, vertex_start + component->vertex_start, 0);
    }
    
    return ed_EndPicking(command_buffer, viewport);
}

struct ed_object_h ed_w_ctx_PickObject(struct stack_list_t *objects, struct list_t *handles, struct ed_editor_viewport_t *viewport)
{
    union r_command_buffer_h command_buffer;
    struct r_render_pass_t *render_pass;
    struct r_pipeline_t *pipeline;
    mat4_t model_view_projection_matrix;
    mat4_t view_projection_matrix;
    
    struct
    {
        mat4_t model_view_projection_matrix;
        uint32_t object_index;
    }push_constant;

    mat4_t_mul(&view_projection_matrix, &viewport->inv_view_matrix, &viewport->projection_matrix);
    render_pass = r_GetRenderPassPointer(ed_picking_render_pass);
    pipeline = r_GetPipelinePointer(render_pass->pipelines[0]);
    
    command_buffer = ed_BeginPicking(viewport);

    for(uint32_t object_index = 0; object_index < handles->cursor; object_index++)
    {
        struct ed_object_h handle = *(struct ed_object_h *)get_list_element(handles, object_index);
        struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
        
        if(object && object->type != ED_OBJECT_TYPE_LAST)
        {
            mat4_t_identity(&push_constant.model_view_projection_matrix);
            mat4_t_mul(&push_constant.model_view_projection_matrix, &object->transform, &view_projection_matrix);
            push_constant.object_index = handle.index + 1;
            r_vkCmdPushConstants(command_buffer, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constant), &push_constant);
            if(object->vertex_offset == 0xffffffff)
            {
                r_vkCmdDraw(command_buffer, object->count, 1, object->start, 0);
            }
            else
            {
                r_vkCmdDrawIndexed(command_buffer, object->count, 1, object->start, object->vertex_offset, 0);
            }
        }
    }
    
    return (struct ed_object_h ){ed_EndPicking(command_buffer, viewport) - 1};
}

void ed_w_ctx_AppendObjectSelection(struct list_t *selections, struct ed_object_h selection, uint32_t shift_pressed)
{
    if(selection.index < 0xffffffff)
    {
        uint32_t prev_index = 0xffffffff;

        for(uint32_t object_index = 0; object_index < selections->cursor; object_index++)
        {
            struct ed_object_h *handle = get_list_element(selections, object_index);
            if(handle->index == selection.index)
            {
                prev_index = object_index;
                break;
            }
        }

        /* the selection behaviour goes as follows: 
                    
        If shift isn't being held down:
        
        if the object wasn't previously selected, the list will be cleared, and then this object 
        will be added to the list. If the object was previously selected, the list will be cleared 
        and (obviously) the object won't be added back in, which effectivelly unselects it.
        

        
        If shift is being held down:
        
        If it wasn't selected, it'll be appended at the end of the list, and will become the "main" 
        selection. If it was selected, it'll be moved to the end of the list, and will become the 
        "main" selection. If it's the "main" selection, it'll be unselected. 
        
        
        
        The last object in the list is the "main" selection, and only the "main" selection will be
        unselected upon being clicked on. The behavior is the same whether shift is being held 
        down or not. If it isn't being held down, the only object in the list will also be the last
        object in the list, and therefore it's the "main" selection. If shift is being held down,
        the object (selected or not) will be put in the end of the list, therefore becoming the "main"
        selection.
        
        This is similar to how blender handles selections
        
        */
        
        if(shift_pressed)
        {
            if(prev_index != 0xffffffff)
            {
                /* object is in the selection list */
                
                if(prev_index == selections->cursor - 1)
                {
                    /* object is the last in the list ( "main" selection), so it gets
                    deselected */
                    selections->cursor--;
                }
                else
                {
                    /* object is not the "main" selection, so it'll get moved to the end of the list. In practice, the 
                    object gets dropped from the list and readded at the end */
                    for(uint32_t object_index = prev_index; object_index < selections->cursor - 1; object_index++)
                    {
                        /* move everything back to fill the now vacant spot of the object that was
                        dropped to become... THE "main" selection... */
                        memcpy(get_list_element(selections, object_index), get_list_element(selections, object_index + 1), sizeof(struct ed_object_h));
                    }
                    
                    /* "pop" off the last object */
                    selections->cursor--;
                    
                    /* set prev_index so the code to add the object to the list gets
                    run */
                    prev_index = 0xffffffff;
                }
            }
        }
        else
        {
            /* shift not being held down */
            
            if(selections->cursor > 1)
            {
                /* the list has more than one object (previously selected by holding shift). In this
                case, the clicked object will become the only one in the list, whether it was previously
                the "main" selection or not. */
                
                /* set prev_index so the code to add the object to the list gets
                run */
                prev_index = 0xffffffff;
            }
            
            /* clear the list */
            selections->cursor = 0;
        }
        
        if(prev_index == 0xffffffff)
        {
            add_list_element(selections, &selection);
        }
    }
    
}

void ed_w_ctx_AppendBrushSelection(struct list_t *selections, struct ed_object_h selection, uint32_t shift_pressed)
{
    if(selection.index < 0xffffffff)
    {
        uint32_t prev_index = 0xffffffff;

        for(uint32_t object_index = 0; object_index < selections->cursor; object_index++)
        {
            struct ed_object_h *handle = get_list_element(selections, object_index);
            if(handle->index == selection.index)
            {
                prev_index = object_index;
                break;
            }
        }

        /* the selection behaviour for brush elements is simpler, and goes as follows: 
                    
        If shift isn't being held down:
        
        if the object wasn't previously selected, the list will be cleared, and then this object 
        will be added to the list. If the object was previously selected, the list will be cleared 
        and (obviously) the object won't be added back in, which effectivelly unselects it.
        

        
        If shift is being held down:
        
        If the object it wasn't selected, it'll be added to the list. If it was selected, it'll be 
        removed from the list, effectively unselecting.
        
        */
        
        if(shift_pressed)
        {
            if(prev_index != 0xffffffff)
            {
                /* object is in the selection list */                
                remove_list_element(selections, prev_index);
                return;
            }
        }
        else
        {
            /* shift not being held down */
            
            if(selections->cursor > 1)
            {
                /* the list has more than one object (previously selected by holding shift). In this
                case, the clicked object will become the only one in the list. */
                
                prev_index = 0xffffffff;
            }
            
            /* clear the list */
            selections->cursor = 0;
        }
        
        if(prev_index == 0xffffffff)
        {
            add_list_element(selections, &selection);
        }
    }
    
}
