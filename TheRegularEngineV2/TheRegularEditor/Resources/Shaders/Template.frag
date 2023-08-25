#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push
{
	mat4 m_Model; // Model
	mat4 m_NormalMat; //Normal mat for light
} push;

void main() 
{
    outColor = vec4(inColor, 1.0);
}