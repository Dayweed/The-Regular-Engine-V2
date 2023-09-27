#version 450

layout(location = 0) in vec4  inWeights;
layout(location = 1) in ivec4 inBones;
layout(location = 2) in vec3  inPosition;
layout(location = 3) in vec3  inNormal;
layout(location = 4) in vec3  inTangent;
layout(location = 5) in vec2  inUV;

layout(push_constant) uniform Push
{
	mat4 m_Model;
} push;

layout(set = 0, binding = 0) uniform UBO
{
	mat4 m_ProjView;
    mat4 L2W[256];
} ubo;

layout(location = 0) out struct 
{
    vec4 Color;
    vec2 UV;
} Out;

void main() 
{
	mat4 L2W = ubo.L2W[inBones.x] * inWeights.x + ubo.L2W[inBones.y] * inWeights.y 
             + ubo.L2W[inBones.z] * inWeights.z + ubo.L2W[inBones.w] * inWeights.w;
	Out.UV = inUV;
	Out.Color = vec4(1);
	gl_Position = ubo.m_ProjView * L2W * vec4(inPosition.xyz, 1.0);
}