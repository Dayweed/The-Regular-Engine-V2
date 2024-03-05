#version 450
#pragma stage : vert

layout(location = 0) in vec3  inPosition;
layout(location = 1) in vec3  inNormal;
layout(location = 2) in vec3  inTangent;
layout(location = 3) in vec3  inBitangent;
layout(location = 4) in vec3  inColor;
layout(location = 5) in vec2  inUV;
layout(location = 6) in vec4  inWeights;
layout(location = 7) in ivec4 inBones;

layout(push_constant) uniform Push
{
	mat4 m_Model;
} push;

layout (binding = 0) uniform UBO 
{
	mat4 view;
	mat4 projection;
} ubo;

layout (set = 0, binding = 8) uniform UBOAnimation
{
	mat4 L2W[256];
} AnimationUBO;

void main() 
{
	mat4 L2W = AnimationUBO.L2W[inBones.x] * inWeights.x + AnimationUBO.L2W[inBones.y] * inWeights.y 
			 + AnimationUBO.L2W[inBones.z] * inWeights.z + AnimationUBO.L2W[inBones.w] * inWeights.w;

	gl_Position = ubo.projection * ubo.view * L2W * vec4(inPosition, 1.0);
	gl_Position.xyz /= gl_Position.w;
	gl_Position.z = 1.0 - gl_Position.z;
	gl_Position.w = 1.0;
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;

void main() 
{
	//outColor = vec4(1.0, 0.0, 0.0, 1.0);
}