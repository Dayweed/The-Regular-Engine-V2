#version 450
#pragma stage : vert

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec3 inColor;
layout(location = 5) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform ParticleUBO
{
	mat4 m_ProjView;
} ubo;

layout(set = 0, binding = 6) uniform MaterialUBO
{
	vec4 m_Color;
} material;

layout (push_constant) uniform PushConstants
{
    mat4 L2W;
} PC;

layout(location = 0) out struct
{
    vec4 Color;
    vec2 UV;
} Out;

void main() 
{
	Out.Color = material.m_Color;
    Out.UV = in_UV;
    gl_Position = ubo.m_ProjView * PC.L2W * vec4(in_Position, 1.0);
}

#version 450
#pragma stage : frag

layout(location = 0) in struct
{
    vec4 Color;
    vec2 UV;
} In;

layout (set = 0, binding = 1) uniform sampler2D DiffuseMap;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(DiffuseMap, In.UV) * In.Color;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));
}