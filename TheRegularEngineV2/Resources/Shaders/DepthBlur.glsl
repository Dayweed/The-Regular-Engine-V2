#version 450
#pragma stage : vert

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

layout(set = 0, binding = 0) uniform DepthBlurUBO
{
	float m_DepthCutOff;
} ubo;

layout(location = 0) out struct
{
	vec2 UV;
	float DepthCutOff;
} Out;

void main() 
{
	Out.UV = in_UV;
	Out.DepthCutOff = ubo.m_DepthCutOff;
	gl_Position = vec4(in_Position, 0.0, 1.0);
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;
layout(set = 0, binding = 8) uniform sampler2D depthMap;
layout(set = 0, binding = 11) uniform sampler2D blurredColorMap;

const float threshold = 0.0001;
//const float depthCutOff = 0.9997;

layout(location = 0) in struct
{
	vec2 UV;
	float DepthCutOff;
} In;

void main()
{
	//Start checking for ID first
	const float depth = texture(depthMap, In.UV).r;
	//const float depthDifference = abs(depth - depthCutOff);
	//float blur = 1.0 - smoothstep(0.0, threshold, depthDifference);
	if(depth > In.DepthCutOff)
	{
		outColor = texture(blurredColorMap, In.UV);
	}
	else
		discard;
}
		