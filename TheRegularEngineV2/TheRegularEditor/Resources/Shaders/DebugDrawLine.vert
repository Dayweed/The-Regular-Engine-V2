#version 450

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push
{
	mat4 m_Model;
} push;

layout(set = 0, binding = 0) uniform UBO
{
	mat4 m_ProjView;
	vec4 m_LightDirection;
} ubo;

vec3 positions[2] = vec3[](
    vec3(0.0, 0.0, 0.0),
    vec3(1.0, 1.0, 1.0)
);
vec4 colors = vec4(1.0, 1.0, 1.0, 1.0);

void main() 
{
    gl_Position = ubo.m_ProjView * push.m_Model * vec4(positions[gl_VertexIndex], 1.0);
    outColor = colors;
}