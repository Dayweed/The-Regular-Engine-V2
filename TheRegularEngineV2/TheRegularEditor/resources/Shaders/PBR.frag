#version 450

layout(location = 2) in struct
{
	vec3 VertColor;
	vec2 TexCoord;
	mat3 BTN;
	vec3 LightPosWorld;
	vec4 LightColor;
	vec4 PosWorld; //w for gamma correction
} In;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D DiffuseMap;
layout(set = 0, binding = 2) uniform sampler2D NormalMap;

const float AMBIENT_INTENSITY = 0.1;

void main() 
{
    //Calculate normal from normal map
    vec3 normal;
    normal.rg = (texture(NormalMap, In.TexCoord).gr) * 2.0 - 1.0;
    normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
    normal = normalize(In.BTN * normal); //Transform normal to world space
    
    //Light calculations
    vec3 lightDirection = In.LightPosWorld - In.PosWorld.xyz;
    const float lightDistance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    const float lightAttenuation = 1.0 / (1.0 + 0.1 * lightDistance + 0.01 * lightDistance * lightDistance);
    const vec3 attenuationColor = lightAttenuation * In.LightColor.rgb * In.LightColor.a;

    //Diffuse intensity
    float diffuseIntensity = AMBIENT_INTENSITY + max(dot(normal, lightDirection), 0.0);

    //Diffuse color
    vec4 diffuseColor = vec4(In.VertColor, 1.0) * texture(DiffuseMap, In.TexCoord);

    //Final color
    outColor = diffuseColor * diffuseIntensity;
    //outColor.rgb += attenuationColor;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / In.PosWorld.w));
    outColor.a = 1.0;
}