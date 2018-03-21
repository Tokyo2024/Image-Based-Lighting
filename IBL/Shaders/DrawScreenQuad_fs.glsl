#version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D uColorTexture;

void main()
{    
    vec2 TexCoords1 = vec2(gl_FragCoord.x/1280.0f, gl_FragCoord.y/720.0f);
//	FragColor = vec4(texture(uColorTexture, TexCoords1).r, texture(uColorTexture, TexCoords1).r, texture(uColorTexture, TexCoords1).r, 1.0);
    FragColor = vec4(texture(uColorTexture, TexCoords1).rgb, 1.0);
}