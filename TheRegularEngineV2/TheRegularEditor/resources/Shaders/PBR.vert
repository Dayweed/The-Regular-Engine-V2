#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec2 inTexCoord;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outTexCoord;

layout(location = 2) out struct
{
	mat3 BTN;
	vec4 LightDirection;
	vec4 WorldSpacePos;
} outStruct;

layout(push_constant) uniform Push
{
	mat4 m_Model;
} push;

layout(set = 0, binding = 0) uniform UBO
{
	mat4 m_ProjView;
	vec4 m_LightDirection;
}ubo;

const float AMBIENT_INTENSITY = 0.05;

void main() 
{
    gl_Position = ubo.m_ProjView * push.m_Model * vec4(inPosition, 1.0);

	//vec3 normalWorldSpace = normalize(mat3(push.m_Model) * inNormal);
	//float lightIntensity = AMBIENT_INTENSITY + max(dot(normalWorldSpace, -normalize(ubo.m_LightDirection.xyz)), 0);

    //outColor = lightIntensity * inColor;
    outColor = inColor;
	outTexCoord = inTexCoord;

	vec3 normal = normalize(mat3(push.m_Model) * inNormal);
	vec3 tangent = normalize(mat3(push.m_Model) * inTangent);
	vec3 bitangent = normalize(mat3(push.m_Model) * inBitangent);

	outStruct.BTN = mat3(tangent, bitangent, normal);
	outStruct.LightDirection = ubo.m_LightDirection;
	outStruct.WorldSpacePos = push.m_Model * vec4(inPosition, 1.0);
}