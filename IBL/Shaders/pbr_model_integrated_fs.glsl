#version 430 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform vec3 specularColor;
uniform vec3 diffuseColor;
uniform vec3 camPos;
//uniform float ao = 1.0f;

uniform sampler2D uSSAOTexture;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 ChangeRGB;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   
// ----------------------------------------------------------------------------
void main()
{	
    float metallic  = specularColor.r;  
	float roughness = specularColor.g;  

    vec3 N = Normal;
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N); 
 
	vec3 albedo = texture(texture_diffuse1, TexCoords).rgb;
//    vec3 albedo = diffuseColor; 
	float ao = textureLod(uSSAOTexture, vec2(gl_FragCoord.x/1280.0f, gl_FragCoord.y/720.0f), 0.0).r;
//    float ao = 1.0f;
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb * ChangeRGB;
    vec3 diffuse    = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 5.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y); 

    vec3 ambient = (kD * diffuse + specular) * ao;
    vec3 color = ambient;

    const float gamma = 2.2;
	color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / gamma));

    FragColor = vec4(color, 1.0);
 //   FragColor = vec4(ao, ao, ao, 1.0);
}