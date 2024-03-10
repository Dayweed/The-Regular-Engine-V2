#version 450
#pragma stage : vert

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

// layout(set = 0, binding = 0) uniform VignetteUBO
// {
// 	vec4 m_Color;
// 	float m_Threshold;
// } ubo;

layout(location = 0) out struct
{
	//vec4 Color;
	vec2 UV;
	//float Threshold;
} Out;

void main() 
{
	//Out.Color = ubo.m_Color;
	Out.UV = in_UV;
	//Out.Threshold = ubo.m_Threshold;
	gl_Position = vec4(in_Position, 0.0, 1.0);
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;
layout(set = 0, binding = 8) uniform sampler2D depthMap;
layout(set = 0, binding = 9) uniform sampler2D IDMap;

layout(location = 0) in struct
{
	//vec4 Color;
	vec2 UV;
	//float Threshold;
} In;

void main()
{
	//Start checking for ID first
	const float ID = texture(IDMap, In.UV).r;
	const float depth = texture(depthMap, In.UV).r;
	if(ID.r < 1.0)
	{
		//if depth lesser means occluded
		if(depth < ID)
		//if(In.Threshold < (ID - depth))
		{
			outColor = vec4(0.65, 0.65, 0.65, 1.0);
			//outColor = In.Color;
		}
		else
		{
			discard;
		}
	}
	else
	{
		discard;
	}

}