#version 450
#pragma stage : vert

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

layout(set = 0, binding = 0) uniform SSAOUBO
{
	vec2 m_ScreenSize;
	float m_Radius;
	float m_Bias;
	float m_Intensity;
	float m_MaxDistance;
} ubo;

layout(location = 0) out struct
{
	vec2 UV;
	vec2 ScreenSize;
	float Radius;
	float Bias;
	float Intensity;
	float MaxDistance;
} Out;

void main() 
{
	Out.UV = in_UV;
	Out.ScreenSize = ubo.m_ScreenSize;
	Out.Radius = ubo.m_Radius;
	Out.Bias = ubo.m_Bias;
	Out.Intensity = ubo.m_Intensity;
	Out.MaxDistance = ubo.m_MaxDistance;
	gl_Position = vec4(in_Position, 0.0, 1.0);
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;
layout(set = 0, binding = 8) uniform sampler2D depthMap;
//layout(set = 0, binding = 13) uniform sampler2D normalMap;

const float threshold = 0.0001;
const int kernelSize = 64;

layout(location = 0) in struct
{
	vec2 UV;
	vec2 ScreenSize;
	float Radius;
	float Bias;
	float Intensity;
	float MaxDistance;
} In;

float rand(vec2 co) 
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	vec2 texCoord = gl_FragCoord.xy / In.ScreenSize;
    float depth = texture(depthMap, texCoord).r;
    
    float ao = 0.0;
    for (int i = 0; i < kernelSize; ++i) 
	{
        // Generate random sample direction in screen space
        float angle = rand(texCoord) * 2.0 * 3.14159;
        vec2 offset = vec2(cos(angle), sin(angle)) * In.Radius;
        vec2 sampleTexCoord = texCoord + offset;

        // Sample depth
        float sampleDepth = texture(depthMap, sampleTexCoord).r;

        // Calculate occlusion factor
		float rangeCheck = (depth - sampleDepth) <= In.MaxDistance ? 1.0 : 0.0;
        ao += (depth - In.Bias) >= sampleDepth ? rangeCheck : 0.0;
    }
    ao /= float(kernelSize);
    ao = 1.0 - ao;

    outColor *= vec4(vec3(ao * In.Intensity), 1.0);
}


		