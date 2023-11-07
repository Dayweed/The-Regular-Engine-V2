#version 450
#pragma stage : vert

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec2 in_UV;

layout(set = 0, binding = 0) uniform UIUBO
{
	mat4 m_ProjView;
} ubo;

layout (push_constant) uniform Transform
{
    mat4 Transform;
} L2W;

struct VertexOut
{
    vec4 Color;
	vec2 UV;
};

layout (location = 0) out VertexOut Out;

void main() 
{
	Out.Color = in_Color;
    Out.UV = in_UV;
    gl_Position = ubo.m_ProjView * L2W.Transform * vec4(in_Position, 1.0);
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;

struct VertexOut
{
    vec4 Color;
	vec2 UV;
};

layout (location = 0) in VertexOut In;
layout (set = 0, binding = 0) uniform sampler2D in_Texture;

void main() 
{
    outColor = texture(in_Texture, In.UV) * In.Color;
}