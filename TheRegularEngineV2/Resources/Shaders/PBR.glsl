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
	mat3 TBN;
	vec4 PosWorld; //w for gamma correction
	vec3 VertColor;
	vec2 TexCoord;
	vec4 MaterialColor; //Material color
	vec4 AmbientColor;
	vec4 ShadowCoord;
	vec4 ShadowCoord2;
	vec4 DirectionalLightDirection [2];
	vec4 DirectionalLightColor;
	vec3 VertNormal;
	vec3 CameraWorldPos;
	float ShadowIntensity;
	float m_DrawShadow;
} Out;

layout(push_constant) uniform Push
{
	mat4 m_Model;
	int  m_DLightIndex;
	bool m_DrawShadow;
} push;

layout(set = 0, binding = 0) uniform UBO
{
	mat4 m_ProjView;
	mat4 m_LightSpaceMatrix [2];
	vec4 m_LightPosition;
	vec4 m_LightColor;
	vec4 m_CameraPosition;
	vec4 m_DirectionalLightDirection [2];
	vec4 m_DirectionalLightColor [2];
	vec4 m_AmbientLight [2];
	float m_ShadowIntensity;
	float GammaValue;

} ubo;

layout(set = 0, binding = 6) uniform MaterialColor
{
	vec4 m_Color;
} MaterialUBO;

void main() 
{
    gl_Position = ubo.m_ProjView * push.m_Model * vec4(inPosition, 1.0);

	mat3 rot = mat3(push.m_Model);

    Out.VertColor = pow(inColor, vec3(ubo.GammaValue));
	Out.TexCoord = inTexCoord;

	vec3 normal = normalize(rot * inNormal);
	vec3 tangent = normalize(rot * inTangent);	
	vec3 bitangent = normalize(rot * inBitangent);
	Out.VertNormal = normal;

	Out.TBN = mat3( tangent, bitangent, normal);
	Out.PosWorld = push.m_Model * vec4(inPosition, 1.0);
	Out.PosWorld.w = ubo.GammaValue;
	Out.MaterialColor = MaterialUBO.m_Color;
	Out.AmbientColor = ubo.m_AmbientLight[0];
	Out.CameraWorldPos = ubo.m_CameraPosition.xyz;

	Out.ShadowCoord = ubo.m_LightSpaceMatrix[0] * push.m_Model * vec4(inPosition, 1.0);
	Out.ShadowCoord2 = ubo.m_LightSpaceMatrix[1] * push.m_Model * vec4(inPosition, 1.0);
	Out.DirectionalLightDirection[0] = ubo.m_DirectionalLightDirection[0];
	Out.DirectionalLightDirection[1] = ubo.m_DirectionalLightDirection[1];
	Out.DirectionalLightColor = ubo.m_DirectionalLightColor[0];
	Out.ShadowIntensity = ubo.m_ShadowIntensity;

	if (push.m_DrawShadow)
	{
		Out.m_DrawShadow = 1.f;
	}
	else
	{
		Out.m_DrawShadow = 0.f;
	}
}

#version 450

#pragma stage : frag

layout(location = 0) in struct
{
	mat3 TBN;
	vec4 PosWorld; //w for gamma correction
	vec3 VertColor;
	vec2 TexCoord;
	vec4 MaterialColor; //Material color
	vec4 AmbientColor;
	vec4 ShadowCoord;
	vec4 ShadowCoord2;
	vec4 DirectionalLightDirection [2];
	vec4 DirectionalLightColor;
	vec3 VertNormal;
	vec3 CameraWorldPos;
	float ShadowIntensity;
	float m_DrawShadow;
} In;

layout(push_constant) uniform Push
{
	mat4 m_Model;
	int  m_DLightIndex;
	bool m_DrawShadow;
} push;

layout(set = 0, binding = 1) uniform sampler2D DiffuseMap;
layout(set = 0, binding = 2) uniform sampler2D NormalMap;
layout(set = 0, binding = 3) uniform sampler2D RoughnessMap;
layout(set = 0, binding = 4) uniform sampler2D AOMap;
layout(set = 0, binding = 5) uniform sampler2D Metalness;
layout(set = 0, binding = 7) uniform sampler2D shadowMap;
layout(set = 0, binding = 10) uniform sampler2D shadowMap2;

layout(location = 0) out vec4 outColor;

const vec3 Glossiness = vec3(0.02, 0.02, 0.02);
const int CelShadingLevels = 3;
const float CelScaleFactor = 1.0 / float(CelShadingLevels);

float Shadow(in vec3 lightCoords, in vec3 normal)
{
	float shadow = 0.0;
	if(lightCoords.z >= 0.0 && lightCoords.z <= 1.0)
	{
		lightCoords.xy = lightCoords.xy * 0.5 + 0.5;

		float currentDepth = lightCoords.z;
		float bias = max(0.025 * (1.0 - dot(-In.DirectionalLightDirection[0].xyz, normal)), 0.015);
		
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
	}
	shadow = clamp(shadow, 0.0, 1.0);

	return shadow;
}

float Shadow2(in vec3 lightCoords, in vec3 normal)
{
	float shadow = 0.0;
	if(lightCoords.z >= 0.0 && lightCoords.z <= 1.0)
	{
		lightCoords.xy = lightCoords.xy * 0.5 + 0.5;

		float currentDepth = lightCoords.z;
		float bias = max(0.025 * (1.0 - dot(-In.DirectionalLightDirection[1].xyz, normal)), 0.015);
		
		int sampleRadius = 4;
		vec2 texelSize = 1.0 / textureSize(shadowMap2, 0);

		for(int y = -sampleRadius; y <= sampleRadius; y++)
		{
			for(int x = -sampleRadius; x <= sampleRadius; x++)
			{
				float closestDepth = texture(shadowMap2, lightCoords.xy + vec2(x, y) * texelSize).r;

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
	float shadow2 = Shadow2(In.ShadowCoord2.xyz / In.ShadowCoord2.w, normal);
	
	//Light calculations
	//vec3 lightDirection = In.LightPosWorld - In.PosWorld.xyz;
	//const float lightDistance = length(lightDirection);
	//lightDirection = normalize(lightDirection);

	//float lightAttenuation = 1.0 / (1.0 + 0.1 * lightDistance + 0.01 * lightDistance * lightDistance);
	//lightAttenuation = clamp(lightAttenuation, 0.0, 1.0);
	//float lightAttenuation = clamp(1 / lightDistance, 0.0, 1.0);
	//const vec3 attenuationColor = lightAttenuation * In.LightColor.rgb;

	//Diffuse intensity
	float diffuseIntensity = max(dot(normal, -normalize(In.DirectionalLightDirection[0].xyz)), 0.005);

	vec3 pixelToCamera = normalize(In.CameraWorldPos - In.PosWorld.xyz);
	float rimFactor = dot(normal, pixelToCamera);
	rimFactor = 1.0 - rimFactor;
	rimFactor = max(rimFactor, 0.0);
	rimFactor = pow(rimFactor, 1);

	//Eye to texel direction
	//const vec3 eyeDirection = normalize(In.PosWorld.xyz - In.CamearPos.xyz);

	//Shininess
	//const float shininess = mix(1, 100, 1 - texture(RoughnessMap, In.TexCoord).r);
	//const float specularIntensity = pow(max(dot(reflect(-In.DirectionalLightDirection, normal), eyeDirection), 0.0), shininess);
	const vec3 ambient = In.AmbientColor.rgb * In.AmbientColor.a * texture(AOMap, In.TexCoord).rgb * texture(AOMap, In.TexCoord).a;
	
	float dp = max(dot(normalize(In.VertNormal), -normalize(In.DirectionalLightDirection[0].xyz)), 0.0025);

	//Diffuse color
	if(dp <= 0.05)
	{
		shadow = 0.0;
		shadow2 = 0.0;
	}
	diffuseIntensity = ceil(diffuseIntensity * CelShadingLevels) * CelScaleFactor;
	dp = smoothstep(0.1, 1.0, dp) * float(CelShadingLevels);
	dp = ceil(dp) * CelScaleFactor;
	diffuseIntensity = mix(diffuseIntensity, dp, 0.5);
	vec3 diffuse = In.VertColor * texture(DiffuseMap, In.TexCoord).rgb * In.MaterialColor.rgb * In.MaterialColor.a * diffuseIntensity * In.DirectionalLightColor.rgb * In.DirectionalLightColor.a;
	vec3 rimColor = texture(DiffuseMap, In.TexCoord).rgb * rimFactor;
	if(push.m_DrawShadow)
	{
		outColor.rgb = ambient * (1.0 - shadow2) * (1.0 - shadow) * (diffuse * texture(DiffuseMap, In.TexCoord).a + rimColor * texture(DiffuseMap, In.TexCoord).a * 0.5);
	}
	else
	{
		outColor.rgb = ambient * (diffuse * texture(DiffuseMap, In.TexCoord).a + rimColor * texture(DiffuseMap, In.TexCoord).a * 0.5);
	}

	//Convert from HDR to LDR before gamma correction - for the blue tint
	outColor.rgb = outColor.rgb / ( outColor.rgb + vec3(1.0, 1.0, 0.9) );

	//Gamma correction
	outColor.rgb = (pow(outColor.rgb, vec3(1.0 / In.PosWorld.w)));
	outColor.a = texture(DiffuseMap, In.TexCoord).a * In.MaterialColor.a;
}