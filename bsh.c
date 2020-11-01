#include "bsh.h"
#include "neighbor/lib/dstuff/ds_mem.h"
//#include "neighbor/r_draw.h"
#include "neighbor/r.h"
#include <string.h>
#include <stdlib.h>

//struct bsh_brush_t *bsh_brushes = NULL;
//struct bsh_brush_t *bsh_last_brush = NULL;

struct stack_list_t bsh_brushes;
//uint32_t ed_brush_count = 0;

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
uint32_t ed_total_vertice_count = 0;
uint32_t ed_total_face_count = 0;
uint32_t ed_total_indice_count = 0;

#define ED_MAX_BRUSH_EDGES 1024
struct ed_brush_edge_t *ed_brush_edges;

uint32_t *bsh_old_face_indices = NULL;
uint32_t bsh_old_face_indices_count = 0;
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

void bsh_Init()
{
    bsh_brushes = create_stack_list(sizeof(struct ed_brush_t), 512);
    ed_brush_edges = mem_Calloc(ED_MAX_BRUSH_EDGES, sizeof(struct ed_brush_edge_t));
}

struct bsh_brush_h bsh_CreateBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t type)
{
    struct ed_brush_t *brush;
    struct bsh_brush_h handle;
    
    handle.index = add_stack_list_element(&bsh_brushes, NULL);
    brush = get_stack_list_element(&bsh_brushes, handle.index);
    
    memset(brush, 0, sizeof(struct ed_brush_t));
    
    brush->type = type;
    brush->position = *position;
    brush->scale = *scale;
    brush->orientation = *orientation;
    
    return handle;
}

struct bsh_brush_h bsh_CreateCubeBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale)
{
    struct ed_brush_t *brush;
    struct bsh_brush_h handle;
    struct ed_brush_face_t *face;
    struct bsh_vertex_t *vertices = bsh_cube_brush_verts;
    struct r_chunk_t *chunk;
    uint32_t draw_vert_index = 0;
    uint32_t indice_offset = 0;
//    static struct r_vertex_t draw_verts[36];
    
    handle = bsh_CreateBrush(position, orientation, scale, BSH_BRUSH_TYPE_CUBE);
    brush = ed_GetBrushPointer(handle);
    
    brush->max_face_count = 6;
    brush->face_count = brush->max_face_count;
    brush->faces = mem_Calloc(brush->max_face_count, sizeof(struct ed_brush_face_t));

    brush->max_indice_count = bsh_cube_brush_indice_count;
    brush->indices = mem_Calloc(brush->max_indice_count, sizeof(uint32_t));
    memcpy(brush->indices, bsh_cube_brush_indices, sizeof(uint32_t) * bsh_cube_brush_indice_count);
    
    brush->max_vertice_count = bsh_cube_brush_vert_count;
    brush->vertice_count = brush->max_vertice_count;
    brush->vertices = mem_Calloc(brush->max_vertice_count, sizeof(vec3_t));
    for(uint32_t vert_index = 0; vert_index < bsh_cube_brush_vert_count; vert_index++)
    {
        brush->vertices[vert_index] = bsh_cube_brush_verts[vert_index];
        brush->vertices[vert_index].x *= scale->x;
        brush->vertices[vert_index].y *= scale->y;
        brush->vertices[vert_index].z *= scale->z;
    }
    
    for(uint32_t face_index = 0; face_index < brush->face_count; face_index++)
    {       
        face = brush->faces + face_index;
        face->indices = brush->indices + brush->indice_count;
        face->indice_count = 4;
        brush->indice_count += 4;
    }

    ed_UpdateBrushEdges(handle);
    bsh_UpdateBrushDrawData(handle);
    
    return handle;
}

struct bsh_brush_h bsh_CreateCylinderBrush(vec3_t *position, mat3_t *orientation, vec3_t *scale, uint32_t vert_count)
{
    return BSH_INVALID_BRUSH_HANDLE;
}

struct bsh_brush_h ed_CopyBrush(struct bsh_brush_h handle)
{
    struct ed_brush_t *brush = ed_GetBrushPointer(handle);
    struct ed_brush_t *new_brush;
    struct bsh_brush_h new_handle = BSH_INVALID_BRUSH_HANDLE;
    
    if(brush)
    {
        new_handle = bsh_CreateBrush(&brush->position, &brush->orientation, &brush->scale, brush->type);
        new_brush = ed_GetBrushPointer(new_handle);
        
        memcpy(new_brush, brush, sizeof(struct ed_brush_t));
        
        new_brush->max_draw_indice_count = 0;
        new_brush->draw_indice_count = 0;
        new_brush->draw_indices = NULL;
        new_brush->max_draw_vertice_count = 0;
        new_brush->draw_vertice_count = 0;
        new_brush->draw_vertices = NULL;
        
        new_brush->indices = mem_Calloc(new_brush->max_indice_count, sizeof(uint32_t));
        memcpy(new_brush->indices, brush->indices, sizeof(uint32_t) * new_brush->indice_count);
        new_brush->edges = mem_Calloc(new_brush->max_edge_count, sizeof(struct ed_brush_edge_t));
        memcpy(new_brush->edges, brush->edges, sizeof(struct ed_brush_edge_t) * new_brush->edge_count);
        new_brush->vertices = mem_Calloc(new_brush->max_vertice_count, sizeof(vec3_t));
        memcpy(new_brush->vertices, brush->vertices, sizeof(vec3_t) * new_brush->vertice_count);
        new_brush->faces = mem_Calloc(new_brush->max_face_count, sizeof(struct ed_brush_face_t));
        memcpy(new_brush->faces, brush->faces, sizeof(struct ed_brush_face_t) * new_brush->max_face_count);
        
        new_brush->indice_count = 0;
        for(uint32_t face_index = 0; face_index < new_brush->face_count; face_index++)
        {
            struct ed_brush_face_t *face = new_brush->faces + face_index;
            face->indices = new_brush->indices + new_brush->indice_count;
            new_brush->indice_count += face->indice_count;
        }
        
        bsh_UpdateBrushDrawData(new_handle);
    }
    
    return new_handle;
}

void bsh_DestroyBrush(struct bsh_brush_h handle)
{
    struct bsh_polygon_t *next_polygon;
    struct ed_brush_t *brush;
    
    brush = ed_GetBrushPointer(handle);
    
    if(brush)
    {        
        mem_Free(brush->vertices);
        mem_Free(brush->draw_vertices);
        mem_Free(brush->draw_indices);
        mem_Free(brush->indices);
        mem_Free(brush->edges);
        mem_Free(brush->faces);
        
        r_FreeChunk(brush->draw_vertices_chunk);
        r_FreeChunk(brush->draw_indices_chunk);
        
        brush->type = BSH_BRUSH_TYPE_NONE;
        remove_stack_list_element(&bsh_brushes, handle.index);
    }
}

void ed_DestroyAllBrushes()
{
    for(uint32_t brush_index = 0; brush_index < bsh_brushes.cursor; brush_index++)
    {
        bsh_DestroyBrush(BSH_BRUSH_HANDLE(brush_index));
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
    struct ed_brush_face_t *face = NULL;
    uint32_t polygon_index = 0;
    brush = ed_GetBrushPointer(handle);
    if(brush)
    {
        face = brush->faces + index;
    }
    
    return face;
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

void bsh_RotateBrush(struct bsh_brush_h handle, mat3_t *rotation)
{
    struct ed_brush_t *brush;
    brush = ed_GetBrushPointer(handle);
    
    if(brush)
    {
        mat3_t_mul(&brush->orientation, &brush->orientation, rotation);
    }
}

void ed_UpdateBrushEdges(struct bsh_brush_h handle)
{
    struct ed_brush_face_t *face;
    struct ed_brush_t *brush;
    uint32_t edge_count = 0;
    
    brush = ed_GetBrushPointer(handle);
    
    if(brush)
    {
        for(uint32_t face_index = 0; face_index < brush->face_count; face_index++)
        {
            face = brush->faces + face_index;
            
            for(uint32_t indice_index = 0; indice_index < face->indice_count; indice_index++)
            {
                uint32_t first_indice = face->indices[indice_index];
                uint32_t second_indice = face->indices[(indice_index + 1) % face->indice_count];
                
                struct ed_brush_edge_t *edge = ed_brush_edges + edge_count;
                
                edge->indices[0] = first_indice;
                edge->indices[1] = second_indice;
                edge->face_a = face_index;
                edge_count++;
            }
        }
        
        for(uint32_t first_edge_index = 0; first_edge_index < edge_count; first_edge_index++)
        {
            struct ed_brush_edge_t *first_edge = ed_brush_edges + first_edge_index;
            
            for(uint32_t second_edge_index = first_edge_index + 1; second_edge_index < edge_count; second_edge_index++)
            {
                struct ed_brush_edge_t *second_edge = ed_brush_edges + second_edge_index;
                
                if(first_edge->indices[0] == second_edge->indices[0] && first_edge->indices[1] == second_edge->indices[1] ||
                   first_edge->indices[1] == second_edge->indices[0] && first_edge->indices[0] == second_edge->indices[1])
                {
                    /* duplicate edge, store the face that generated this duplicate edge as the second bounded face
                    for the current edge, and then pull the last edge in the list to its place */
                        
                    first_edge->face_b = second_edge->face_a;
                    ed_brush_edges[second_edge_index] = ed_brush_edges[edge_count - 1];
                    edge_count--;
                    second_edge_index--;
                }
            }
        }
        
        if(edge_count > brush->max_edge_count)
        {
            brush->max_edge_count = edge_count;
            brush->edges = mem_Realloc(brush->edges, sizeof(struct ed_brush_edge_t) * brush->max_edge_count);
        }
        
        brush->edge_count = edge_count;
        memcpy(brush->edges, ed_brush_edges, sizeof(struct ed_brush_edge_t) * brush->edge_count);
    }
}

void bsh_UpdateBrushDrawData(struct bsh_brush_h handle)
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
    
    if(brush->indice_count > brush->max_draw_vertice_count)
    {
        brush->max_draw_vertice_count = brush->indice_count;
        brush->draw_vertices = mem_Realloc(brush->draw_vertices, sizeof(struct r_vertex_t) * brush->max_draw_vertice_count);
    }

    for(uint32_t face_index = 0; face_index < brush->face_count; face_index++)
    {
        struct ed_brush_face_t *face = brush->faces + face_index;
        draw_indices_offset += (face->indice_count - 2) * 3;
    }
    
    draw_indices_offset += brush->edge_count * 2;
    
    if(draw_indices_offset > brush->max_draw_indice_count)
    {
        brush->max_draw_indice_count = draw_indices_offset;
        brush->draw_indices = mem_Realloc(brush->draw_indices, sizeof(uint32_t) * brush->max_draw_indice_count);
        
        if(brush->draw_indice_count)
        {
            r_FreeChunk(brush->draw_indices_chunk);
            r_FreeChunk(brush->draw_vertices_chunk);
        }
        
        brush->draw_vertices_chunk = r_AllocVerts(brush->indice_count);
        brush->draw_indices_chunk = r_AllocIndexes(brush->max_draw_indice_count);
        
        struct r_chunk_t *chunk = r_GetChunkPointer(brush->draw_indices_chunk);
        brush->draw_indice_start = chunk->start / sizeof(uint32_t);
//        brush->count = draw_indices_offset;
        
        chunk = r_GetChunkPointer(brush->draw_vertices_chunk);
        brush->vertex_offset = chunk->start / sizeof(struct r_vertex_t);
    }
    
    brush->draw_indice_count = draw_indices_offset;
    brush->draw_vertice_count = brush->indice_count;
    
    draw_verts_offset = 0;
    draw_indices_offset = 0;
    
    for(uint32_t face_index = 0; face_index < brush->face_count; face_index++)
    {
        struct ed_brush_face_t *face = brush->faces + face_index;
        
        vec3_t *vert0 = brush->vertices + face->indices[0];
        vec3_t *vert1 = brush->vertices + face->indices[1];
        vec3_t *vert2 = brush->vertices + face->indices[2];
        
        vec3_t edge0;
        vec3_t edge1;
        vec3_t normal;
        
        vec3_t_sub(&edge0, vert1, vert0);
        vec3_t_sub(&edge1, vert2, vert1);
        
        vec3_t_cross(&normal, &edge1, &edge0);
        vec3_t_normalize(&face->normal, &normal);
        
        face->draw_vertices_start = draw_verts_offset;
        for(uint32_t vert_index = 0; vert_index < face->indice_count; vert_index++)
        {
            vec3_t *vert = brush->vertices + face->indices[vert_index];
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
        face->draw_indices_start = draw_indices_offset;
        for(uint32_t indice_index = 1; indice_index < face->indice_count - 1;)
        {
            brush->draw_indices[draw_indices_offset] = draw_verts_offset;
            draw_indices_offset++;
            brush->draw_indices[draw_indices_offset] = draw_verts_offset + indice_index;
            draw_indices_offset++;
            indice_index++;
            brush->draw_indices[draw_indices_offset] = draw_verts_offset + indice_index;
            draw_indices_offset++;
        }
        
        face->draw_indices_count = draw_indices_offset - face->draw_indices_start;        
        draw_verts_offset += face->indice_count;
    }
    brush->face_draw_indice_count = draw_indices_offset;
    
    for(uint32_t edge_index = 0; edge_index < brush->edge_count; edge_index++)
    {
        struct ed_brush_edge_t *edge = brush->edges + edge_index;
        edge->draw_indices_start = draw_indices_offset;
        brush->draw_indices[draw_indices_offset] = edge->indices[0];
        draw_indices_offset++;
        brush->draw_indices[draw_indices_offset] = edge->indices[1];
        draw_indices_offset++;
    }
    brush->edge_draw_indice_count = draw_indices_offset - brush->face_draw_indice_count;
    
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
        face = brush->faces + face_index;
        
        uint32_t new_face_count = brush->face_count + face->indice_count;
        if(new_face_count > brush->max_face_count)
        {
            brush->max_face_count = new_face_count;
            brush->faces = mem_Realloc(brush->faces, sizeof(struct ed_brush_face_t) * new_face_count);
            face = brush->faces + face_index;
        }
        
        /* the new faces will be quads, and the number of new faces is equal to the number of 
        edges in the extruded face, which is the same as the number of vertices. Each new face 
        will need four indices, hence the 4 */
        uint32_t new_indice_count = brush->indice_count + face->indice_count * 4;
        if(new_indice_count > brush->max_indice_count)
        {
            brush->max_indice_count = new_indice_count;
            brush->indices = mem_Realloc(brush->indices, sizeof(uint32_t) * new_indice_count);
            
            uint32_t indice_offset = 0;
            for(uint32_t face_index = 0; face_index < brush->face_count; face_index++)
            {
                struct ed_brush_face_t *face = brush->faces + face_index;
                face->indices = brush->indices + indice_offset;
                indice_offset += face->indice_count;
            }
        }
        
        if(face->indice_count > bsh_old_face_indices_count)
        {
            bsh_old_face_indices_count = face->indice_count;
            bsh_old_face_indices = mem_Realloc(bsh_old_face_indices, sizeof(uint32_t) * bsh_old_face_indices_count);
        }
        
        memcpy(bsh_old_face_indices, face->indices, sizeof(uint32_t) * face->indice_count);
        
        /* new vertices are going to be created, and those new vertices will be fully used by the extruded face, and 
        partially by the faces created from the extrusion. */
        uint32_t new_vertice_count = brush->vertice_count + face->indice_count;
        if(new_vertice_count > brush->max_vertice_count)
        {
            brush->vertices = mem_Realloc(brush->vertices, sizeof(vec3_t) * new_vertice_count);
        }
        
        for(uint32_t vertice_index = 0; vertice_index < face->indice_count; vertice_index++)
        {
            brush->vertices[brush->vertice_count + vertice_index] = brush->vertices[face->indices[vertice_index]];
            face->indices[vertice_index] = brush->vertice_count + vertice_index;
        }
                
        for(uint32_t vertice_index = 0; vertice_index < face->indice_count; vertice_index++)
        {
            struct ed_brush_face_t *new_face = brush->faces + brush->face_count;
            brush->face_count++;
            
            new_face->indice_count = 4;
            new_face->indices = brush->indices + brush->indice_count;
            
            /* first two verts are the old verts of extruded face */
            new_face->indices[0] = bsh_old_face_indices[vertice_index];
            new_face->indices[1] = bsh_old_face_indices[(vertice_index + 1) % face->indice_count];
            
            /* the rest are the new verts of the extruded face */
            new_face->indices[2] = face->indices[(vertice_index + 1) % face->indice_count];
            new_face->indices[3] = face->indices[vertice_index];
            
            vec3_t edge0;
            vec3_t edge1;
            
            vec3_t_sub(&edge0, brush->vertices + new_face->indices[1], brush->vertices + new_face->indices[0]);
            vec3_t_sub(&edge1, brush->vertices + new_face->indices[2], brush->vertices + new_face->indices[1]);
            
            brush->indice_count += 4;
        }
        
        brush->vertice_count += face->indice_count;
        ed_UpdateBrushEdges(handle);
        bsh_UpdateBrushDrawData(handle);
    }
}

void ed_SerializeBrushes(void **buffer, uint32_t *buffer_size)
{
    uint32_t size = 0;
    struct ed_brush_header_t *header;
    char *output;
    
    size = sizeof(struct ed_brush_header_t);
    size += (sizeof(struct ed_brush_data_t) + sizeof(struct ed_brush_vertice_data_t)) * bsh_brushes.used;
    
    for(uint32_t brush_index = 0; brush_index < bsh_brushes.cursor; brush_index++)
    {
        struct ed_brush_t *brush = ed_GetBrushPointer(BSH_BRUSH_HANDLE(brush_index));
        if(brush)
        {
            size += sizeof(struct ed_brush_face_data_t) * brush->face_count;
            size += sizeof(uint32_t) * brush->indice_count;
            size += sizeof(vec3_t) * brush->vertice_count;
        }
    }
    
    output = mem_Calloc(1, size);
    
    *buffer = output;
    *buffer_size = size;
    
    header = (struct ed_brush_header_t *)output;
    output += sizeof(struct ed_brush_header_t);
    
    header->brush_count = bsh_brushes.used;
    
    for(uint32_t brush_index = 0; brush_index < bsh_brushes.cursor; brush_index++)
    {
        struct ed_brush_t *brush = ed_GetBrushPointer(BSH_BRUSH_HANDLE(brush_index));
        
        if(brush)
        {
            struct ed_brush_data_t *brush_data = output;
            output += sizeof(struct ed_brush_data_t);
            
            brush_data->face_count = brush->face_count;
            brush_data->indice_count = brush->indice_count;
            brush_data->vertice_count = brush->vertice_count;
            brush_data->type = brush->type;
            brush_data->position = brush->position;
            brush_data->size = brush->scale;
            brush_data->orientation = brush->orientation;
            
            
            for(uint32_t face_index = 0; face_index < brush->face_count; face_index++)
            {
                struct ed_brush_face_t *face = brush->faces + face_index;
                struct ed_brush_face_data_t *face_data = output;
                output += sizeof(struct ed_brush_face_data_t) + sizeof(uint32_t) * face->indice_count;
                
                strcpy(face_data->material, "default");
                memcpy(face_data->indices, face->indices, sizeof(uint32_t) * face->indice_count);
                face_data->uv_rotation = 0.0;
                face_data->u_scale = 1.0;
                face_data->v_scale = 1.0;
                face_data->indice_count = face->indice_count;
                face_data->tex_coord_mode = ED_TEX_COORD_MODE_WORLD;
            }
            
            struct ed_brush_vertice_data_t *vertice_data = output;
            output += sizeof(struct ed_brush_vertice_data_t) + sizeof(vec3_t) * brush->vertice_count;
            memcpy(vertice_data->vertices, brush->vertices, sizeof(vec3_t) * brush->vertice_count);
        }
    }
}

void ed_UnserializeBrushes(void *buffer)
{
    char *input = buffer;
    struct ed_brush_header_t *header = input;
    input += sizeof(struct ed_brush_header_t);
    
    for(uint32_t brush_index = 0; brush_index < header->brush_count; brush_index++)
    {
        struct ed_brush_data_t *brush_data = input;
        input += sizeof(struct ed_brush_data_t);
        
        struct bsh_brush_h handle = bsh_CreateBrush(&brush_data->position, &brush_data->orientation, &brush_data->size, brush_data->type);
        struct ed_brush_t *brush = ed_GetBrushPointer(handle);
        
        brush->type = brush_data->type;
        
        brush->vertice_count = brush_data->vertice_count;
        brush->max_vertice_count = brush_data->vertice_count;
        
        brush->indice_count = brush_data->indice_count;
        brush->max_indice_count = brush_data->indice_count;
        
        brush->face_count = brush_data->face_count;
        brush->max_face_count = brush_data->face_count;
        
        
        brush->vertices = mem_Calloc(brush_data->vertice_count, sizeof(vec3_t));
        brush->indices = mem_Calloc(brush_data->indice_count, sizeof(uint32_t));
        brush->faces = mem_Calloc(brush_data->face_count, sizeof(struct ed_brush_face_t));
        
        uint32_t indice_offset = 0;
        for(uint32_t face_index = 0; face_index < brush_data->face_count; face_index++)
        {
            struct ed_brush_face_data_t *face_data = (struct ed_brush_face_data_t *)input;
            input += sizeof(struct ed_brush_face_data_t) + face_data->indice_count * sizeof(uint32_t);
            struct ed_brush_face_t *face = brush->faces + face_index;
            
            face->indice_count = face_data->indice_count;
            face->tex_coord_mode = face_data->tex_coord_mode;
            face->indices = brush->indices + indice_offset;
            
            indice_offset += face->indice_count;
            memcpy(face->indices, face_data->indices, sizeof(uint32_t) * face_data->indice_count);
        }
        
        struct ed_brush_vertice_data_t *vertice_data = (struct ed_brush_vertice_data_t *)input;
        input += sizeof(struct ed_brush_vertice_data_t) + sizeof(vec3_t) * brush_data->vertice_count;
        memcpy(brush->vertices, vertice_data->vertices, sizeof(vec3_t) * brush_data->vertice_count);
        ed_UpdateBrushEdges(handle);
        bsh_UpdateBrushDrawData(handle);
    }
}


struct stack_list_t *bsh_GetBrushList()
{
    return &bsh_brushes;
}





