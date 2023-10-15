#version 450

#pragma stage : vert

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec3 inColor;
layout(location = 5) in vec2 inTexCoord;

layout(location = 0) out struct
{
	mat3 BTN;
	vec4 LightColor;
	vec4 CamearPos;
	vec4 PosWorld; //w for gamma correction
	vec3 LightPosWorld;
	vec3 VertColor;
	vec2 TexCoord;
} Out;

layout(push_constant) uniform Push
{
	mat4 m_Model;
} push;

layout(set = 0, binding = 0) uniform UBO
{
	mat4 m_ProjView;
	vec3 m_LightPosition;
	vec4 m_LightColor;
	vec4 m_CameraPosition;
	vec4 m_DirectionalLight;
}ubo;

const float AMBIENT_INTENSITY = 0.05;
const float gamma = 2.2;

void main() 
{
    gl_Position = ubo.m_ProjView * push.m_Model * vec4(inPosition, 1.0);

	mat3 rot = mat3(push.m_Model);

	vec3 normalWorldSpace = normalize(mat3(push.m_Model) * inNormal);
	float lightIntensity = AMBIENT_INTENSITY + max(dot(normalWorldSpace, -normalize(ubo.m_DirectionalLight.xyz)), 0);

    Out.VertColor = lightIntensity * inColor;
    Out.VertColor = pow(Out.VertColor, gamma.rrr);
	Out.TexCoord = inTexCoord;

	vec3 normal = normalize(rot * inNormal);
	vec3 tangent = normalize(rot * inTangent);
	vec3 bitangent = normalize(rot * inBitangent);

	Out.BTN = mat3(tangent, bitangent, normal);
	Out.LightPosWorld = ubo.m_LightPosition;
	Out.PosWorld = push.m_Model * vec4(inPosition, 1.0);
	Out.PosWorld.w = gamma;
	Out.LightColor = ubo.m_LightColor;
	Out.CamearPos = ubo.m_CameraPosition;
}


#version 450

#pragma stage : frag

layout(location = 0) in struct
{
	mat3 BTN;
	vec4 LightColor;
	vec4 CamearPos;
	vec4 PosWorld; //w for gamma correction
	vec3 LightPosWorld;
	vec3 VertColor;
	vec2 TexCoord;
} In;

layout(set = 0, binding = 1) uniform sampler2D DiffuseMap;
layout(set = 0, binding = 2) uniform sampler2D NormalMap;
layout(set = 0, binding = 3) uniform sampler2D RoughnessMap;
layout(set = 0, binding = 4) uniform sampler2D AOMap;

layout(location = 0) out vec4 outColor;


const float AMBIENT_INTENSITY = 0.05;
const vec3 Glossiness = vec3(0.02, 0.02, 0.02);

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

	float lightAttenuation = 1.0 / (1.0 + 0.1 * lightDistance + 0.01 * lightDistance * lightDistance);
	lightAttenuation = clamp(lightAttenuation, 0.0, 1.0);
	//float lightAttenuation = clamp(1 / lightDistance, 0.0, 1.0);
	const vec3 attenuationColor = lightAttenuation * In.LightColor.rgb;

	//Diffuse intensity
	const float diffuseIntensity = max(dot(normal, lightDirection), 0.0);

	//Eye to texel direction
	const vec3 eyeDirection = normalize(In.PosWorld.xyz - In.CamearPos.xyz);

	//Shininess
	const float shininess = mix(1, 100, 1 - texture(RoughnessMap, In.TexCoord).r);
	const float specularIntensity = pow(max(dot(reflect(lightDirection, normal), eyeDirection), 0.0), shininess);
	
	//Diffuse color
	vec4 diffuseColor = vec4(In.VertColor, 1.0) * texture(DiffuseMap, In.TexCoord);

	outColor.rgb = AMBIENT_INTENSITY.rrr * diffuseColor.rgb * texture(AOMap, In.TexCoord).rgb;

	vec3 lightModel = In.LightColor.rgb * (specularIntensity.rrr * Glossiness + diffuseIntensity.rrr * diffuseColor.rgb) * In.LightColor.a;

	outColor.rgb += lightModel * attenuationColor;

	//Convert from HDR to LDR before gamma correction - for the blue tint
	outColor.rgb = outColor.rgb / ( outColor.rgb + vec3(1.0, 1.0, 0.9) );

	//Gamma correction
	outColor.rgb = pow(outColor.rgb, vec3(1.0 / In.PosWorld.w));
	outColor.a = 1.0;
}