#version 450
#pragma stage : vert

layout(location = 0) in vec3 in_Position;

layout(set = 0, binding = 0) uniform UBO
{
	mat4 m_ProjView;
	vec3 m_LightPosition;
	vec4 m_LightColor;
	vec4 m_CameraPosition;
	vec4 m_DirectionalLightDirection;
	vec4 m_DirectionalLightColor;
	vec4 m_AmbientLight;

	mat4 m_Proj;
	mat4 m_View;
}ubo;

layout(location = 0) out vec3 outPosition;

void main() 
{
	mat4 UpdatedView = mat4(mat3(ubo.m_View));
    outPosition = in_Position;
    gl_Position = vec4(ubo.m_Proj * UpdatedView * vec4(in_Position, 1.0)).xyww;
}

#version 450
#pragma stage : frag

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform samplerCube SamplerCubeMap;

void main() 
{
    outColor = texture(SamplerCubeMap, inUVW);
}