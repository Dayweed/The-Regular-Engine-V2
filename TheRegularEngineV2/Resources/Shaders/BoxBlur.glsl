#version 450
#pragma stage : vert

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

layout(set = 0, binding = 0) uniform BoxBlurUBO
{
	vec2 texelSize;
} ubo;

layout(location = 0) out struct
{
	vec2 UV;
	vec2 texelSize;
} Out;

void main() 
{
	Out.UV = in_UV;
	Out.texelSize = ubo.texelSize;
	gl_Position = vec4(in_Position, 0.0, 1.0);
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;
layout(set = 0, binding = 8) uniform sampler2D depthMap;
layout(set = 0, binding = 12) uniform sampler2D colorMap;

layout(location = 0) in struct
{
	vec2 UV;
	vec2 texelSize;
} In;

const int kernelSize = 3; // Adjust the kernel size for a larger blur
const float strength = 2.0; // Adjust the strength of the blur
const float depthCutOff = 0.9997;

void main()
{
	//Box blur
	vec4 blurColor = vec4(0.0);
	float depth = texture(depthMap, In.UV).r;
	float weight = 0;

   for (int i = -kernelSize; i <= kernelSize; ++i) 
   {
        for (int j = -kernelSize; j <= kernelSize; ++j) 
		{
            // Calculate the sampling position and weight for each texel in the kernel
            const vec2 offset = vec2(float(i), float(j)) * In.texelSize * strength;
            const float depthDifference = abs(depth - texture(depthMap, In.UV + offset).r);
            // Accumulate the weighted color values
            blurColor += texture(colorMap, In.UV + offset);
			weight += 1.0 - smoothstep(0.0, depthCutOff, depthDifference);
        }
    }
    
    // Normalize the blurred color by the total weight
	blurColor /= weight;
    outColor = blurColor;
}
		