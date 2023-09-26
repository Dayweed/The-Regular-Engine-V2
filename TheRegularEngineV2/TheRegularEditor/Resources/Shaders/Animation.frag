#version 450

layout(location = 0) in struct 
{
	vec4 Color;
	vec2 UV;
} In;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D Diffuse;
layout(set = 0, binding = 2) uniform sampler2D DiffuseAO;
layout(set = 0, binding = 3) uniform sampler2D NormalMap;
layout(set = 0, binding = 4) uniform sampler2D Specular;
layout(set = 0, binding = 5) uniform sampler2D Glossiness;

void main() 
{
   outColor = In.Color;;// * texture(Diffuse, In.UV);
}