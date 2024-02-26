#version 450
#pragma stage : vert

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_UV;

layout(push_constant) uniform PushConst
{
	mat4 Proj;
	vec4 Color;
};

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

void main() 
{
	gl_Position = Proj * vec4(in_Position, 1.0);
	outUV = in_UV;
	outColor = Color;
}

#version 450
#pragma stage : frag

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D FontTexture;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(FontTexture,inUV).r);
	outColor = inColor * sampled;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));
}