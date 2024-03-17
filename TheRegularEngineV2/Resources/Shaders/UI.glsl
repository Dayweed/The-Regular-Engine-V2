#version 450
#pragma stage : vert

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_UV;

layout(set = 0, binding = 0) uniform UIUBO
{
	mat4 m_ProjView;
    float mGammeValue;
} ubo;

layout (push_constant) uniform Transform
{
    mat4 Transform;
    vec4 Color;
} PC;

struct VertexOut
{
    vec4 Color;
	vec2 UV;
    float Gamma;
};

layout (location = 0) out VertexOut Out;

void main() 
{
	Out.Color = PC.Color;
    Out.UV = in_UV;
    Out.Gamma = ubo.mGammeValue;
    gl_Position = ubo.m_ProjView * PC.Transform * vec4(in_Position, 1.0);
    gl_Position.z = 1;
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;

struct VertexOut
{
    vec4 Color;
	vec2 UV;
    float Gamma;
};

layout (location = 0) in VertexOut In;
layout (set = 0, binding = 1) uniform sampler2D UI_Texture;

void main()
{
    outColor = texture(UI_Texture, In.UV) * In.Color;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / In.Gamma));
}