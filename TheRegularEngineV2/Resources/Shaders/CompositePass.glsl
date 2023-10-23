#version 450
#pragma stage : vert

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_Tex;

layout(location = 0) out vec2 outTex;

void main() 
{
    gl_Position = vec4(in_Position, 1.0);
    outTex = in_Tex;
}

#version 450
#pragma stage : frag

layout(location = 0) in vec2 in_Tex;
layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D u_Texture;

void main() 
{
	outColor = vec4(texture(u_Texture, in_Tex).rgb, 1.0);
}