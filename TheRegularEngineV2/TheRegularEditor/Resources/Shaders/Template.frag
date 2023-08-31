#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

void main() 
{
    outColor = vec4(inColor, 1.0) * texture(texSampler, inTexCoord);
    outColor = vec4(outColor.rgb, 1.0);
}