#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

layout(location = 2) in struct
{
	mat3 BTN;
    vec4 LightDirection;
    vec4 WorldSpacePos;

} inStruct;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D DiffuseMap;
layout(set = 0, binding = 2) uniform sampler2D NormalMap;

void main() 
{
    //Calculate normal from normal map
    vec3 normal;
    normal.xy = (texture(NormalMap, inTexCoord).gr * 2.0) - 1.0;
    normal.z = sqrt(1.0 - clamp(dot(normal.xy, normal.xy), 0.0, 1.0));
    normal = normalize(normal * inStruct.BTN);
    
    //Light calculations
    const float lightDistance = length(inStruct.LightDirection.xyz);
    vec3 lightDirection = normalize(inStruct.LightDirection.xyz);

    //Diffuse intensity
    float diffuseIntensity = max(dot(normal, -lightDirection), 0.0);

    //Diffuse color
    vec4 diffuseColor = texture(DiffuseMap, inTexCoord) * vec4(inColor, 1.0) * texture(NormalMap, inTexCoord);

    //Final color
    outColor = diffuseColor * diffuseIntensity;
    outColor = vec4(outColor.rgb, 1.0);
}