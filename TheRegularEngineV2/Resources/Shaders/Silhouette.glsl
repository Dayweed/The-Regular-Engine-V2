#version 450
#pragma stage : vert

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

struct VertexOut
{
	vec2 UV;
};

layout (location = 0) out VertexOut Out;

void main() 
{
	Out.UV = in_UV;
	gl_Position = vec4(in_Position, 0.0, 1.0);
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;
layout(set = 0, binding = 8) uniform sampler2D depthMap;
layout(set = 0, binding = 9) uniform sampler2D IDMap;

struct VertexOut
{
	vec2 UV;
};

layout (location = 0) in VertexOut In;

void main()
{
	//Start checking for ID first
	const float ID = texture(IDMap, In.UV).r;
	const float depth = texture(depthMap, In.UV).r;
	if(ID.r < 1.0)
	{
		if(depth < ID)
        {
            outColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
	}
	else
	{
		discard;
	}
}