#version 410 core

float specularStrength = 0.5f;

in vec3 Normal;
in vec3 FragPos;

in vec2 TexCoord;

out vec4 color;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D ourTexture1;

uniform int q;

void main() {
    if (q == 1 || q == 2) color = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    else if (q == 3) color = vec4(0.5f, 0.0f, 0.0f, 1.0f);
    else {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;
        
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        float ambientStrength = 0.1f;
        vec3 ambient = ambientStrength * lightColor;
        
        vec3 result = (ambient + diffuse + specular) * objectColor;
        
        if (q == 5) color = texture(ourTexture1, TexCoord);
        else color = vec4(result, 1.0f);
    }
}