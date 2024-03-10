#version 450
#pragma stage : vert

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

layout(set = 0, binding = 0) uniform VignetteUBO
{
	vec4 m_Color;
    vec2 m_Resolution;
    float m_Radius;
    float m_Softness;
} ubo;

struct VertexOut
{
    vec4 Color;
    vec2 Resolution;
	vec2 UV;
    float Radius;
    float Softness;
};

layout (location = 0) out VertexOut Out;

void main() 
{
    Out.Resolution = ubo.m_Resolution;
	Out.Color = ubo.m_Color;
    Out.UV = in_UV;
    Out.Radius = ubo.m_Radius;
    Out.Softness = ubo.m_Softness;
    gl_Position = vec4(in_Position, 0.0, 1.0);
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;

struct VertexOut
{
    vec4 Color;
    vec2 Resolution;
	vec2 UV;
    float Radius;
    float Softness;
};

layout (location = 0) in VertexOut In;

void main()
{
	vec2 uv = (gl_FragCoord.xy / In.Resolution) - vec2(0.5);
	float dist = length(uv);
	float vignette = smoothstep(In.Radius, In.Radius - In.Softness, dist);
    outColor = In.Color - vignette;
}