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
    
    while(polygon)
    {
        for(uint32_t vert_index = 0; vert_index < 4; vert_index++)
        {
            r_tri_verts[vert_index].position.x = polygon->vertices[vert_index].position.x;
            r_tri_verts[vert_index].position.y = polygon->vertices[vert_index].position.y;
            r_tri_verts[vert_index].position.z = polygon->vertices[vert_index].position.z;
            r_tri_verts[vert_index].position.w = 1.0;     
            
            r_tri_verts[vert_index].color = vec4_t_c(1.0, 1.0, 1.0, 1.0);       
        }
        
        r_tri_indices[0] = 0;
        r_tri_indices[1] = 1;
        r_tri_indices[2] = 2;
        r_tri_indices[3] = 2;
        r_tri_indices[4] = 3;
        r_tri_indices[5] = 0;
        
        r_i_DrawTrisImmediate(r_tri_verts, 4, r_tri_indices, 6, 1);
        polygon = polygon->next;
    }
}

void r_ed_DrawBrushes()
{
    struct bsh_brush_t *brush;
    brush = bsh_GetBrushList();
    struct r_view_t *view;
    
    view = r_GetViewPointer();
    r_RecomputeInvViewMatrix();
    r_RecomputeProjectionMatrix();
    r_i_BeginSubmission(&view->inv_view_matrix, &view->projection_matrix);
    while(brush)
    {
        r_ed_DrawBrush(brush);
        brush = brush->next;
    }
    r_i_EndSubmission();
}





