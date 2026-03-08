#version 330 core
out vec4 FragColor;

#define MAX_LIGHTS 4
in vec3 Normal;    
in vec3 FragPos;   
in vec4 FragPosLightSpace;

uniform int   u_LightCount;
uniform vec3  u_LightPos[MAX_LIGHTS];

uniform vec3  u_BaseColor;
uniform float u_Metallic;
uniform float u_Roughness;
uniform vec3  u_CameraPos;
uniform sampler2D u_ShadowMap;// depth map de la lumière 0

const float PI = 3.14159265359;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 N,vec3 L)
{   // Passage en coordonnées NDC [-1,1] → [0,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Fragment hors de la frustum lumière → pas d'ombre
    if(projCoords.z > 1.0) return 0.0;

    float shadow = 0.0;
    float currentDepth = projCoords.z;

    // Biais pour éviter le shadow acne (dépend de l'angle N·L)
    float bias = max(0.005 * (1.0 - dot(N, L)), 0.0005);

    // PCF : moyenne sur 9 samples voisins pour lisser les bords
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;

}

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(u_CameraPos - FragPos);
    
    vec3 Lo = vec3(0.0);
    
    for(int i = 0; i < u_LightCount; ++i){
        vec3 L = normalize(u_LightPos[i] - FragPos);
        vec3 H = normalize(L + V);

        float dotNH =clamp(dot(N, H), 0.0, 1.0);
        float dotNV =clamp(dot(N, V), 0.0, 1.0);
        float dotNL =clamp(dot(N, L), 0.0, 1.0);
        float dotVH =clamp(dot(V, H), 0.0, 1.0);

        float alpha = u_Roughness * u_Roughness;
        float alpha2 = alpha * alpha;

        // 1. Distribution D (Trowbridge-Reitz GGX)
        float denomD = (dotNH * dotNH * (alpha2 - 1.0) + 1.0);
        float D = alpha2 / (PI * denomD * denomD); 

        // 2. Visibilité V (Smith)
        float visV = dotNV + sqrt(alpha2 + (1.0 - alpha2) * (dotNV * dotNV));
        float visL = dotNL + sqrt(alpha2 + (1.0 - alpha2) * (dotNL * dotNL)); 
        float Visibility = 1.0 / (visV * visL); 

        // 3. Fresnel F (Schlick)
        vec3 f0 = mix(vec3(0.04), u_BaseColor, u_Metallic);//0.04=((1-ior)/(1+ior))^2
        vec3 F = f0 + (1.0 - f0) * pow(1.0 - abs(dotVH), 5.0); 

        // 4. Combinaison Diffuse et Spéculaire
        vec3 f_specular = F * D * Visibility; 
        vec3 c_diff = mix(u_BaseColor, vec3(0.0), u_Metallic); 
        vec3 f_diffuse = (vec3(1.0) - F) * (1.0 / PI) * c_diff;

        // 5. Shadow 
        float shadow = (i == 0) ? ShadowCalculation(FragPosLightSpace, N, L) : 0.0;    

        Lo+= (f_diffuse + f_specular) * dotNL*(1.0-shadow); 
    }
    // Tonemapping et correction gamma
    vec3 color = Lo;
    color = color / (color + vec3(1.0));
    FragColor = vec4(pow(color, vec3(1.0/2.2)), 1.0);
}