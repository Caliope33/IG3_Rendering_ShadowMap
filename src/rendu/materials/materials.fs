#version 330 core
out vec4 FragColor;

in vec3 Normal;    // Nom synchronisé avec le VS 
in vec3 FragPos;   // Nom synchronisé avec le VS 

uniform vec3  u_BaseColor;
uniform float u_Metallic;
uniform float u_Roughness;
uniform vec3  u_LightPos;
uniform vec3  u_CameraPos;

const float PI = 3.14159265359;

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(u_CameraPos - FragPos);
    vec3 L = normalize(u_LightPos - FragPos);
    vec3 H = normalize(L + V);

    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotVH = clamp(dot(V, H), 0.0, 1.0);

    float alpha = u_Roughness * u_Roughness;
    float alpha2 = alpha * alpha;

    // 1. Distribution D (Trowbridge-Reitz GGX)
    float denomD = (dotNH * dotNH * (alpha2 - 1.0) + 1.0);
    float D = alpha2 / (PI * denomD * denomD); 

    // 2. Visibilité V (Smith Joint simplifiée)
    float visV = dotNV + sqrt(alpha2 + (1.0 - alpha2) * (dotNV * dotNV));
    float visL = dotNL + sqrt(alpha2 + (1.0 - alpha2) * (dotNL * dotNL)); 
    float Visibility = 1.0 / (visV * visL); 

    // 3. Fresnel F (Schlick)
    vec3 f0 = mix(vec3(0.04), u_BaseColor, u_Metallic);
    vec3 F = f0 + (1.0 - f0) * pow(1.0 - abs(dotVH), 5.0); 

    // 4. Combinaison Diffuse et Spéculaire
    vec3 f_specular = F * D * Visibility; 
    vec3 c_diff = mix(u_BaseColor, vec3(0.0), u_Metallic); 
    vec3 f_diffuse = (vec3(1.0) - F) * (1.0 / PI) * c_diff;

    vec3 color = (f_diffuse + f_specular) * dotNL; 

    // Tonemapping et correction gamma
    color = color / (color + vec3(1.0));
    FragColor = vec4(pow(color, vec3(1.0/2.2)), 1.0);
}