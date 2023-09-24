#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec2 inTexCoord;

layout(location = 2) out struct
{
	vec3 VertColor;
	vec2 TexCoord;
	mat3 BTN;
	vec3 LightPosWorld;
	vec4 LightColor;
	vec4 PosWorld; //w for gamma correction
} Out;

layout(push_constant) uniform Push
{
	mat4 m_Model;
} push;

layout(set = 0, binding = 0) uniform UBO
{
	mat4 m_ProjView;
	//vec4 m_LightDirection;
	vec3 m_LightPosition;
	vec4 m_LightColor;
}ubo;

const float AMBIENT_INTENSITY = 0.05;
const float gamma = 2.2;

void main() 
{
    gl_Position = ubo.m_ProjView * push.m_Model * vec4(inPosition, 1.0);

	//vec3 normalWorldSpace = normalize(mat3(push.m_Model) * inNormal);
	//float lightIntensity = AMBIENT_INTENSITY + max(dot(normalWorldSpace, -normalize(ubo.m_LightDirection.xyz)), 0);

    //outColor = lightIntensity * inColor;
    Out.VertColor = pow(inColor, gamma.rrr);
	Out.TexCoord = inTexCoord;

	mat3 rot = mat3(push.m_Model);

	vec3 normal = normalize(rot * inNormal);
	vec3 tangent = normalize(rot * inTangent);
	vec3 bitangent = normalize(rot * inBitangent);

	Out.BTN = mat3(tangent, bitangent, normal);
	//outStruct.LightDirection = normalize(ubo.m_LightDirection.xyz);
	Out.LightPosWorld = ubo.m_LightPosition;
	Out.PosWorld = push.m_Model * vec4(inPosition, 1.0);
	Out.PosWorld.w = gamma;
	Out.LightColor = ubo.m_LightColor;
}