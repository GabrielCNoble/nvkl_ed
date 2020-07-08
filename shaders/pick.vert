#version 330
#extension GL_ARB_enhanced_layouts : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 r_VertexPosition;



layout(push_constant) uniform r_Input
{
    mat4 r_ModelViewProjectionMatrix;
    int r_ObjIndex;
};


layout(location = 0) out flat int index;

void main()
{
    gl_Position = r_ModelViewProjectionMatrix * vec4(r_VertexPosition, 1.0);
    index = r_ObjIndex;     
}
