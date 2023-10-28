#version 450
#pragma stage : vert

layout(location = 0) in vec3 in_Position;

layout(set = 0, binding = 0) uniform UBO
{
	mat4 m_ProjView;
	vec4 m_LightDirection;
} ubo;

layout(location = 0) out vec3 outPosition;

void main() 
{
    outPosition = in_Position;
    outPosition *= -1.0;
    gl_Position = ubo.m_ProjView * vec4(in_Position, 1.0);
}

#version 450
#pragma stage : frag

layout (location = 0) in vec3 in_Position;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform samplerCube SamplerCubeMap;

void main() 
{
    outColor = texture(SamplerCubeMap, in_Position);
}