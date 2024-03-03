#version 450
#pragma stage : vert

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec3 inColor;
layout(location = 5) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform UBO
{
	mat4 m_ProjView;
	mat4 m_LightSpaceMatrix;
	vec3 m_LightPosition;
	vec4 m_LightColor;
	vec4 m_CameraPosition;
	vec4 m_DirectionalLightDirection;
	vec4 m_DirectionalLightColor;
	vec4 m_AmbientLight;
	float m_ShadowIntensity;
} ubo;

layout(set = 0, binding = 8) uniform ParticleUBO
{
	mat4 ParticleL2W [500];
} ubo_particle;

layout(set = 0, binding = 6) uniform MaterialUBO
{
	vec4 m_Color;
} material;

layout(location = 0) out struct
{
	mat3 TBN;
	vec4 PosWorld; //w for gamma correction
	vec3 VertColor;
	vec2 TexCoord;
	vec4 MaterialColor; //Material color
	vec4 AmbientColor;
	vec4 DirectionalLightDirection;
	vec4 DirectionalLightColor;
	vec3 VertNormal;
	vec3 CameraWorldPos;
} Out;

const float gamma = 2.2;

void main() 
{
    gl_Position = ubo.m_ProjView * ubo_particle.ParticleL2W[gl_InstanceIndex] * vec4(inPosition, 1.0);

    mat3 rot = mat3(ubo_particle.ParticleL2W[gl_InstanceIndex]);
	vec3 normal = normalize(rot * inNormal);
	vec3 tangent = normalize(rot * inTangent);	
	vec3 bitangent = normalize(rot * inBitangent);

	Out.TBN = mat3( tangent, bitangent, normal);
    Out.PosWorld = ubo_particle.ParticleL2W[gl_InstanceIndex] * vec4(inPosition, 1.0);
    Out.PosWorld.w = gamma;
    Out.VertColor = pow(inColor, gamma.rrr);
	Out.TexCoord = inTexCoord;
	Out.MaterialColor = material.m_Color;
    Out.AmbientColor = ubo.m_AmbientLight;
	Out.DirectionalLightDirection = ubo.m_DirectionalLightDirection;
	Out.DirectionalLightColor = ubo.m_DirectionalLightColor;
    Out.VertNormal = normal;
    Out.CameraWorldPos = ubo.m_CameraPosition.xyz;
}

#version 450
#pragma stage : frag

layout(set = 0, binding = 1) uniform sampler2D DiffuseMap;
layout(set = 0, binding = 2) uniform sampler2D NormalMap;
layout(set = 0, binding = 3) uniform sampler2D RoughnessMap;
layout(set = 0, binding = 4) uniform sampler2D AOMap;
layout(set = 0, binding = 5) uniform sampler2D Metalness;
layout(set = 0, binding = 7) uniform sampler2D shadowMap;

layout(location = 0) in struct
{
	mat3 TBN;
	vec4 PosWorld; //w for gamma correction
	vec3 VertColor;
	vec2 TexCoord;
	vec4 MaterialColor; //Material color
	vec4 AmbientColor;
	vec4 DirectionalLightDirection;
	vec4 DirectionalLightColor;
	vec3 VertNormal;
	vec3 CameraWorldPos;
} In;

layout(location = 0) out vec4 outColor;

const int CelShadingLevels = 2;
const float CelScaleFactor = 1.0 / float(CelShadingLevels);

void main()
{
	vec3 normal;
	normal.rg = (texture(NormalMap, In.TexCoord).gr) * 2.0 - 1.0;
	normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
	normal = normalize(In.TBN * normal); //Transform normal to world space

	float diffuseIntensity = max(dot(normal, -normalize(In.DirectionalLightDirection.xyz)), 0.005);
	vec3 pixelToCamera = normalize(In.CameraWorldPos - In.PosWorld.xyz);
	float rimFactor = dot(normal, pixelToCamera);
	rimFactor = 1.0 - rimFactor;
	rimFactor = max(rimFactor, 0.0);
	rimFactor = pow(rimFactor, 1);

	const vec3 ambient = In.AmbientColor.rgb * In.AmbientColor.a * texture(AOMap, In.TexCoord).rgb * texture(AOMap, In.TexCoord).a;
	
    float dp = max(dot(normalize(In.VertNormal), -normalize(In.DirectionalLightDirection.xyz)), 0.0025);
    diffuseIntensity = ceil(diffuseIntensity * CelShadingLevels) * CelScaleFactor;
	dp = smoothstep(0.1, 1.0, dp) * float(CelShadingLevels);
	dp = ceil(dp) * CelScaleFactor;
	diffuseIntensity = mix(diffuseIntensity, dp, 0.5);
    vec3 diffuse = In.VertColor * texture(DiffuseMap, In.TexCoord).rgb * In.MaterialColor.rgb * In.MaterialColor.a * diffuseIntensity * In.DirectionalLightColor.rgb * In.DirectionalLightColor.a;
	vec3 rimColor = texture(DiffuseMap, In.TexCoord).rgb * rimFactor;

    outColor.rgb = ambient * (diffuse * texture(DiffuseMap, In.TexCoord).a + rimColor * texture(DiffuseMap, In.TexCoord).a * 0.5);
	outColor.rgb = outColor.rgb / ( outColor.rgb + vec3(1.0, 1.0, 0.9) );

	//Gamma correction
	outColor.rgb = (pow(outColor.rgb, vec3(1.0 / In.PosWorld.w)));
	outColor.a = texture(DiffuseMap, In.TexCoord).a * In.MaterialColor.a;
}