#version 450

#pragma stage : vert

layout(location = 0) in vec3  inPosition;
layout(location = 1) in vec3  inNormal;
layout(location = 2) in vec3  inTangent;
layout(location = 3) in vec3  inBitangent;
layout(location = 4) in vec3  inColor;
layout(location = 5) in vec2  inTexCoord;
layout(location = 6) in vec4  inWeights;
layout(location = 7) in ivec4 inBones;

layout(location = 0) out struct
{
	mat3 TBN;
	vec4 PosWorld; //w for gamma correction
	vec3 VertColor;
	vec2 TexCoord;
	vec4 MaterialColor; //Material color
	vec4 AmbientColor;
	vec4 ShadowCoord;
	vec4 DirectionalLightDirection;
	vec4 DirectionalLightColor;
	vec3 VertNormal;
	vec3 CameraWorldPos;
	float ShadowIntensity;
	bool DrawShadow;
} Out;

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

layout (set = 0, binding = 8) uniform UBOAnimation
{
	mat4 L2W[256];
} AnimationUBO;

layout(set = 0, binding = 6) uniform MaterialColor
{
	vec4 m_Color;
} MaterialUBO;

layout(push_constant) uniform Push
{
	mat4 m_Model;
	bool m_DrawShadow;
} push;

const float gamma = 2.2;

void main() 
{
	mat4 L2W = AnimationUBO.L2W[inBones.x] * inWeights.x + AnimationUBO.L2W[inBones.y] * inWeights.y 
             + AnimationUBO.L2W[inBones.z] * inWeights.z + AnimationUBO.L2W[inBones.w] * inWeights.w;

	gl_Position = ubo.m_ProjView * L2W * vec4(inPosition.xyz, 1.0);

	mat3 rot = mat3(push.m_Model);

    Out.VertColor = pow(inColor, gamma.rrr);
	Out.TexCoord = inTexCoord;

	vec3 normal = normalize(rot * inNormal);
	vec3 tangent = normalize(rot * inTangent);	
	vec3 bitangent = normalize(rot * inBitangent);

	Out.VertNormal = normal;

	Out.TBN = mat3( tangent, bitangent, normal);
	Out.PosWorld = push.m_Model * vec4(inPosition, 1.0);
	Out.PosWorld.w = gamma;
	Out.MaterialColor = MaterialUBO.m_Color;
	Out.AmbientColor = ubo.m_AmbientLight;
	Out.CameraWorldPos = ubo.m_CameraPosition.xyz;

	Out.ShadowCoord = ubo.m_LightSpaceMatrix * push.m_Model * vec4(inPosition, 1.0);
	Out.DirectionalLightDirection = ubo.m_DirectionalLightDirection;
	Out.DirectionalLightColor = ubo.m_DirectionalLightColor;
	Out.ShadowIntensity = ubo.m_ShadowIntensity;
	Out.DrawShadow = push.m_DrawShadow;
}


#version 450

#pragma stage : frag

layout(location = 0) in struct
{
	mat3 TBN;
	vec4 PosWorld; //w for gamma correction
	vec3 VertColor;
	vec2 TexCoord;
	vec4 MaterialColor;
	vec4 AmbientColor;
	vec4 ShadowCoord;
	vec4 DirectionalLightDirection;
	vec4 DirectionalLightColor;
	vec3 VertNormal;
	vec3 CameraWorldPos;
	float ShadowIntensity;
	bool DrawShadow;
} In;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D DiffuseMap;
layout(set = 0, binding = 2) uniform sampler2D NormalMap;
layout(set = 0, binding = 3) uniform sampler2D RoughnessMap;
layout(set = 0, binding = 4) uniform sampler2D AOMap;
layout(set = 0, binding = 5) uniform sampler2D Metalness;
layout(set = 0, binding = 7) uniform sampler2D shadowMap;

const vec3 Glossiness = vec3(0.02, 0.02, 0.02);
const int CelShadingLevels = 2;
const float CelScaleFactor = 1.0 / float(CelShadingLevels);

float Shadow(in vec3 lightCoords, in vec3 normal)
{
	float shadow = 0.0;
	if(lightCoords.z >= 0.0 && lightCoords.z <= 1.0)
	{
		lightCoords.xy = lightCoords.xy * 0.5 + 0.5;

		float currentDepth = lightCoords.z;
		float bias = max(0.025 * (1.0 - dot(-In.DirectionalLightDirection.xyz, normal)), 0.015);
		
		int sampleRadius = 4;
		vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

		for(int y = -sampleRadius; y <= sampleRadius; y++)
		{
			for(int x = -sampleRadius; x <= sampleRadius; x++)
			{
				float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * texelSize).r;

				if(currentDepth - bias > closestDepth)
				{
					shadow += 1.0;
				}
			}
		}

		shadow /= pow(float((sampleRadius * 2 + 1)), 2.0);
	}

	if(shadow < 0.05)
	{
		shadow = 0.0;
	}
	else
	{
		shadow = In.ShadowIntensity;
		shadow *= 0.25;
	}
	shadow = clamp(shadow, 0.0, 1.0);

	return shadow;
}

void main() 
{
	//Calculate normal from normal map
	vec3 normal;
	normal.rg = (texture(NormalMap, In.TexCoord).gr) * 2.0 - 1.0;
	normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
	normal = normalize(In.TBN * normal); //Transform normal to world space

	float shadow = Shadow(In.ShadowCoord.xyz / In.ShadowCoord.w, normal);
	
	//Diffuse intensity
	float diffuseIntensity = max(dot(normal, -normalize(In.DirectionalLightDirection.xyz)), 0.005);

	vec3 pixelToCamera = normalize(In.CameraWorldPos - In.PosWorld.xyz);
	float rimFactor = dot(normal, pixelToCamera);
	rimFactor = 1.0 - rimFactor;
	rimFactor = max(rimFactor, 0.0);
	rimFactor = pow(rimFactor, 2.0);

	const vec3 ambient = In.AmbientColor.rgb * In.AmbientColor.a * texture(AOMap, In.TexCoord).rgb * texture(AOMap, In.TexCoord).a;
	
	float dp = max(dot(normalize(In.VertNormal), -normalize(In.DirectionalLightDirection.xyz)), 0.0);

	//Diffuse color
	if(dp <= 0.05)
	{
		shadow = 0.0;
	}
	diffuseIntensity = ceil(diffuseIntensity * CelShadingLevels) * CelScaleFactor;
	dp = smoothstep(0.1, 1.0, dp) * float(CelShadingLevels);
	dp = ceil(dp) * CelScaleFactor;
	diffuseIntensity = mix(diffuseIntensity, dp, 0.5);
	vec3 diffuse = In.VertColor * texture(DiffuseMap, In.TexCoord).rgb * In.MaterialColor.rgb * In.MaterialColor.a * diffuseIntensity * In.DirectionalLightColor.rgb * In.DirectionalLightColor.a;
	vec3 rimColor = texture(DiffuseMap, In.TexCoord).rgb * rimFactor;
	if(In.DrawShadow)
		outColor.rgb = ambient * (1.0 - shadow) * (diffuse * texture(DiffuseMap, In.TexCoord).a + rimColor * texture(DiffuseMap, In.TexCoord).a * 0.5);
	else
		outColor.rgb = ambient * (diffuse * texture(DiffuseMap, In.TexCoord).a + rimColor * texture(DiffuseMap, In.TexCoord).a * 0.5);
	//Convert from HDR to LDR before gamma correction - for the blue tint
	outColor.rgb = outColor.rgb / ( outColor.rgb + vec3(1.0, 1.0, 0.9) );

	//Gamma correction
	outColor.rgb = (pow(outColor.rgb, vec3(1.0 / In.PosWorld.w)));
	outColor.a = 1.0;
}