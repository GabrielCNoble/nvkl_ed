#include "bsh.h"
#include "neighbor/lib/dstuff/ds_mem.h"
#include "neighbor/r_draw.h"
#include <string.h>
#include <stdlib.h>

//struct bsh_brush_t *bsh_brushes = NULL;
//struct bsh_brush_t *bsh_last_brush = NULL;

struct stack_list_t bsh_brushes;

uint32_t bsh_cube_brush_vert_count = 8;
vec3_t bsh_cube_brush_verts[] = 
{
    vec3_t_c( 0.5, 0.5,-0.5),
    vec3_t_c( 0.5,-0.5,-0.5),
    vec3_t_c(-0.5,-0.5,-0.5),
    vec3_t_c(-0.5, 0.5,-0.5),
    
    vec3_t_c( 0.5, 0.5, 0.5),
    vec3_t_c( 0.5,-0.5, 0.5),
    vec3_t_c(-0.5,-0.5, 0.5),
    vec3_t_c(-0.5, 0.5, 0.5),
};


//uint32_t bsh_max_draw_verts;
//struct r_vertex_t *bsh_draw_verts;
//uint32_t bsh_max_draw_indices;
//uint32_t *bsh_draw_indices;

uint32_t bsh_cube_brush_indice_count = 24;
uint32_t bsh_cube_brush_indices[] = 
{
    /* +Z */
    7, 6, 5, 4,
    
    /* -Z */
    0, 1, 2, 3,
    
    /* +X */
    3, 2, 6, 7,
    
    /* -X */
    4, 5, 1, 0,
    
    /* +Y */
    4, 0, 3, 7,
    
    /* -Y */
    1, 5, 6, 2
};
//struct r_vertex_t bsh_cube_brush_verts[] = 
//{
//    /* +X */
//    (struct r_vertex_t){.position = { 0.5, 0.5, 0.5, 1.0}, .normal = { 1.0, 0.0, 0.0, 0.0}, /*.tangent = { 0.0, 0.0, 1.0},*/ .tex_coords = {0.0, 0.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5,-0.5, 0.5, 1.0}, .normal = { 1.0, 0.0, 0.0, 0.0}, /*.tangent = { 0.0, 0.0, 1.0},*/ .tex_coords = {0.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5,-0.5,-0.5, 1.0}, .normal = { 1.0, 0.0, 0.0, 0.0}, /*.tangent = { 0.0, 0.0, 1.0},*/ .tex_coords = {1.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5, 0.5,-0.5, 1.0}, .normal = { 1.0, 0.0, 0.0, 0.0}, /*.tangent = { 0.0, 0.0, 1.0},*/ .tex_coords = {1.0, 0.0, 0.0, 0.0}},
//    
//    /* -X */
//    (struct r_vertex_t){.position = {-0.5, 0.5,-0.5, 1.0}, .normal = {-1.0, 0.0, 0.0, 0.0}, /*.tangent = { 0.0, 0.0,-1.0},*/ .tex_coords = {0.0, 0.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = {-0.5,-0.5,-0.5, 1.0}, .normal = {-1.0, 0.0, 0.0, 0.0}, /*.tangent = { 0.0, 0.0,-1.0},*/ .tex_coords = {0.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = {-0.5,-0.5, 0.5, 1.0}, .normal = {-1.0, 0.0, 0.0, 0.0}, /*.tangent = { 0.0, 0.0,-1.0},*/ .tex_coords = {1.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = {-0.5, 0.5, 0.5, 1.0}, .normal = {-1.0, 0.0, 0.0, 0.0}, /*.tangent = { 0.0, 0.0,-1.0},*/ .tex_coords = {1.0, 0.0, 0.0, 0.0}},
//    
//    /* +Y */
//    (struct r_vertex_t){.position = {-0.5, 0.5,-0.5, 1.0}, .normal = { 0.0, 1.0, 0.0, 0.0}, /*.tangent = {-1.0, 0.0, 1.0},*/ .tex_coords = {0.0, 0.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = {-0.5, 0.5, 0.5, 1.0}, .normal = { 0.0, 1.0, 0.0, 0.0}, /*.tangent = {-1.0, 0.0, 1.0},*/ .tex_coords = {0.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5, 0.5, 0.5, 1.0}, .normal = { 0.0, 1.0, 0.0, 0.0}, /*.tangent = {-1.0, 0.0, 1.0},*/ .tex_coords = {1.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5, 0.5,-0.5, 1.0}, .normal = { 0.0, 1.0, 0.0, 0.0}, /*.tangent = {-1.0, 0.0, 1.0},*/ .tex_coords = {1.0, 0.0, 0.0, 0.0}},
//    
//    /* -Y */
//    (struct r_vertex_t){.position = {-0.5,-0.5, 0.5, 1.0}, .normal = { 0.0,-1.0, 0.0, 0.0}, /*.tangent = { 1.0, 0.0, 1.0},*/ .tex_coords = {0.0, 0.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = {-0.5,-0.5,-0.5, 1.0}, .normal = { 0.0,-1.0, 0.0, 0.0}, /*.tangent = { 1.0, 0.0, 1.0},*/ .tex_coords = {0.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5,-0.5,-0.5, 1.0}, .normal = { 0.0,-1.0, 0.0, 0.0}, /*.tangent = { 1.0, 0.0, 1.0},*/ .tex_coords = {1.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5,-0.5, 0.5, 1.0}, .normal = { 0.0,-1.0, 0.0, 0.0}, /*.tangent = { 1.0, 0.0, 1.0},*/ .tex_coords = {1.0, 0.0, 0.0, 0.0}},
//    
//    /* +Z */
//    (struct r_vertex_t){.position = {-0.5, 0.5, 0.5, 1.0}, .normal = { 0.0, 0.0, 1.0, 0.0}, /*.tangent = { 1.0, 0.0, 0.0},*/ .tex_coords = {0.0, 0.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = {-0.5,-0.5, 0.5, 1.0}, .normal = { 0.0, 0.0, 1.0, 0.0}, /*.tangent = { 1.0, 0.0, 0.0},*/ .tex_coords = {0.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5,-0.5, 0.5, 1.0}, .normal = { 0.0, 0.0, 1.0, 0.0}, /*.tangent = { 1.0, 0.0, 0.0},*/ .tex_coords = {1.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5, 0.5, 0.5, 1.0}, .normal = { 0.0, 0.0, 1.0, 0.0}, /*.tangent = { 1.0, 0.0, 0.0},*/ .tex_coords = {1.0, 0.0, 0.0, 0.0}},
//    
//    /* -Z */
//    (struct r_vertex_t){.position = {-0.5,-0.5,-0.5, 1.0}, .normal = { 0.0, 0.0,-1.0, 0.0}, /*.tangent = {-1.0, 0.0, 0.0},*/ .tex_coords = {0.0, 0.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = {-0.5, 0.5,-0.5, 1.0}, .normal = { 0.0, 0.0,-1.0, 0.0}, /*.tangent = {-1.0, 0.0, 0.0},*/ .tex_coords = {0.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5, 0.5,-0.5, 1.0}, .normal = { 0.0, 0.0,-1.0, 0.0}, /*.tangent = {-1.0, 0.0, 0.0},*/ .tex_coords = {1.0, 1.0, 0.0, 0.0}},
//    (struct r_vertex_t){.position = { 0.5,-0.5,-0.5, 1.0}, .normal = { 0.0, 0.0,-1.0, 0.0}, /*.tangent = {-1.0, 0.0, 0.0},*/ .tex_coords = {1.0, 0.0, 0.0, 0.0}},
//};

void bsh_Init()
{
    bsh_brushes = create_stack_list(sizeof(struct ed_brush_t), 512);
}

struct bsh_brush_h bsh_CreateBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t type)
{
    struct ed_brush_t *brush;
    struct bsh_brush_h handle;
    
    handle.index = add_stack_list_element(&bsh_brushes, NULL);
    brush = get_stack_list_element(&bsh_brushes, handle.index);
    
    brush->start = 0;
    brush->count = 0;
    brush->vertex_offset = 0;
    
    brush->face_indice_count = 0;
    
    brush->type = type;
    brush->subtractive = 0;
    brush->position = *position;
    brush->scale = *scale;
    brush->orientation = *orientation;
    brush->polygons = NULL;
    
    return handle;
}

struct bsh_brush_h bsh_CreateCubeBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale)
{
    struct ed_brush_t *brush;
    struct bsh_brush_h handle;
    struct ed_brush_face_t *polygon;
    struct bsh_vertex_t *vertices = bsh_cube_brush_verts;
    struct r_chunk_t *chunk;
    uint32_t draw_vert_index = 0;
    uint32_t indice_offset = 0;
    static struct r_vertex_t draw_verts[36];
    
    handle = bsh_CreateBrush(position, orientation, scale, BSH_BRUSH_TYPE_CUBE);
    brush = ed_GetBrushPointer(handle);
    
    brush->vertices = mem_Calloc(bsh_cube_brush_vert_count, sizeof(vec3_t));
    brush->face_indices = mem_Calloc(bsh_cube_brush_indice_count, sizeof(uint32_t));
    
    for(uint32_t vert_index = 0; vert_index < bsh_cube_brush_vert_count; vert_index++)
    {
        brush->vertices[vert_index] = bsh_cube_brush_verts[vert_index];
        brush->vertices[vert_index].x *= scale->x;
        brush->vertices[vert_index].y *= scale->y;
        brush->vertices[vert_index].z *= scale->z;
    }
    
    for(uint32_t polygon_index = 0; polygon_index < 6; polygon_index++)
    {        
        polygon = mem_Calloc(1, sizeof(struct ed_brush_face_t));
        polygon->next = NULL;
        polygon->indice_count = 4;
        polygon->indices = brush->face_indices + brush->face_indice_count;
        
        for(uint32_t index = 0; index < 4; index++)
        {
            polygon->indices[index] = bsh_cube_brush_indices[index + brush->face_indice_count];
        }
        
        brush->face_indice_count += 4;
        
        if(!brush->polygons)
        {
            brush->polygons = polygon;
        }
        else
        {
            brush->last_polygon->next = polygon;
        }
        brush->last_polygon = polygon;
    }
    
    bsh_UpdateDrawTriangles(handle);
    
    return handle;
}

struct bsh_brush_h bsh_CreateCylinderBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t vert_count)
{
    return BSH_INVALID_BRUSH_HANDLE;
}

void bsh_DestroyBrush(struct bsh_brush_h handle)
{
    struct bsh_polygon_t *next_polygon;
    struct ed_brush_t *brush;
    
    brush = ed_GetBrushPointer(handle);
    
    if(brush)
    {        
        while(brush->polygons)
        {
            next_polygon = brush->polygons->next;
//            mem_Free(brush->polygons->vertices);
            mem_Free(brush->polygons);
            brush->polygons = next_polygon;
        }
        
        brush->type = BSH_BRUSH_TYPE_NONE;
        remove_stack_list_element(&bsh_brushes, handle.index);
    }
}

struct ed_brush_t *ed_GetBrushPointer(struct bsh_brush_h handle)
{
    struct ed_brush_t *brush;
    brush = get_stack_list_element(&bsh_brushes, handle.index);
    
    if(brush && brush->type == BSH_BRUSH_TYPE_NONE)
    {
        brush = NULL;
    }
    
    return brush;
}

struct ed_brush_face_t *ed_GetBrushFacePointer(struct bsh_brush_h handle, uint32_t index)
{
    struct ed_brush_t *brush;
    struct ed_brush_face_t *polygon = NULL;
    uint32_t polygon_index = 0;
    brush = ed_GetBrushPointer(handle);
    if(brush)
    {
        polygon = brush->polygons;
        while(polygon)
        {
            if(index == polygon_index)
            {
                break;
            }
            polygon_index++;
            polygon = polygon->next;
        }
    }
    
    return polygon;
}

void bsh_TranslateBrush(struct bsh_brush_h handle, vec3_t *translation)
{
    struct ed_brush_t *brush;
    brush = ed_GetBrushPointer(handle);
    
    if(brush)
    {
        vec3_t_add(&brush->position, &brush->position, translation);
    }
}

void bsh_UpdateDrawTriangles(struct bsh_brush_h handle)
{
//    static struct r_vertex_t draw_verts[1024];
//    static uint32_t draw_indices[1024];
    uint32_t draw_verts_count = 0;
    uint32_t draw_verts_offset = 0;
    uint32_t draw_indices_offset = 0;
    struct ed_brush_face_t *polygon;
    struct ed_brush_t *brush;
    
    vec4_t tex_coords[] = {
        vec4_t_c(0.0, 0.0, 0.0, 0.0),
        vec4_t_c(0.0, 1.0, 0.0, 0.0),
        vec4_t_c(1.0, 1.0, 0.0, 0.0),
        vec4_t_c(1.0, 0.0, 0.0, 0.0),
    };
    
    brush = ed_GetBrushPointer(handle);
    
    if(brush->face_indice_count > brush->draw_vertice_count)
    {
        brush->draw_vertice_count = brush->face_indice_count;
        brush->draw_vertices = mem_Realloc(brush->draw_vertices, sizeof(struct r_vertex_t) * brush->draw_vertice_count);
    }
    
    polygon = brush->polygons;
    while(polygon)
    {
        draw_indices_offset += (polygon->indice_count - 2) * 3;
        polygon = polygon->next;
    }
    
    if(draw_indices_offset > brush->draw_indice_count)
    {
        brush->draw_indice_count = draw_indices_offset;
        brush->draw_indices = mem_Realloc(brush->draw_indices, sizeof(uint32_t) * brush->draw_indice_count);
        
        if(brush->count)
        {
            r_FreeChunk(brush->draw_indices_chunk);
            r_FreeChunk(brush->draw_vertices_chunk);
        }
        
        brush->draw_vertices_chunk = r_AllocVerts(brush->face_indice_count);
        brush->draw_indices_chunk = r_AllocIndexes(brush->draw_indice_count);
        
        struct r_chunk_t *chunk = r_GetChunkPointer(brush->draw_indices_chunk);
        brush->start = chunk->start / sizeof(uint32_t);
        brush->count = draw_indices_offset;
        
        chunk = r_GetChunkPointer(brush->draw_vertices_chunk);
        brush->vertex_offset = chunk->start / sizeof(struct r_vertex_t);
    }
    
    draw_verts_offset = 0;
    draw_indices_offset = 0;
    
    polygon = brush->polygons;
    while(polygon)
    {
        vec3_t *vert0 = brush->vertices + polygon->indices[0];
        vec3_t *vert1 = brush->vertices + polygon->indices[1];
        vec3_t *vert2 = brush->vertices + polygon->indices[2];
        
        vec3_t edge0;
        vec3_t edge1;
        vec3_t normal;
        
        vec3_t_sub(&edge0, vert1, vert0);
        vec3_t_sub(&edge1, vert2, vert1);
        
        vec3_t_cross(&normal, &edge1, &edge0);
        vec3_t_normalize(&polygon->face_normal, &normal);
        
        polygon->draw_vertices_start = draw_verts_offset;
        for(uint32_t vert_index = 0; vert_index < polygon->indice_count; vert_index++)
        {
            vec3_t *vert = brush->vertices + polygon->indices[vert_index];
            brush->draw_vertices[draw_verts_offset + vert_index].position.x = vert->x;
            brush->draw_vertices[draw_verts_offset + vert_index].position.y = vert->y;
            brush->draw_vertices[draw_verts_offset + vert_index].position.z = vert->z;
            brush->draw_vertices[draw_verts_offset + vert_index].position.w = 1.0;
            
            brush->draw_vertices[draw_verts_offset + vert_index].tex_coords = tex_coords[vert_index];
            
            brush->draw_vertices[draw_verts_offset + vert_index].normal.x = normal.x;
            brush->draw_vertices[draw_verts_offset + vert_index].normal.y = normal.y;
            brush->draw_vertices[draw_verts_offset + vert_index].normal.z = normal.z;
            brush->draw_vertices[draw_verts_offset + vert_index].normal.w = 0.0;
        }
        
        polygon->draw_indices_start = draw_indices_offset;
        for(uint32_t indice_index = 1; indice_index < polygon->indice_count - 1;)
        {
            brush->draw_indices[draw_indices_offset] = draw_verts_offset;
            draw_indices_offset++;
            brush->draw_indices[draw_indices_offset] = draw_verts_offset + indice_index;
            draw_indices_offset++;
            indice_index++;
            brush->draw_indices[draw_indices_offset] = draw_verts_offset + indice_index;
            draw_indices_offset++;
        }
        
        polygon->draw_indices_count = draw_indices_offset - polygon->draw_indices_start;
//        polygon->draw_indices_start += brush->start;
        
        draw_verts_offset += polygon->indice_count;
        polygon = polygon->next;
    }
    
    r_FillVertsChunk(brush->draw_vertices_chunk, brush->draw_vertices, draw_verts_offset);
    r_FillIndexChunk(brush->draw_indices_chunk, brush->draw_indices, draw_indices_offset);
}

void ed_ExtrudeBrushFace(struct bsh_brush_h handle, uint32_t face_index)
{
    struct ed_brush_t *brush;    
    struct ed_brush_face_t *face;
    struct ed_brush_face_t *new_face;
    uint32_t edge_index = 0;
    brush = ed_GetBrushPointer(handle);
    
    if(brush)
    {
        face = ed_GetBrushFacePointer(handle, face_index);
        
        /* new vertices are going to be created, and those new vertices will be fully used by the extruded face, and 
        partially by the faces created from the extrusion. */
        brush->vertices = mem_Realloc(brush->vertices, sizeof(vec3_t) * (brush->vertice_count + face->indice_count));
        /* the new faces will be quads, and the number of new faces is 
        equal to the number of edges in the extruded face, which is the
        same as the number of vertices. Each new face will need four
        indices, hence the 4 */
        brush->face_indices = mem_Realloc(brush->face_indices, sizeof(uint32_t) * (brush->face_indice_count + face->indice_count * 4));
        
        for(uint32_t vertice_index = 0; vertice_index < face->indice_count; vertice_index++)
        {
            new_face = mem_Calloc(1, sizeof(struct ed_brush_face_t));
            new_face->indice_count = 4;
            new_face->indices = brush->face_indices + brush->face_indice_count;
            
            /* first two verts are the old verts of extruded face */
            new_face->indices[0] = face->indices[edge_index];
            new_face->indices[1] = face->indices[edge_index % face->indice_count];
            
            /* the rest are the new verts of the extruded face */
            new_face->indices[2] = face->indices[edge_index % face->indice_count] + brush->vertice_count;
            new_face->indices[3] = face->indices[edge_index] + brush->vertice_count;
            
            vec3_t edge0;
            vec3_t edge1;
            
            vec3_t_sub(&edge0, brush->vertices + new_face->indices[1], brush->vertices + new_face->indices[0]);
            vec3_t_sub(&edge1, brush->vertices + new_face->indices[2], brush->vertices + new_face->indices[1]);
            
            brush->last_polygon->next = new_face;
            brush->last_polygon = new_face;
            
            brush->face_indice_count += 4;
        }
        
        brush->vertice_count += face->indice_count;
    }
}

//struct bsh_polygon_t *bsh_CopyPolygons(struct bsh_polygon_t *polygons)
//{
//    struct bsh_polygon_t *copy = NULL;
//    struct bsh_polygon_t *new_polygon = NULL;
//    struct bsh_polygon_t *prev_polygon = NULL;
//    
//    while(polygons)
//    {
//        new_polygon = mem_Calloc(1, sizeof(struct bsh_polygon_t));
//        memcpy(new_polygon, polygons, sizeof(struct bsh_polygon_t));
//        new_polygon->vertices = mem_Calloc(new_polygon->vert_count, sizeof(struct bsh_vertex_t));
//        memcpy(new_polygon->vertices, polygons->vertices, sizeof(struct bsh_vertex_t) * new_polygon->vert_count);
//        
//        if(!copy)
//        {
//            copy = new_polygon;
//        }
//        
//        if(prev_polygon)
//        {
//            prev_polygon->next = new_polygon;
//        }
//        
//        prev_polygon = new_polygon;
//        polygons = polygons->next;
//    }
//    
//    return copy;
//}
//
//void bsh_DestroyPolygons(struct bsh_polygon_t *polygons)
//{
//    struct bsh_polygon_t *next;
//    
//    while(polygons)
//    {
//        next = polygons->next;
//        mem_Free(polygons->vertices);
//        mem_Free(polygons);
//        polygons = next;
//    }
//}
//
//struct bsh_polygon_t *bsh_FlipPolygons(struct bsh_polygon_t *polygons)
//{
//    struct bsh_polygon_t *polygon = polygons;
//    struct bsh_vertex_t temp;
//    while(polygon)
//    {
//        for(uint32_t vert_index = 0; vert_index < polygon->vert_count >> 1; vert_index++)
//        {
//            uint32_t other_vert_index = polygon->vert_count - vert_index - 1;
//            temp = polygon->vertices[vert_index];
//            polygon->vertices[vert_index] = polygon->vertices[other_vert_index];
//            polygon->vertices[other_vert_index] = temp;
//        }
//        
//        vec3_t_mul(&polygon->face_normal, &polygon->face_normal, -1.0);
//        polygon = polygon->next;
//    }
//    
//    return polygons;
//}
//
//uint32_t bsh_SplitEdges(struct bsh_polygon_t *polygon, vec3_t *point, vec3_t *normal, uint32_t *edges, float *times)
//{
//    uint32_t edge_vert_index = 0;
//    float dists[2];
//    float denom;
//    vec3_t plane_edge_vec;
//    
//    for(uint32_t vert_index = 0; vert_index < polygon->vert_count;)
//    {
//        uint32_t first_edge_vert = vert_index;
//        
//        vec3_t_sub(&plane_edge_vec, &polygon->vertices[vert_index % polygon->vert_count].position, point);
//        dists[0] = vec3_t_dot(normal, &plane_edge_vec);
//        vert_index++;
//        vec3_t_sub(&plane_edge_vec, &polygon->vertices[vert_index % polygon->vert_count].position, point);
//        dists[1] = vec3_t_dot(normal, &plane_edge_vec);
//        
//        if(dists[0] * dists[1] <= 0.0)
//        {
//            denom = dists[1] - dists[0];
//            if(denom)
//            {
//                denom = fabs(dists[0] / denom);
//                
//                if(denom < 1.0)
//                {
//                    edges[edge_vert_index] = first_edge_vert;
//                    times[edge_vert_index] = denom;
//                    edge_vert_index++;
//                    
//                    if(edge_vert_index > 1)
//                    {
//                        break;
//                    }
//                }
//            }
//        }
//    }
//    
//    return edge_vert_index > 1;
//}
//
//uint32_t bsh_SplitPolygon(struct bsh_polygon_t *polygon, vec3_t *point, vec3_t *normal, struct bsh_polygon_t **front, struct bsh_polygon_t **back)
//{
//    uint32_t edges[2];
//    float times[2];
//    
//    struct bsh_polygon_t *splits[2] = {NULL};
//    struct bsh_polygon_t *split;
//
//    if(bsh_SplitEdges(polygon, point, normal, edges, times))
//    {
//        uint32_t split_verts;
//        
//        if(edges[0] > edges[1])
//        {
//            split_verts = (edges[1] + polygon->vert_count) - edges[0];
//        }
//        else
//        {
//            split_verts = edges[1] - edges[0];
//        }
//        
//        uint32_t verts_left = polygon->vert_count - split_verts;
//        
//        for(uint32_t split_index = 0; split_index < 2; split_index++)
//        {
//            split = mem_Calloc(1, sizeof(struct bsh_polygon_t));
//            split->face_normal = polygon->face_normal;
//            split->vertices = mem_Calloc(split_verts + 2, sizeof(struct bsh_vertex_t));
//            
//            uint32_t vert_index = edges[split_index];
//            uint32_t split_side = 0;
//            
//            struct bsh_vertex_t *vertex0 = polygon->vertices + edges[split_index];
//            struct bsh_vertex_t *vertex1 = polygon->vertices + (edges[split_index] + 1) % polygon->vert_count;
//            float time = times[split_index];
//            
//            vec3_t_lerp(&split->vertices[0].position, &vertex0->position, &vertex1->position, time);
//            vec3_t_lerp(&split->vertices[0].normal, &vertex0->normal, &vertex1->normal, time);
//            vec3_t_lerp(&split->vertices[0].tangent, &vertex0->tangent, &vertex1->tangent, time);
//            split->vert_count++;
//            
//            while(vert_index != edges[split_index ^ 1])
//            {
//                vert_index = (vert_index + 1) % polygon->vert_count;
//                split->vertices[split->vert_count] = polygon->vertices[vert_index];
//                split->vert_count++;
//                vec3_t plane_to_vec_vert;
//                vec3_t_sub(&plane_to_vec_vert, &polygon->vertices[vert_index].position, point);
//                /* find out which split is this. If the dot product between the vec from the plane to
//                the current vertex and the normal is negative, we'll be dealing with the back split.
//                Front split will be at index 0, back split will be at 1. */
//                split_side |= vec3_t_dot(&plane_to_vec_vert, normal) < 0.0;
//            }
//            
//            vertex0 = polygon->vertices + edges[split_index ^ 1];
//            vertex1 = polygon->vertices + (edges[split_index ^ 1] + 1) % polygon->vert_count;
//            time = times[split_index ^ 1];
//            
//            vec3_t_lerp(&split->vertices[split->vert_count].position, &vertex0->position, &vertex1->position, time);
//            vec3_t_lerp(&split->vertices[split->vert_count].normal, &vertex0->normal, &vertex1->normal, time);
//            vec3_t_lerp(&split->vertices[split->vert_count].tangent, &vertex0->tangent, &vertex1->tangent, time);
//            split->vert_count++;
//            
//            splits[split_side] = split;
//            split_verts = verts_left;
//        }
//        
//        *front = splits[0];
//        *back = splits[1];
//        
//        return 1;
//    }
//    
//    return 0;
//}
//
//void bsh_SplitPolygons(struct bsh_polygon_t *polygons, vec3_t *point, vec3_t *normal, struct bsh_polygon_t **front, struct bsh_polygon_t **back, struct bsh_polygon_t **coplanar)
//{
//    struct bsh_polygon_t *polygon;
//    struct bsh_polygon_t *next_polygon;
//    struct bsh_polygon_t *front_list = NULL;
//    struct bsh_polygon_t *back_list = NULL;
//    struct bsh_polygon_t *coplanar_list = NULL;
//    struct bsh_polygon_t *front_split;
//    struct bsh_polygon_t *back_split;
//    
//    polygon = polygons;
//    
//    while(polygon)
//    {
//        next_polygon = polygon->next;
//        
//        switch(bsh_ClassifyPolygon(polygon, point, normal))
//        {
//            case BSH_POLYGON_FRONT:
//                polygon->next = front_list;
//                front_list = polygon;
//            break;
//            
//            case BSH_POLYGON_BACK:
//                polygon->next = back_list;
//                back_list = polygon;
//            break;
//            
//            case BSH_POLYGON_COPLANAR:
//                polygon->next = coplanar_list;
//                coplanar_list = polygon;
//            break;
//            
//            case BSH_POLYGON_STRADDLING:
//                bsh_SplitPolygon(polygon, point, normal, &front_split, &back_split);
//                
//                polygon->next = NULL;
//                bsh_DestroyPolygons(polygon);
//                
//                front_split->next = front_list;
//                front_list = front_split;
//                
//                back_split->next = back_list;
//                front_list = back_split;
//            break;
//        }
//        
//        polygon = next_polygon;
//    }
//    
//    *front = front_list;
//    *back = back_list;
//    *coplanar = coplanar_list;
//}
//
//struct bsh_polygon_t *bsh_BestSplitter(struct bsh_polygon_t **polygons)
//{
//    struct bsh_polygon_t *splitter;
//    struct bsh_polygon_t *prev_splitter = NULL;
//    struct bsh_polygon_t *polygon;
//    struct bsh_polygon_t *best_splitter = NULL;
//    struct bsh_polygon_t *best_splitter_prev = NULL;
//    uint32_t best_split_count = 0xffffffff;
//    uint32_t cur_split_count;
//    uint32_t edges[2];
//    float times[2];
//    
//    splitter = *polygons;
//    
//    while(splitter)
//    {
//        polygon = splitter->next;
//        cur_split_count = 0;
//        while(polygon)
//        {
//            if(bsh_SplitEdges(polygon, &splitter->vertices[0].position, &splitter->face_normal, edges, times))
//            {
//                cur_split_count++;
//            }
//            polygon = polygon->next;
//        }
//        
//        if(cur_split_count < best_split_count)
//        {
//            best_split_count = cur_split_count;
//            best_splitter = splitter;
//            best_splitter_prev = prev_splitter;
//        }
//        
//        prev_splitter = splitter;
//        splitter = splitter->next;
//    }
//    
//    if(best_splitter_prev)
//    {
//        best_splitter_prev->next = best_splitter->next;
//    }
//    else
//    {
//        /* polygon is the first in the list, so update the list pointer */
//        *polygons = best_splitter->next;
//    }
//    
//    best_splitter->next = NULL;
//    
//    return best_splitter;
//}
//
//uint32_t bsh_ClassifyPoint(vec3_t *point, vec3_t *plane_point, vec3_t *plane_normal)
//{
//    vec3_t plane_point_vec;
//    float dist;
//    vec3_t_sub(&plane_point_vec, point, plane_point);
//    dist = vec3_t_dot(&plane_point_vec, plane_normal);
//    
//    if(dist > 0.0)
//    {
//        return BSH_POINT_FRONT;
//    }
//    else if(dist < 0.0)
//    {
//        return BSH_POINT_BACK;
//    }
//    
//    return BSH_POINT_COPLANAR;
//}
//
//uint32_t bsh_ClassifyPolygon(struct bsh_polygon_t *polygon, vec3_t *point, vec3_t *normal)
//{
//    int32_t last_classification = BSH_POINT_COPLANAR;
//    
//    for(uint32_t vert_index = 0; vert_index < polygon->vert_count; vert_index++)
//    {
//        int32_t classification = bsh_ClassifyPoint(&polygon->vertices[vert_index].position, point, normal);
//        
//        if(classification != last_classification)
//        {
//            if(abs(classification - last_classification) == 1)
//            {
//                /* if one is BSH_POINT_FRONT and the other is BSH_POINT_BACK */
//                return BSH_POLYGON_STRADDLING;
//            }
//            
//            if(classification != BSH_POINT_COPLANAR)
//            {
//                last_classification = classification;
//            }
//        }
//    }
//    
//    return last_classification;
//}
//
//struct bsh_bsp_t *bsh_BspFromPolygonsRecursive(struct bsh_polygon_t *polygons)
//{
//    struct bsh_polygon_t *splitter;
//    struct bsh_polygon_t *next_polygon;
//    struct bsh_polygon_t *front = NULL;
//    struct bsh_polygon_t *back = NULL; 
//    struct bsh_polygon_t *coplanar = NULL;
////    struct bsh_polygon_t *anti_coplanar = NULL;
//    struct bsh_bsp_t *bsp;
//    
//    bsp = mem_Calloc(1, sizeof(struct bsh_bsp_t));
//    splitter = bsh_BestSplitter(&polygons);
//    
//    if(polygons)
//    {
//        bsh_SplitPolygons(polygons, &splitter->vertices[0].position, &splitter->face_normal, &front, &back, &coplanar);
//    }
//    
//    bsp->splitter = splitter;
//    splitter->next = coplanar;
//    
//    if(front)
//    {
//        bsp->front = bsh_BspFromPolygonsRecursive(front);
//    }
//    else
//    {
//        bsp->front = mem_Calloc(1, sizeof(struct bsh_bsp_t));
//        bsp->front->type = BSH_BSP_TYPE_EMPTY;
//    }
//    
//    if(back)
//    {
//        bsp->back = bsh_BspFromPolygonsRecursive(back);
//    }
//    else
//    {
//        bsp->back = mem_Calloc(1, sizeof(struct bsh_bsp_t));
//        bsp->back->type = BSH_BSP_TYPE_SOLID;
//    }
//    
//    return bsp;
//}
//
//struct bsh_bsp_t *bsh_BspFromPolygons(struct bsh_polygon_t *polygons)
//{
//    struct bsh_polygon_t *polygons_copy = bsh_CopyPolygons(polygons);
//    return bsh_BspFromPolygonsRecursive(polygons_copy);
//}
//
//void bsh_DestroyBsp(struct bsh_bsp_t *bsp)
//{
//    if(bsp)
//    {
//        bsh_DestroyPolygons(bsp->splitter);
//        bsh_DestroyBsp(bsp->back);
//        bsh_DestroyBsp(bsp->front);
//        mem_Free(bsp);
//    }
//}
//
//struct bsh_bsp_t *bsh_IncrementalSetOpRecursive(struct bsh_bsp_t *bsp, struct bsh_polygon_t *polygons, uint32_t op)
//{
//    
//    struct bsh_polygon_t *front;
//    struct bsh_polygon_t *back;
//    struct bsh_polygon_t *coplanar;
//    
//    if(op == BSH_SET_OP_SUBTRACTION)
//    {
//        polygons = bsh_FlipPolygons(polygons);
//        op = BSH_SET_OP_INTERSECTION;
//    }
//    
//    if(bsp->back == bsp->front)
//    {
//        switch(op)
//        {
//            case BSH_SET_OP_UNION:
//                if(bsp->type == BSH_BSP_TYPE_EMPTY)
//                {
//                    return bsh_BspFromPolygonsRecursive(polygons);
//                }
//            break;
//            
//            case BSH_SET_OP_INTERSECTION:
//                if(bsp->type == BSH_BSP_TYPE_SOLID)
//                {
//                    return bsh_BspFromPolygonsRecursive(polygons);
//                }
//            break;
//        }
//        
//        return bsp;
//    }
//    else
//    {
//        
//    }
//}
//
//struct bsh_bsp_t *bsh_IncrementalSetOp(struct bsh_bsp_t *bsp, struct bsh_polygon_t *polygons, uint32_t op)
//{
//    struct bsh_polygon_t *polygons_copy = bsh_CopyPolygons(polygons);
//    return bsh_IncrementalSetOpRecursive(bsp, polygons_copy, op);
//}
//
struct stack_list_t *bsh_GetBrushList()
{
    return &bsh_brushes;
}





