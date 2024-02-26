#version 450
#pragma stage : vert

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec3 inColor;
layout(location = 5) in vec2 inTexCoord;

layout(push_constant) uniform Push
{
	mat4 m_Model;
} push;

layout (binding = 0) uniform UBO 
{
	mat4 view;
    mat4 projection;
} ubo;

void main() 
{
    gl_Position = ubo.projection * ubo.view * push.m_Model * vec4(inPosition, 1.0);
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;

void main() 
{
    //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}