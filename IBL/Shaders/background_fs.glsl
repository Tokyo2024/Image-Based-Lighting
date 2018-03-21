#version 430 core

out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;
uniform sampler2D SSAOMap;
uniform vec3 ChangeRGB;

void main()
{	
    const float gamma = 2.2;
	
    vec3 envColor = texture(environmentMap, WorldPos).rgb * ChangeRGB;
    float ao = textureLod(SSAOMap, vec2(gl_FragCoord.x/1280.0f, gl_FragCoord.y/720.0f), 0.0).r; 
    // Reinhardɫ��ӳ��
    vec3 mapped = envColor / (envColor + vec3(1.0));
    // GammaУ��
    mapped = pow(mapped, vec3(1.0 / gamma));
//	if (ao > 0.6) ao = 1.0;
	mapped = mapped * ao; 
	FragColor = vec4(mapped, 1.0);
//    FragColor = vec4(ao, ao, ao, 1.0);
}