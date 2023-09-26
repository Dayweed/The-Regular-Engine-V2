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

void main() 
{
    gl_Position = ubo.m_ProjView * push.m_Model * vec4(in_Position, 1.0);
    outColor = in_Color;
}