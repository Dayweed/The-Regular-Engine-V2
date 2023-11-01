#version 450
#pragma stage : vert

layout(location = 0) in vec3 in_Position;

layout (binding = 0) uniform UBO 
{
	mat4 depthMVP;
} ubo;

layout(location = 0) out vec3 outPosition;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
    gl_Position = ubo.depthMVP * vec4(in_Position, 1.0);
}

#version 450
#pragma stage : frag

layout (location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}