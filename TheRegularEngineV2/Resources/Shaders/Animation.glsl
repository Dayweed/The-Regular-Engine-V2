#version 450

#pragma stage : vert

layout(location = 0) in vec4  inWeights;
layout(location = 1) in ivec4 inBones;
layout(location = 2) in vec3  inPosition;
layout(location = 3) in vec3  inNormal;
layout(location = 4) in vec3  inTangent;
layout(location = 5) in vec2  inUV;

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


#version 450

#pragma stage : frag

layout(location = 0) in struct 
{
	vec4 Color;
	vec2 UV;
} In;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D Diffuse;
layout(set = 0, binding = 2) uniform sampler2D DiffuseAO;
layout(set = 0, binding = 3) uniform sampler2D NormalMap;
layout(set = 0, binding = 4) uniform sampler2D Specular;
layout(set = 0, binding = 5) uniform sampler2D Glossiness;

void main() 
{
   outColor = In.Color * texture(Diffuse, In.UV);
}