#include "r_ed.h"
#include "neighbor/r_draw.h"

struct r_i_vertex_t r_tri_verts[512];
uint32_t r_tri_indices[512*3];

void r_ed_DrawPolygon(struct bsh_polygon_t *polygon, vec3_t *color, uint32_t wireframe)
{
    uint32_t index_index = 0;
    for(uint32_t vert_index = 0; vert_index < polygon->vert_count; vert_index++)
    {
        r_tri_verts[vert_index].position.x = polygon->vertices[vert_index].position.x;
        r_tri_verts[vert_index].position.y = polygon->vertices[vert_index].position.y;
        r_tri_verts[vert_index].position.z = polygon->vertices[vert_index].position.z;
        r_tri_verts[vert_index].position.w = 1.0;     
        
        r_tri_verts[vert_index].color = vec4_t_c(color->x, color->y, color->z, 1.0);       
    }
    
    if(!wireframe)
    {   
        for(uint32_t vert_index = 1; vert_index < polygon->vert_count - 1;)
        {
            r_tri_indices[index_index] = 0;
            index_index++;
            r_tri_indices[index_index] = vert_index;
            index_index++;
            vert_index++;
            r_tri_indices[index_index] = vert_index;
            index_index++;
        }
        
        r_i_DrawTrisImmediate(r_tri_verts, polygon->vert_count, r_tri_indices, index_index, 1);
    }
    else
    {
        r_i_DrawLinesImmediate(r_tri_verts, polygon->vert_count, 1.0, 1);
    }
}

void r_ed_DrawBrush(struct bsh_brush_t *brush)
{
    struct bsh_polygon_t *polygon;
    polygon = brush->polygons;
    mat4_t transform;
    mat4_t_identity(&transform);
    
    r_Draw(brush->start, brush->count, r_GetDefaultMaterialPointer(), &transform);
}

void r_ed_DrawBrushes()
{
    struct stack_list_t *brushes;
    struct r_view_t *view;
    brushes = bsh_GetBrushList();
    mat4_t transform;
    struct r_begin_submission_info_t begin_info;
    
    mat4_t_identity(&transform);
    
    view = r_GetViewPointer();
    
    begin_info.inv_view_matrix = view->inv_view_matrix;
    begin_info.projection_matrix = view->projection_matrix;
    begin_info.framebuffer = R_INVALID_FRAMEBUFFER_HANDLE;
    begin_info.viewport = view->viewport;
    begin_info.scissor = view->scissor;
    begin_info.clear_framebuffer = 1;
    
    r_BeginSubmission(&begin_info);
    for(uint32_t brush_index = 0; brush_index < brushes->cursor; brush_index++)
    {
        struct bsh_brush_t *brush = bsh_GetBrushPointer(BSH_BRUSH_HANDLE(brush_index));
        if(brush)
        {
            r_Draw(brush->start, brush->count, r_GetDefaultMaterialPointer(), &transform);
        }
    }
    r_EndSubmission();
}





