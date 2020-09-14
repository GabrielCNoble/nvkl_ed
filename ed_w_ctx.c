#include "ed_w_ctx.h"
#include "neighbor/in.h"
#include "neighbor/ui.h"
#include "neighbor/r_draw.h"
#include "neighbor/lib/dstuff/ds_obj.h"
#include "neighbor/lib/dstuff/ds_mem.h"

extern struct ed_context_t ed_contexts[ED_CONTEXT_LAST];
extern struct r_render_pass_handle_t ed_picking_render_pass;
struct list_t ed_object_handles;
struct list_t ed_transform_brush_face_indices;

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
uint32_t ed_translation_manipulator_vert_count;
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
#define ED_TRANSLATION_MANIPULATOR_SHAFT_THICKNESS 0.05

uint32_t ed_rotation_manipulator_vert_count;
struct r_i_vertex_t *ed_rotation_manipulator_verts;
struct r_chunk_h ed_rotation_manipulator_vert_chunk;
struct r_chunk_h ed_rotation_manipulator_index_chunk;
#define ED_ROTATION_MANIPULATOR_OUTTER_DIAMETER 1.0
#define ED_ROTATION_MANIPULATOR_INNER_DIAMETER 0.8
#define ED_ROTATION_MANIPULATOR_RING_VERT_COUNT 32
#define ED_ROTATION_MANIPULATOR_RING_INDICE_COUNT (ED_ROTATION_MANIPULATOR_RING_VERT_COUNT*6)

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
//    [ED_TRANSFORM_TYPE_TRANSLATION] = {
//        .component_count = 3,
//        .components = (struct ed_manipulator_component_t []){
//            {
//                .vertex_start = 0, 
//                .start = 0, 
//                .count = ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT, 
//                .id = ED_MANIPULATOR_AXIS_ID_X_AXIS,
//            },
//            
//            {
//                .vertex_start = ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT, 
//                .start = 0, 
//                .count = ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT, 
//                .id = ED_MANIPULATOR_AXIS_ID_Y_AXIS,
//            },
//            
//            {
//                .vertex_start = ED_TRANSLATION_MANIPULATOR_AXIS_VERT_COUNT * 2, 
//                .start = 0, 
//                .count = ED_TRANSLATION_MANIPULATOR_SHAFT_INDICE_COUNT, 
//                .id = ED_MANIPULATOR_AXIS_ID_Z_AXIS,
//            },
//        }
//    },
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
    [ED_WORLD_CONTEXT_STATE_EXTRUDING_BRUSH_FACES] = ed_w_ctx_ExtrudingBrushFacesState,
    [ED_WORLD_CONTEXT_STATE_COPYING_SELECTIONS] = ed_w_ctx_CopyingSelectionsState,
    [ED_WORLD_CONTEXT_STATE_DELETING_SELECTIONS] = ed_w_ctx_DeletingSelectionsState,
    [ED_WORLD_CONTEXT_STATE_FLYING] = ed_w_ctx_FlyingState,
};

float ed_linear_thresholds[] = {
    1.0, 
    0.5,
    0.25,
    0.2,
    0.1,
    0.05,
    0.01,
    0.0,
};

float ed_angular_thresholds[] = {
    180.0,
    90.0,
    60.0,
    45.0,
    30.0,
    10.0,
    1.0,
    0.0
};

void ed_w_ctx_Init()
{
//    ed_objects = create_stack_list(sizeof(struct ed_object_t), 512);
//    ed_temp_objects = create_stack_list(sizeof(struct ed_object_t), 512);
    ed_object_handles = create_list(sizeof(struct ed_object_h), 512);
    ed_transform_brush_face_indices = create_list(sizeof(uint32_t), 512);
    
    ed_contexts[ED_CONTEXT_WORLD].input_function = ed_w_ctx_Input;
    ed_contexts[ED_CONTEXT_WORLD].update_function = ed_w_ctx_Update;
    ed_contexts[ED_CONTEXT_WORLD].layout_function = ed_w_ctx_Layout;
    ed_contexts[ED_CONTEXT_WORLD].reset_function = ed_w_ctx_Reset;
    ed_contexts[ED_CONTEXT_WORLD].context_data = mem_Calloc(1, sizeof(struct ed_world_context_data_t));
    ed_contexts[ED_CONTEXT_WORLD].update_frame = 0xffffffff;
    ed_contexts[ED_CONTEXT_WORLD].hold_focus = 0;
    
    struct ed_world_context_data_t *data;
    struct ed_world_context_sub_context_t *sub_context;
    
    data = ed_contexts[ED_CONTEXT_WORLD].context_data;
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].selections = create_list(sizeof(struct ed_object_h), 512);
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects = create_stack_list(sizeof(struct ed_object_t), 512);
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].selections = create_list(sizeof(struct ed_object_h), 512);
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].objects = create_stack_list(sizeof(struct ed_object_t), 512);
    data->manipulation_state.linear_threshold = 1.0;
    
    data->current_selection_target = ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT;
    mat4_t_identity(&data->manipulator_state.transform);
    
    bsh_Init();
    
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
    
    
    
    struct geometry_data_t manipulator_data;
    struct ed_manipulator_component_t *components;
    struct r_chunk_h chunk;
    
    
    /* translation manipulator */
    load_wavefront("data/tmanip.obj", &manipulator_data);
    
    ed_translation_manipulator_vert_count = manipulator_data.vertices.cursor * 3;
    ed_manipulators[ED_TRANSFORM_TYPE_TRANSLATION].components = mem_Calloc(3, sizeof(struct ed_manipulator_component_t));
    ed_manipulators[ED_TRANSFORM_TYPE_TRANSLATION].component_count = 3;
    ed_manipulators[ED_TRANSFORM_TYPE_TRANSLATION].vertices = mem_Calloc(ed_translation_manipulator_vert_count, sizeof(struct r_i_vertex_t));;
    components = ed_manipulators[ED_TRANSFORM_TYPE_TRANSLATION].components;
    
    verts = ed_manipulators[ED_TRANSFORM_TYPE_TRANSLATION].vertices;
    components[0].id = ED_MANIPULATOR_AXIS_ID_X_AXIS;
    components[0].start = 0;
    components[0].count = manipulator_data.vertices.cursor;
    for(uint32_t vert_index = 0; vert_index < manipulator_data.vertices.cursor; vert_index++)
    {
        vec3_t *vert = get_list_element(&manipulator_data.vertices, vert_index);
        verts[vert_index].position = vec4_t_c(vert->x, vert->y, vert->z, 1.0);
        verts[vert_index].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    }
    verts += manipulator_data.vertices.cursor;
    components[1].id = ED_MANIPULATOR_AXIS_ID_Y_AXIS;
    components[1].start = manipulator_data.vertices.cursor;
    components[1].count = manipulator_data.vertices.cursor;
    for(uint32_t vert_index = 0; vert_index < manipulator_data.vertices.cursor; vert_index++)
    {
        vec3_t *vert = get_list_element(&manipulator_data.vertices, vert_index);
        verts[vert_index].position = vec4_t_c(vert->y, vert->x, vert->z, 1.0);
        verts[vert_index].color = vec4_t_c(0.0, 1.0, 0.0, 1.0);
    }
    verts += manipulator_data.vertices.cursor;
    components[2].id = ED_MANIPULATOR_AXIS_ID_Z_AXIS;
    components[2].start = manipulator_data.vertices.cursor * 2;
    components[2].count = manipulator_data.vertices.cursor;
    for(uint32_t vert_index = 0; vert_index < manipulator_data.vertices.cursor; vert_index++)
    {
        vec3_t *vert = get_list_element(&manipulator_data.vertices, vert_index);
        verts[vert_index].position = vec4_t_c(vert->z, vert->y, vert->x, 1.0);
        verts[vert_index].color = vec4_t_c(0.0, 0.0, 1.0, 1.0);
    }
    
    chunk = r_AllocChunk(r_vertex_heap, sizeof(struct r_i_vertex_t) * ed_translation_manipulator_vert_count, sizeof(struct r_i_vertex_t));
    r_FillBufferChunk(chunk, ed_manipulators[ED_TRANSFORM_TYPE_TRANSLATION].vertices, sizeof(struct r_i_vertex_t) * ed_translation_manipulator_vert_count, 0);
    ed_manipulators[ED_TRANSFORM_TYPE_TRANSLATION].chunk = chunk;
    
    destroy_list(&manipulator_data.vertices);
    destroy_list(&manipulator_data.normals);
    destroy_list(&manipulator_data.tangents);
    destroy_list(&manipulator_data.tex_coords);
    destroy_list(&manipulator_data.batches);
    
    /*
    =======================================================================================================
    =======================================================================================================
    =======================================================================================================
    */
    
    /* rotation manipulator */
    load_wavefront("data/rmanip.obj", &manipulator_data);
    ed_rotation_manipulator_vert_count = manipulator_data.vertices.cursor * 3;
    ed_manipulators[ED_TRANSFORM_TYPE_ROTATION].vertices = mem_Calloc(manipulator_data.vertices.cursor * 3, sizeof(struct r_i_vertex_t));
    ed_manipulators[ED_TRANSFORM_TYPE_ROTATION].components = mem_Calloc(3, sizeof(struct ed_manipulator_component_t));
    ed_manipulators[ED_TRANSFORM_TYPE_ROTATION].component_count = 3;
    components = ed_manipulators[ED_TRANSFORM_TYPE_ROTATION].components;
    
    verts = ed_manipulators[ED_TRANSFORM_TYPE_ROTATION].vertices;
    components[0].id = ED_MANIPULATOR_AXIS_ID_X_AXIS;
    components[0].count = manipulator_data.vertices.cursor;
    components[0].start = 0;
    for(uint32_t vert_index = 0; vert_index < manipulator_data.vertices.cursor; vert_index++)
    {
        vec3_t *vert = get_list_element(&manipulator_data.vertices, vert_index);
        verts[vert_index].position = vec4_t_c(vert->x, vert->y, vert->z, 1.0);
        verts[vert_index].color = vec4_t_c(1.0, 0.0, 0.0, 1.0);
    }
    verts += manipulator_data.vertices.cursor;
    components[1].id = ED_MANIPULATOR_AXIS_ID_Y_AXIS;
    components[1].count = manipulator_data.vertices.cursor;
    components[1].start = manipulator_data.vertices.cursor;
    for(uint32_t vert_index = 0; vert_index < manipulator_data.vertices.cursor; vert_index++)
    {
        vec3_t *vert = get_list_element(&manipulator_data.vertices, vert_index);
        verts[vert_index].position = vec4_t_c(vert->y, vert->x, vert->z, 1.0);
        verts[vert_index].color = vec4_t_c(0.0, 1.0, 0.0, 1.0);
    }
    verts += manipulator_data.vertices.cursor;
    components[2].id = ED_MANIPULATOR_AXIS_ID_Z_AXIS;
    components[2].count = manipulator_data.vertices.cursor;
    components[2].start = manipulator_data.vertices.cursor * 2;
    for(uint32_t vert_index = 0; vert_index < manipulator_data.vertices.cursor; vert_index++)
    {
        vec3_t *vert = get_list_element(&manipulator_data.vertices, vert_index);
        verts[vert_index].position = vec4_t_c(vert->z, vert->y, vert->x, 1.0);
        verts[vert_index].color = vec4_t_c(0.0, 0.0, 1.0, 1.0);
    }
    
    chunk = r_AllocChunk(r_vertex_heap, sizeof(struct r_i_vertex_t) * ed_rotation_manipulator_vert_count, sizeof(struct r_i_vertex_t));
    r_FillBufferChunk(chunk, ed_manipulators[ED_TRANSFORM_TYPE_ROTATION].vertices, sizeof(struct r_i_vertex_t) * ed_rotation_manipulator_vert_count, 0);
    ed_manipulators[ED_TRANSFORM_TYPE_ROTATION].chunk = chunk;
    
    destroy_list(&manipulator_data.vertices);
    destroy_list(&manipulator_data.normals);
    destroy_list(&manipulator_data.tangents);
    destroy_list(&manipulator_data.tex_coords);
    destroy_list(&manipulator_data.batches);
}

void ed_w_ctx_Input(void *context_data, struct ed_editor_window_t *window)
{
    struct ed_world_context_data_t *data = (struct ed_world_context_data_t *)context_data;
    
    ed_w_ctx_StateFunction[data->context_state](data, window);
        
    if(data->just_changed_state)
    {
        data->just_changed_state--;
    }
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
    
    objects = &data->target_data[data->current_selection_target].objects;
    selections = &data->target_data[data->current_selection_target].selections;
    
    if(selections->cursor)
    {    
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
                    mem_CheckGuards();
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
        ed_w_ctx_ManipulatorDrawTransform(&draw_transform, &data->manipulator_state, viewport);
        
        struct ed_manipulator_t *manipulator = ed_manipulators + data->manipulator_state.transform_type;
        for(uint32_t component_index = 0; component_index < manipulator->component_count; component_index++)
        {
            struct ed_manipulator_component_t *component = manipulator->components + component_index;
            r_i_DrawImmediate(manipulator->vertices + component->start, component->count, NULL, 0, &draw_transform);
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

uint32_t ed_w_ctx_Layout(void *context_data, struct ed_editor_window_t *window)
{
    struct ed_world_context_data_t *data = (struct ed_world_context_data_t *)context_data;
    struct ed_editor_viewport_t *viewport = (struct ed_editor_viewport_t *)window;
    char window_name[256];
    int32_t mouse_x;
    int32_t mouse_y;
    uint32_t hovered = 0;
    
    in_GetMousePos(&mouse_x, &mouse_y);
    
    sprintf(window_name, "Viewport-%p", viewport);
    igSetNextWindowSize((ImVec2){(float)viewport->width, (float)viewport->height}, ImGuiCond_Once);
    ImGuiStyle *style = igGetStyle();
    uint32_t window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;
    
    if(data->selection_area_active)
    {
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    
    ImVec2 size;
    igGetContentRegionAvail(&size);
    
    igPushStyleVarFloat(ImGuiStyleVar_IndentSpacing, 0.0);
    igPushStyleVarVec2(ImGuiStyleVar_WindowPadding, (ImVec2){8.0, 8.0});
    igPushStyleVarFloat(ImGuiStyleVar_WindowRounding, 0.0);
    
//    if(igBeginChildStr(window_name, (ImVec2){viewport->width, viewport->height}, 1, window_flags))
    if(igBegin(window_name, NULL, window_flags))
    {
        igPopStyleVar(3);
        
        if(igBeginChildStr("buttons", (ImVec2){viewport->width, 20.0}, 0, 0))
        {
            char preview_str[256];
            float *thresholds;
            uint32_t thresholds_count;
            float *snapping_value;
            float preview_value;
            char *label;
            
            
            switch(data->manipulator_state.transform_type)
            {
                case ED_TRANSFORM_TYPE_TRANSLATION:
                    thresholds = ed_linear_thresholds;
                    thresholds_count = sizeof(ed_linear_thresholds) / sizeof(ed_linear_thresholds[0]);
                    snapping_value = &data->manipulation_state.linear_threshold;
                    label = "Linear snapping";
                    preview_value = data->manipulation_state.linear_threshold;
                break;
                
                case ED_TRANSFORM_TYPE_ROTATION:
                    thresholds = ed_angular_thresholds;
                    thresholds_count = sizeof(ed_linear_thresholds) / sizeof(ed_linear_thresholds[0]);
                    snapping_value = &data->manipulation_state.angular_threshold;
                    label = "Angular snapping";
                    preview_value = data->manipulation_state.angular_threshold;
                break;
            }
            
            sprintf(preview_str, "%0.2f", preview_value);
            igSetNextItemWidth(80.0);
            if(igBeginCombo(label, preview_str, 0))
            {
                for(uint32_t value_index = 0; value_index < thresholds_count; value_index++)
                {
                    sprintf(preview_str, "%0.2f", thresholds[value_index]);
                    if(igSelectableBool(preview_str, 0, 0, (ImVec2){0.0, 0.0}))
                    {
                        *snapping_value = thresholds[value_index];
                    }
                }
                
                igEndCombo();
            }

        }
        igEndChild();
        
        igSeparator();
        
        struct r_framebuffer_t *framebuffer = r_GetFramebufferPointer(viewport->framebuffer);
        struct r_texture_t *texture = r_GetTexturePointer(framebuffer->textures[0]);
        ImTextureID texture_id = (void *)framebuffer->textures[0].index;
        if(igBeginChildStr("viewport", (ImVec2){0.0, 0.0}, 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImVec2 content_min;
            ImVec2 content_max;
            igGetContentRegionAvail(&content_max);
            
            if(viewport->width != (uint32_t)content_max.x || (uint32_t)viewport->height != content_max.y)
            {
                viewport->width = (uint32_t)content_max.x;
                viewport->height = (uint32_t)content_max.y;
                r_ResizeFramebuffer(viewport->framebuffer, viewport->width, viewport->height);
            }
            
            igImage(texture_id, (ImVec2){(float) viewport->width, (float) viewport->height}, 
                (ImVec2){0.0, 0.0}, (ImVec2){1.0, 1.0}, (ImVec4){1.0, 1.0, 1.0, 1.0}, (ImVec4){0.0, 0.0, 0.0, 0.0});
                
            if(window->focused)
            {
                data->selection_area_active = igIsItemHovered(0);
            }
            
            ImVec2 position;
            igGetItemRectMin(&position);
            
            viewport->x = position.x;
            viewport->y = position.y;
            viewport->mouse_x = mouse_x - viewport->x;
            viewport->mouse_y = mouse_y - viewport->y;
        }
        igEndChild();
        
        hovered = igIsWindowHovered(ImGuiHoveredFlags_ChildWindows);
    }
    igEnd();
    
    return hovered;
}

void ed_w_ctx_Reset(void *context_data)
{
    struct ed_world_context_data_t *data = (struct ed_world_context_data_t *)context_data;
    struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects;
    for(uint32_t object_index = 0; object_index < objects->cursor; object_index++)
    {
        ed_w_ctx_DestroyObject(objects, ED_OBJECT_HANDLE(object_index));
    }
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].selections.cursor = 0;
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].objects.cursor = 0;
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].selections.cursor = 0;
    
    data->context_state = ED_WORLD_CONTEXT_STATE_IDLE;
    data->just_changed_state = 0;
    data->current_selection_target = ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT;
}

/*
===========================================================================================================================================
===========================================================================================================================================
===========================================================================================================================================
*/

struct ed_object_h ed_w_ctx_CreateObject(struct stack_list_t *objects, mat4_t *transform, uint32_t type, uint32_t start, uint32_t count, uint32_t vertex_count, uint32_t vertex_offset, uint32_t topology, union ed_object_ref_t object_ref)
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
    object->topology = topology;
    object->object = object_ref;
    
    return handle;
}

void ed_w_ctx_DestroyObject(struct stack_list_t *objects, struct ed_object_h handle)
{
    struct ed_object_t *object;
    
    object = ed_w_ctx_GetObjectPointer(objects, handle);
    if(object)
    {
        switch(object->type)
        {
            case ED_OBJECT_TYPE_BRUSH:
                bsh_DestroyBrush(object->object.brush);
            break;
        }
        
        object->type = ED_OBJECT_TYPE_LAST;
        remove_stack_list_element(objects, handle.index);
    }
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
    mat4_t_comp(&transform, &brush->orientation, &brush->position);
    
    return ed_w_ctx_CreateObject(objects, &transform, ED_OBJECT_TYPE_BRUSH, brush->start, brush->count, brush->draw_vertice_count, brush->vertex_offset, 
                                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, (union ed_object_ref_t){.brush = brush_handle});
}

struct ed_object_h ed_w_ctx_CreateBrushFaceObject(struct stack_list_t *objects, mat4_t *transform, struct bsh_brush_h handle, uint32_t face_index)
{
    struct ed_brush_t *brush = ed_GetBrushPointer(handle);
    struct ed_brush_face_t *face = ed_GetBrushFacePointer(handle, face_index);
    union ed_object_ref_t ref = (union ed_object_ref_t){.brush = handle, .face_index = face_index};
    
    struct ed_object_h face_handle = ed_w_ctx_CreateObject(objects, transform, ED_OBJECT_TYPE_FACE, 0, 0, 0, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, ref);
    struct ed_object_t *face_object = ed_w_ctx_GetObjectPointer(objects, face_handle);
    face_object->start = face->draw_indices_start + brush->start;
    face_object->count = face->draw_indices_count;
    face_object->vertex_count = face->indice_count;
    face_object->vertex_offset = brush->vertex_offset;
    
    return face_handle;
}

struct ed_object_h ed_w_ctx_CreateBrushEdgeObject(struct stack_list_t *objects, struct bsh_brush_h handle, uint32_t face_index, uint32_t edge_index)
{
//    struct ed_brush_t *brush = ed_GetBrushPointer(handle);
//    struct ed_brush_face_t *face = ed_GetBrushFacePointer(handle, face_index);
//    union ed_object_ref_t ref = (union ed_object_ref_t){.brush = handle, .face_index = face_index};
//    
//    struct ed_object_h edge_handle = ed_w_ctx_CreateObject(objects, transform, ED_OBJECT_TYPE_EDGE, 0, 0, 0, 0, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, ref);
//    struct ed_object_t *edge_object = ed_w_ctx_GetObjectPointer(objects, edge_handle);
//    edge_object->start = face->draw_indices_start + brush->start;
//    edge_object->count = 2;
//    edge_object->vertex_count = 2;
//    edge_object->vertex_offset = brush->vertex_offset;
}

struct ed_object_h ed_w_ctx_CreateBrushObjectFromBrush(struct stack_list_t *objects, struct bsh_brush_h handle)
{
    struct ed_brush_t *brush;
    mat3_t orientation;
    mat4_t transform;
    mat4_t scale;
    
    mat3_t_identity(&orientation);
    brush = ed_GetBrushPointer(handle);
    mat4_t_comp(&transform, &brush->orientation, &brush->position);
    
    return ed_w_ctx_CreateObject(objects, &transform, ED_OBJECT_TYPE_BRUSH, brush->start, brush->count, 
                                 brush->draw_vertice_count, brush->vertex_offset, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, (union ed_object_ref_t){.brush = handle});
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
    data->just_changed_state = 2;
}

void ed_w_ctx_IdleState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    data->selection = ED_INVALID_OBJECT_HANDLE;
    data->manipulation_state.setup = 0;
    data->manipulation_state.axis_constraint = vec3_t_c(0.0, 0.0, 0.0);
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
    else if(in_GetKeyState(SDL_SCANCODE_LSHIFT) & IN_INPUT_STATE_PRESSED)
    {
        if(in_GetKeyState(SDL_SCANCODE_D) & IN_INPUT_STATE_JUST_PRESSED)
        {
            ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_COPYING_SELECTIONS);
        }
    }
    else if(in_GetKeyState(SDL_SCANCODE_DELETE) & IN_INPUT_STATE_JUST_PRESSED)
    {
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_DELETING_SELECTIONS);
    }
    else if(in_GetKeyState(SDL_SCANCODE_G) & IN_INPUT_STATE_JUST_PRESSED)
    {
        data->manipulator_state.transform_type = ED_TRANSFORM_TYPE_TRANSLATION;
    }
    else if(in_GetKeyState(SDL_SCANCODE_R) & IN_INPUT_STATE_JUST_PRESSED)
    {
        data->manipulator_state.transform_type = ED_TRANSFORM_TYPE_ROTATION;
    }
}

void ed_w_ctx_LeftClickState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    /* object picking happens when the left button is released */
    uint32_t mouse_state = in_GetMouseState(IN_MOUSE_BUTTON_LEFT) & (IN_INPUT_STATE_JUST_PRESSED | IN_INPUT_STATE_PRESSED | IN_INPUT_STATE_JUST_RELEASED);
    
    if(!data->selection_area_active)
    {
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_IDLE);
    }
    
    if(data->just_changed_state)
    {
        uint32_t picked_axis = 0;
        
        if(data->target_data[data->current_selection_target].selections.cursor)
        {
            picked_axis = ed_w_ctx_PickManipulator(&data->manipulator_state, (struct ed_editor_viewport_t *)window);
        }
        
        if(picked_axis)
        {
            data->manipulation_state.axis_constraint = ed_manipulator_plane_normal[picked_axis];
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
    else if((mouse_state & IN_INPUT_STATE_PRESSED) && (in_GetDragState() & IN_DRAG_STATE_DRAGGING))
    {
        if(data->target_data[data->current_selection_target].selections.cursor)
        {
            if(data->current_selection_target == ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT)
            {
                ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_MANIPULATING);
            }
            else if(in_GetKeyState(SDL_SCANCODE_LSHIFT) & IN_INPUT_STATE_PRESSED)
            {
                ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_EXTRUDING_BRUSH_FACES);
            }
        }
        else
        {
            ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_CREATING_BRUSH);
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
    
    if(!data->selection_area_active)
    {
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_IDLE);
    }
    
    if(data->just_changed_state)
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
                for(uint32_t face_index = 0; face_index < brush->polygon_count; face_index++)
                {
                    struct ed_object_h face_handle = ed_w_ctx_CreateBrushFaceObject(objects, &object->transform, object->object.brush, face_index);
                    add_list_element(&ed_object_handles, &face_handle);
                }
            }
        }
        
        data->selection = ed_w_ctx_PickObject(objects, &ed_object_handles, (struct ed_editor_viewport_t *)window);
    }
//    else if(mouse_state & IN_INPUT_STATE_PRESSED)
//    {
//        if(!data->target_data[data->current_selection_target].selections.cursor && (in_GetDragState() & IN_DRAG_STATE_DRAGGING))
//        {
//            ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_CREATING_BRUSH);
//        }
//    }
    else if(mouse_state & IN_INPUT_STATE_JUST_RELEASED)
    {
        ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_PICK_BRUSH_FACE);
    }
}

void ed_w_ctx_PickObjectState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects;
    struct list_t *selections = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].selections;
    
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].selections.cursor = 0;
    
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

int32_t ed_w_ctx_CompareBrushFaceObjectRef(void *a, void *b)
{
    union ed_object_ref_t *ref_a = (union ed_object_ref_t *)a;
    union ed_object_ref_t *ref_b = (union ed_object_ref_t *)b;
    return ref_a->brush.index - ref_b->brush.index;
}

void ed_w_ctx_PickBrushFaceState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].objects;
    struct list_t *selections = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].selections;
    
    data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].selections.cursor = 0;
    
    ed_object_handles.cursor = 0;
    add_list_element(&ed_object_handles, &data->selection);
    struct ed_object_h selection = ed_w_ctx_PickObject(objects, &ed_object_handles, (struct ed_editor_viewport_t *)window);
    
    if(selection.index == data->selection.index)
    {
        ed_w_ctx_AppendBrushSelection(selections, selection, in_GetKeyState(SDL_SCANCODE_LSHIFT) & IN_INPUT_STATE_PRESSED);
        qsort_list(selections, ed_w_ctx_CompareBrushFaceObjectRef);
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

        vec3_t forward_vec = vec3_t_c_vec4_t(&viewport->view_matrix.rows[2]);

        vec3_t mouse_vec = ed_w_ctx_WorldSpaceMouseVec(viewport);
        vec3_t_sub(&camera_plane_vec, &manipulator_pos, &camera_pos);
        
        if(data->just_changed_state)
        {
            vec3_t axis_constraint = data->manipulation_state.axis_constraint;
            
            switch(data->manipulator_state.transform_type)
            {
                case ED_TRANSFORM_TYPE_TRANSLATION:
                {
                    vec3_t plane_normal;
                    
                    if(axis_constraint.x == 0.0 && axis_constraint.y == 0.0 && axis_constraint.z == 0.0)
                    {
                        /* no axis constraint, so the transformation will happen along
                        the plane parallel to the view */
                        data->manipulation_state.pick_normal = forward_vec;
                    }
                    else
                    {
                        /* axis constraint in place, the transform will happen along
                        this axis, so find the plane that contains both the manipulator
                        center and the constraint axis */
                        plane_normal = axis_constraint;
                        float dist = vec3_t_dot(&camera_plane_vec, &plane_normal);
                        vec3_t_fmadd(&plane_point, &manipulator_pos, &plane_normal, -dist);
                        vec3_t_sub(&plane_normal, &camera_pos, &plane_point);
                        vec3_t_normalize(&data->manipulation_state.pick_normal, &plane_normal);
                    }
                }
                break;
                
                case ED_TRANSFORM_TYPE_ROTATION:
                    if(axis_constraint.x == 0.0 && axis_constraint.y == 0.0 && axis_constraint.z == 0.0)
                    {
                        data->manipulation_state.pick_normal = forward_vec;
                    }
                    else
                    {
                        data->manipulation_state.pick_normal = axis_constraint;
                    }
                break;
            }
        }
        
        float denom = vec3_t_dot(&mouse_vec, &data->manipulation_state.pick_normal);
        float intersection = 0.0;
        
        if(denom)
        {
            intersection = vec3_t_dot(&camera_plane_vec, &data->manipulation_state.pick_normal) / denom;                
        }
        
        vec3_t_fmadd(&plane_point, &camera_pos, &mouse_vec, intersection);
        
        if(data->just_changed_state)
        {
            vec3_t_sub(&data->manipulation_state.pick_offset, &manipulator_pos, &plane_point);
        }
        
        mat4_t transform;
        mat4_t_identity(&transform);
        
        switch(data->manipulator_state.transform_type)
        {
            case ED_TRANSFORM_TYPE_TRANSLATION:
            {
                vec3_t_add(&plane_point, &plane_point, &data->manipulation_state.pick_offset);
                vec3_t_sub(&drag_delta, &plane_point, &manipulator_pos);
            
                if(data->manipulation_state.linear_threshold)
                {
                    drag_delta.x -= fmodf(drag_delta.x, data->manipulation_state.linear_threshold);
                    drag_delta.y -= fmodf(drag_delta.y, data->manipulation_state.linear_threshold);
                    drag_delta.z -= fmodf(drag_delta.z, data->manipulation_state.linear_threshold);
                }
                
                vec3_t axis_constraint = data->manipulation_state.axis_constraint;
                if(axis_constraint.x || axis_constraint.y || axis_constraint.z)
                {
                    /* if there's any sort of axis constraint in place, make sure
                    we transform along the constraint axis */
                    float proj = vec3_t_dot(&axis_constraint, &drag_delta);
                    vec3_t_mul(&drag_delta, &axis_constraint, proj);
                }
                        
                transform.rows[3] = vec4_t_c(drag_delta.x, drag_delta.y, drag_delta.z, 1.0);
            }
            break;
            
            case ED_TRANSFORM_TYPE_ROTATION:
            {
                vec3_t pick_vec;
                vec3_t pick_offset = data->manipulation_state.pick_offset;
                vec3_t pick_normal = data->manipulation_state.axis_constraint;
                vec3_t_sub(&pick_vec, &plane_point, &manipulator_pos);
                vec3_t_normalize(&pick_vec, &pick_vec);
                vec3_t_normalize(&pick_offset, &pick_offset);
                vec3_t pick_angle;
                vec3_t_cross(&pick_angle, &pick_vec, &pick_offset);
                float rotation = vec3_t_dot(&pick_angle, &pick_normal) / 3.14159265;
                
                if(data->manipulation_state.angular_threshold)
                {
                    float threshold = data->manipulation_state.angular_threshold / 180.0;
                    rotation -= fmodf(rotation, threshold);
                }
                
                if(rotation)
                {
                    vec3_t_sub(&data->manipulation_state.pick_offset, &plane_point, &manipulator_pos);
                }
                
                if(pick_normal.x > 0.0)
                {
                    mat4_t_rotate_x(&transform, rotation);
                }
                else if(pick_normal.y > 0.0)
                {
                    mat4_t_rotate_y(&transform, rotation);
                }
                else
                {
                    mat4_t_rotate_z(&transform, rotation);
                }
            }
            break;
        }
        
        struct ed_manipulator_state_t *state = &data->manipulator_state;
        
        if(data->current_selection_target == ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT)
        {
            ed_w_ctx_TransformObjects(&transform, objects, selections, state->transform_type, state->transform_mode);
        }
        else
        {
            ed_w_ctx_TransformBrushElements(&transform, objects, selections, state->transform_type, state->transform_mode);
        }
                
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
//            if(!data->just_changed_state)
//            {
//                vec3_t direction;
//                vec3_t_sub(&direction, &plane_point, &data->creating_brush_state.start_corner);
//                
////                plane_point = data->creating_brush_state.start_corner;
//                
////                if(direction.x < 0.0)
////                {
////                    plane_point.x += floorf(direction.x / data->manipulation_state.linear_threshold);
////                }
////                else
////                {
////                    plane_point.x += ceilf(direction.x / data->manipulation_state.linear_threshold);
////                }
////                
////                
////                if(direction.z < 0.0)
////                {
////                    plane_point.z += floorf(direction.z / data->manipulation_state.linear_threshold);
////                }
////                else
////                {
////                    plane_point.z += ceilf(direction.z / data->manipulation_state.linear_threshold);
////                }
//            }
            
            plane_point.x -= fmodf(plane_point.x, data->manipulation_state.linear_threshold);
            plane_point.z -= fmodf(plane_point.z, data->manipulation_state.linear_threshold);
        }
        
        if(data->just_changed_state)
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

void ed_w_ctx_ExtrudingBrushFacesState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    struct list_t *selections = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].selections;
    struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_BRUSH].objects;
    
    for(uint32_t selection_index = 0; selection_index < selections->cursor; selection_index++)
    {
        struct ed_object_h handle = *(struct ed_object_h  *)get_list_element(selections, selection_index);
        struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
        struct bsh_brush_h brush_handle = object->object.brush;
        struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
        ed_ExtrudeBrushFace(object->object.brush, object->object.face_index);
        bsh_UpdateDrawTriangles(object->object.brush);
        
        for(uint32_t object_index = 0; object_index < data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects.cursor; object_index++)
        {
            object = ed_w_ctx_GetObjectPointer(&data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects, ED_OBJECT_HANDLE(object_index));
            if(object && object->type == ED_OBJECT_TYPE_BRUSH)
            {
                if(object->object.brush.index == brush_handle.index)
                {
                    object->start = brush->start;
                    object->count = brush->count;
                    object->vertex_count = brush->draw_vertice_count;
                    object->vertex_offset = brush->vertex_offset;
                    break;
                }
            }
        }
    }
    
    if(selections->cursor == 1)
    {
        struct ed_object_h handle = *(struct ed_object_h  *)get_list_element(selections, 0);
        struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
        struct ed_brush_face_t *face = ed_GetBrushFacePointer(object->object.brush, object->object.face_index);
        struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
        mat3_t_vec3_t_mul(&data->manipulation_state.axis_constraint, &face->face_normal, &brush->orientation);
    }
    
    ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_MANIPULATING);
}

void ed_w_ctx_CopyingSelectionsState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    if(data->current_selection_target == ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT)
    {
        struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects;
        struct list_t *selections = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].selections;
        ed_w_ctx_CopySelections(objects, selections);
    }
    
    ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_IDLE);
}

void ed_w_ctx_DeletingSelectionsState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    if(data->current_selection_target == ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT)
    {
        struct stack_list_t *objects = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].objects;
        struct list_t *selections = &data->target_data[ED_WORLD_CONTEXT_SELECTION_TARGET_OBJECT].selections;
        ed_w_ctx_DeleteSelections(objects, selections);
    }
    
    ed_w_ctx_SetState(data, ED_WORLD_CONTEXT_STATE_IDLE);
}

void ed_w_ctx_FlyingState(struct ed_world_context_data_t *data, struct ed_editor_window_t *window)
{
    if(data->just_changed_state)
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

vec3_t ed_w_ctx_WorldSpaceMouseVec(struct ed_editor_viewport_t *viewport)
{
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
    
    return mouse_vec;
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
                    vec3_t transformed_vec = brush->vertices[face->indices[vert_index]];
                    mat3_t_vec3_t_mul(&transformed_vec, &transformed_vec, &brush->orientation);
                    vec3_t_add(&face_position, &face_position, &transformed_vec);
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

void ed_w_ctx_ManipulatorDrawTransform(mat4_t *draw_transform, struct ed_manipulator_state_t *manipulator_state, struct ed_editor_viewport_t *viewport)
{
    vec3_t view_pos = vec3_t_c_vec4_t(&viewport->view_matrix.rows[3]);
    vec3_t manipulator_pos = vec3_t_c_vec4_t(&manipulator_state->transform.rows[3]);
    vec3_t view_manipulator_vec;
    
    vec3_t_sub(&view_manipulator_vec, &manipulator_pos, &view_pos);
    float scale = vec3_t_length(&view_manipulator_vec);
    
    switch(manipulator_state->transform_type)
    {
        case ED_TRANSFORM_TYPE_TRANSLATION:
            scale *= 0.015;
        break;
        
        case ED_TRANSFORM_TYPE_ROTATION:
            scale *= 0.25;
        break;
    }
    
    *draw_transform = manipulator_state->transform;
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

void ed_w_ctx_TransformObjects(mat4_t *transform, struct stack_list_t *objects, struct list_t *selections, uint32_t transform_type, uint32_t transform_mode)
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
                }
            }
        }
        break;
        
        case ED_TRANSFORM_TYPE_ROTATION:
        {
            mat3_t orientation;
            orientation.rows[0] = vec3_t_c_vec4_t(&transform->rows[0]);
            orientation.rows[1] = vec3_t_c_vec4_t(&transform->rows[1]);
            orientation.rows[2] = vec3_t_c_vec4_t(&transform->rows[2]);
            
            for(uint32_t selection_index = 0; selection_index < selections->cursor; selection_index++)
            {
                struct ed_object_h handle = *(struct ed_object_h *)get_list_element(selections, selection_index);
                struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
                
                switch(object->type)
                {
                    case ED_OBJECT_TYPE_BRUSH:
                    {
                        bsh_RotateBrush(object->object.brush, &orientation);
                        struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
                        mat4_t_identity(&object->transform);
                        mat4_t_comp(&object->transform, &brush->orientation, &brush->position);
                    }
                    break;
                }
            }
        }
        break;          
    }
}

void ed_w_ctx_TransformBrushElements(mat4_t *transform, struct stack_list_t *objects, struct list_t *selections, uint32_t transform_type, uint32_t transform_mode)
{
    struct ed_brush_t *current_brush = NULL;
    struct bsh_brush_h current_brush_handle;
    
    ed_transform_brush_face_indices.cursor = 0;
    
    switch(transform_type)
    {
        case ED_TRANSFORM_TYPE_TRANSLATION:
        {
            vec3_t translation;
            translation.x = transform->rows[3].x;
            translation.y = transform->rows[3].y;
            translation.z = transform->rows[3].z;
            uint32_t selection_index = 0;
            while(1)
            {
                for(; selection_index < selections->cursor; selection_index++)
                {
                    struct ed_object_h handle = *(struct ed_object_h *)get_list_element(selections, selection_index);
                    struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, handle);
                    struct ed_brush_t *brush = ed_GetBrushPointer(object->object.brush);
                    struct ed_brush_face_t *face = ed_GetBrushFacePointer(object->object.brush, object->object.face_index);
                    
                    if((brush != current_brush) && ed_transform_brush_face_indices.cursor)
                    {
                        break;
                    }
                    
                    current_brush = brush;
                    current_brush_handle = object->object.brush;
                    
                    for(uint32_t vert_index = 0; vert_index < face->indice_count; vert_index++)
                    {
                        if(find_list_element(&ed_transform_brush_face_indices, &face->indices[vert_index]) == 0xffffffff)
                        {
                            add_list_element(&ed_transform_brush_face_indices, &face->indices[vert_index]);
                        }
                    }
                }
                
                if(!ed_transform_brush_face_indices.cursor)
                {
                    break;
                }
                
                vec3_t rotated_translation;
                mat3_t inverse_rotation = current_brush->orientation;
                mat3_t_transpose(&inverse_rotation, &inverse_rotation);
                mat3_t_vec3_t_mul(&rotated_translation, &translation, &inverse_rotation);
                
                for(uint32_t vert_index = 0; vert_index < ed_transform_brush_face_indices.cursor; vert_index++)
                {
                    uint32_t indice = *(uint32_t *)get_list_element(&ed_transform_brush_face_indices, vert_index);
                    vec3_t *vert = current_brush->vertices + indice;
                    vec3_t_add(vert, vert, &rotated_translation);
                }
                ed_transform_brush_face_indices.cursor = 0;
                bsh_UpdateDrawTriangles(current_brush_handle);
            }
        }
        break;
    }
}


uint32_t ed_w_ctx_PickManipulator(struct ed_manipulator_state_t *manipulator_state, struct ed_editor_viewport_t *viewport)
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
    
    if(viewport->mouse_x < 0 || viewport->mouse_x >= viewport->width)
    {
        return 0;
    }
    
    if(viewport->mouse_y < 0 || viewport->mouse_y >= viewport->height)
    {
        return 0;
    }
    
    struct
    {
        mat4_t model_view_projection_matrix;
        uint32_t object_index;
    }push_constant;
      
    mat4_t_mul(&view_projection_matrix, &viewport->inv_view_matrix, &viewport->projection_matrix);
    render_pass = r_GetRenderPassPointer(ed_picking_render_pass);
    pipeline = r_GetPipelinePointer(render_pass->pipelines[0]);
    command_buffer = ed_BeginPicking(viewport);
    
    
//    index_chunk = r_GetChunkPointer(ed_translation_manipulator_index_chunk);
    
    ed_w_ctx_ManipulatorDrawTransform(&push_constant.model_view_projection_matrix, manipulator_state, viewport);
    mat4_t_mul(&push_constant.model_view_projection_matrix, &push_constant.model_view_projection_matrix, &view_projection_matrix);
    
    manipulator = ed_manipulators + manipulator_state->transform_type;
    vertex_chunk = r_GetChunkPointer(manipulator->chunk);
    uint32_t vertex_start = vertex_chunk->start / sizeof(struct r_i_vertex_t);
    
    for(uint32_t component_index = 0; component_index < manipulator->component_count; component_index++)
    {
        struct ed_manipulator_component_t *component = manipulator->components + component_index;
        push_constant.object_index = component->id;
        r_vkCmdPushConstants(command_buffer, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constant), &push_constant);
        r_vkCmdDraw(command_buffer, component->count, 1, vertex_start + component->start, 0);
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
    
    if(viewport->mouse_x < 0 || viewport->mouse_x >= viewport->width)
    {
        return ED_INVALID_OBJECT_HANDLE;
    }
    
    if(viewport->mouse_y < 0 || viewport->mouse_y >= viewport->height)
    {
        return ED_INVALID_OBJECT_HANDLE;
    }
    
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

void ed_w_ctx_CopySelections(struct stack_list_t *objects, struct list_t *selections)
{
    for(uint32_t selection_index = 0; selection_index < selections->cursor; selection_index++)
    {
        struct ed_object_h *handle = get_list_element(selections, selection_index);
        struct ed_object_t *object = ed_w_ctx_GetObjectPointer(objects, *handle);
        
        switch(object->type)
        {
            case ED_OBJECT_TYPE_BRUSH:
            {
                struct bsh_brush_h new_brush = ed_CopyBrush(object->object.brush);
                *handle = ed_w_ctx_CreateBrushObjectFromBrush(objects, new_brush);                
            }            
            break;
        }
    }
}

void ed_w_ctx_DeleteSelections(struct stack_list_t *objects, struct list_t *selections)
{
    for(uint32_t selection_index = 0; selection_index < selections->cursor; selection_index++)
    {
        struct ed_object_h handle = *(struct ed_object_h *)get_list_element(selections, selection_index);
        ed_w_ctx_DestroyObject(objects, handle);
    }
    
    selections->cursor = 0;
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
