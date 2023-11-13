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
	//vec4 LightColor;
	//vec4 CamearPos;
	vec4 PosWorld; //w for gamma correction
	//vec3 LightPosWorld;
	vec3 VertColor;
	vec2 TexCoord;
	vec4 Color;
	vec4 AmbientColor;
	vec4 ShadowCoord;
	vec4 DirectionalLightDirection;
	vec4 DirectionalLightColor;
} Out;

layout(push_constant) uniform Push
{
	mat4 m_Model;
} push;

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
} ubo;

layout(set = 0, binding = 6) uniform MaterialColor
{
	vec4 m_Color;
} MaterialUBO;

const float gamma = 2.2;

const mat4 C2T = mat4
( 
	0.5, 0.0, 0.0, 0.5,
	0.0, 0.5, 0.0, 0.5,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0 
);

void main() 
{
    gl_Position = ubo.m_ProjView * push.m_Model * vec4(inPosition, 1.0);

	mat3 rot = mat3(push.m_Model);

	// vec3 normalWorldSpace = normalize(mat3(push.m_Model) * inNormal);
	// float lightIntensity = ubo.m_AmbientLight.a + max(dot(normalWorldSpace, -normalize(ubo.m_DirectionalLightDirection.xyz)), 0);

    // Out.VertColor = lightIntensity *  ubo.m_AmbientLight.rgb * inColor;
	// Out.VertColor = Out.VertColor * ubo.m_DirectionalLightColor.rgb * ubo.m_DirectionalLightColor.a;
    Out.VertColor = pow(inColor, gamma.rrr);
	Out.TexCoord = inTexCoord;

	vec3 normal = normalize(rot * inNormal);
	vec3 tangent = normalize(rot * inTangent);
	vec3 bitangent = normalize(rot * inBitangent);

	Out.BTN = mat3(tangent, bitangent, normal);
	//Out.LightPosWorld = ubo.m_LightPosition;
	Out.PosWorld = push.m_Model * vec4(inPosition, 1.0);
	Out.PosWorld.w = gamma;
	//Out.LightColor = ubo.m_LightColor;
	//Out.CamearPos = ubo.m_CameraPosition;
	Out.Color = MaterialUBO.m_Color;
	Out.AmbientColor = ubo.m_AmbientLight;

	Out.ShadowCoord = ubo.m_LightSpaceMatrix * push.m_Model * vec4(inPosition, 1.0);
	Out.DirectionalLightDirection = ubo.m_DirectionalLightDirection;
	Out.DirectionalLightColor = ubo.m_DirectionalLightColor;
}

#version 450

#pragma stage : frag

layout(location = 0) in struct
{
	mat3 BTN;
	//vec4 LightColor;
	//vec4 CamearPos;
	vec4 PosWorld; //w for gamma correction
	//vec3 LightPosWorld;
	vec3 VertColor;
	vec2 TexCoord;
	vec4 Color;
	vec4 AmbientColor;
	vec4 ShadowCoord;
	vec4 DirectionalLightDirection;
	vec4 DirectionalLightColor;
} In;

layout(set = 0, binding = 1) uniform sampler2D DiffuseMap;
layout(set = 0, binding = 2) uniform sampler2D NormalMap;
layout(set = 0, binding = 3) uniform sampler2D RoughnessMap;
layout(set = 0, binding = 4) uniform sampler2D AOMap;
layout(set = 0, binding = 5) uniform sampler2D Metalness;
layout(set = 0, binding = 7) uniform sampler2D shadowMap;

layout(location = 0) out vec4 outColor;

const vec3 Glossiness = vec3(0.02, 0.02, 0.02);

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

	return shadow;
}

void main() 
{
	//Calculate normal from normal map
	vec3 normal;
	normal.rg = (texture(NormalMap, In.TexCoord).gr) * 2.0 - 1.0;
	normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
	normal = normalize(In.BTN * normal); //Transform normal to world space

	const float shadow = Shadow(In.ShadowCoord.xyz / In.ShadowCoord.w, normal);
	
	//Light calculations
	//vec3 lightDirection = In.LightPosWorld - In.PosWorld.xyz;
	//const float lightDistance = length(lightDirection);
	//lightDirection = normalize(lightDirection);

	//float lightAttenuation = 1.0 / (1.0 + 0.1 * lightDistance + 0.01 * lightDistance * lightDistance);
	//lightAttenuation = clamp(lightAttenuation, 0.0, 1.0);
	//float lightAttenuation = clamp(1 / lightDistance, 0.0, 1.0);
	//const vec3 attenuationColor = lightAttenuation * In.LightColor.rgb;

	//Diffuse intensity
	const float diffuseIntensity = max(dot(normal, -In.DirectionalLightDirection.xyz), 0.0);

	//Eye to texel direction
	//const vec3 eyeDirection = normalize(In.PosWorld.xyz - In.CamearPos.xyz);

	//Shininess
	//const float shininess = mix(1, 100, 1 - texture(RoughnessMap, In.TexCoord).r);
	//const float specularIntensity = pow(max(dot(reflect(-In.DirectionalLightDirection, normal), eyeDirection), 0.0), shininess);
	
	//Diffuse color
	vec4 diffuseColor = vec4(In.VertColor, 1.0) * texture(DiffuseMap, In.TexCoord);

	//outColor.rgb = In.AmbientColor.rgb * In.AmbientColor.a + diffuseColor.rgb * texture(AOMap, In.TexCoord).rgb * In.Color.rgb * In.Color.a;
	outColor.rgb = diffuseColor.rgb * texture(AOMap, In.TexCoord).rgb * In.Color.rgb * In.Color.a + In.AmbientColor.rgb * In.AmbientColor.a;

	vec3 lightModel = In.DirectionalLightColor.rgb * (diffuseIntensity.rrr * diffuseColor.rgb) * In.DirectionalLightColor.a * (1.0 - shadow);

	outColor.rgb += lightModel;

	//Convert from HDR to LDR before gamma correction - for the blue tint
	outColor.rgb = outColor.rgb / ( outColor.rgb + vec3(1.0, 1.0, 0.9) );

	//Gamma correction
	outColor.rgb = (pow(outColor.rgb, vec3(1.0 / In.PosWorld.w)));
	outColor.a = 1.0;
}