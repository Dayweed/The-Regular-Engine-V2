#version 450

layout(location = 0) in vec4 in_Color;
layout(location = 0) out vec4 outColor;

void main() 
{
	outColor = in_Color;
}