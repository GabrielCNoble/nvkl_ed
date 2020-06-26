#include "r_ed.h"
#include "neighbor/r_draw.h"

struct r_i_vertex_t r_tri_verts[512];

void r_DrawBrush(struct bsh_brush_t *brush)
{
    struct bsh_polygon_t *polygon;
    polygon = brush->polygons;
    
    while(polygon)
    {
        for(uint32_t vert_index = 0, brush_index = 0; vert_index < 6;)
        {
            r_tri_verts[vert_index].position.x = polygon->vertices[brush_index].position.x;
            r_tri_verts[vert_index].position.y = polygon->vertices[brush_index].position.y;
            r_tri_verts[vert_index].position.z = polygon->vertices[brush_index].position.z;
            r_tri_verts[vert_index].position.w = 1.0;
            r_tri_verts[vert_index].color = vec4_t_c(1.0, 1.0, 1.0, 1.0);   
            
            vert_index++;
            brush_index++;
            
            r_tri_verts[vert_index].position.x = polygon->vertices[brush_index].position.x;
            r_tri_verts[vert_index].position.y = polygon->vertices[brush_index].position.y;
            r_tri_verts[vert_index].position.z = polygon->vertices[brush_index].position.z;
            r_tri_verts[vert_index].position.w = 1.0;
            r_tri_verts[vert_index].color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
            
            vert_index++;
            brush_index = (brush_index + 1) % polygon->vert_count;
            
            r_tri_verts[vert_index].position.x = polygon->vertices[brush_index].position.x;
            r_tri_verts[vert_index].position.y = polygon->vertices[brush_index].position.y;
            r_tri_verts[vert_index].position.z = polygon->vertices[brush_index].position.z;
            r_tri_verts[vert_index].position.w = 1.0;
            r_tri_verts[vert_index].color = vec4_t_c(1.0, 1.0, 1.0, 1.0);
            
            vert_index++;
            
        }
        
        r_i_DrawTrisImmediate(r_tri_verts, 6, NULL, 0, 1);
        polygon = polygon->next;
    }
}

void r_DrawBrushes()
{
    struct bsh_brush_t *brush;
    brush = bsh_GetBrushList();
    struct r_view_t *view;
    
    view = r_GetViewPointer();
    
    r_i_BeginSubmission(&view->inv_view_matrix, &view->projection_matrix);
    while(brush)
    {
        r_DrawBrush(brush);
        brush = brush->next;
    }
    r_i_EndSubmission();
}





