#include "ui_ed.h"
#include "neighbor/lib/imgui/imgui.h"
//#include "lib/imgui/ImGuizmo.h"

#ifdef __cplusplus
extern "C"
{
#endif

void ui_ed_BeginFrame()
{
//    ImGuizmo::BeginFrame();
}

void ui_ed_EndFrame()
{
    
}

void ui_ed_SetRect(float x, float y, float w, float h)
{
//    ImGuizmo::SetRect(x, y, w, h);
}

void ui_ed_Manipulate(mat4_t *view_matrix, mat4_t *projection_matrix, uint32_t operation, uint32_t mode, mat4_t *gizmo_transform, mat4_t *delta_transform)
{
//    ImGuizmo::OPERATION op;
//    ImGuizmo::MODE mod;
//    
//    switch(operation)
//    {
//        case TRANSFORM_OP_TRANSLATION:
//            op = ImGuizmo::TRANSLATE;
//        break;
//        
//        case TRANSFORM_OP_ROTATION:
//            op = ImGuizmo::ROTATE;
//        break;
//        
//        case TRANSFORM_OP_SCALE:
//            op = ImGuizmo::SCALE;
//        break;
//        
//        case TRANSFORM_OP_BOUNDS:
//            op = ImGuizmo::BOUNDS;
//        break;
//    }
//    
//    switch(mode)
//    {
//        case TRANSFORM_MODE_LOCAL:
//            mod = ImGuizmo::LOCAL;
//        break;
//        
//        case TRANSFORM_MODE_WORLD:
//            mod = ImGuizmo::WORLD;
//        break;
//    }
//    
//    ImGuizmo::Manipulate((float *)view_matrix->comps, (float *)projection_matrix->comps, op, mod, (float *)gizmo_transform->comps, (float *)delta_transform->comps, NULL, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
