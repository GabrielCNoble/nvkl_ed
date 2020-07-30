#version 330
#extension GL_ARB_enhanced_layouts : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 r_VertexPosition;



layout(push_constant) uniform r_Input
{
    layout(offset = 0)mat4 r_ModelViewProjectionMatrix;
    layout(offset = 64) int r_ObjIndex;
};


layout(location = 0) out flat int index;

void main()
{
    gl_Position = r_ModelViewProjectionMatrix * r_VertexPosition;
    index = r_ObjIndex;     
}
