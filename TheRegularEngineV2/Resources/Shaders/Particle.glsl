#version 450
#pragma stage : vert

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_UV;

layout(set = 0, binding = 0) uniform ParticleUBO
{
	mat4 m_ProjView;
} ubo;

layout (push_constant) uniform PushConstants
{
    mat4 Transform;
    vec4 Color;
} PC;

layout(location = 0) out struct
{
    vec4 Color;
    vec2 UV;
} Out;

void main() 
{
	Out.Color = PC.Color;
    Out.UV = in_UV;
    gl_Position = ubo.m_ProjView * PC.Transform * vec4(in_Position, 1.0);
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