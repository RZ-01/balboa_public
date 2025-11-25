#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

uniform vec3 viewPos;

uniform vec3 dirLightDir;
uniform vec3 pointLightPos;

uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform float spotCutOff;
uniform float spotOuterCutOff;

uniform float shininess;

vec3 CalcDirectionalLight(vec3 N, vec3 V, vec3 objectColor);
vec3 CalcPointLight(vec3 N, vec3 objectColor);
vec3 CalcSpotLight(vec3 N, vec3 objectColor);

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 objectColor = Color;

    vec3 result = vec3(0.0);
    //result += CalcDirectionalLight(N, V, objectColor);
    //result += CalcPointLight(N, objectColor);
    result += CalcSpotLight(N, objectColor);

    FragColor = vec4(result, 1.0);
}


// Let’s assume ambientStrength=0.1, specularStrength=0.5
vec3 CalcDirectionalLight(vec3 N, vec3 V, vec3 objectColor)
{
    vec3 L = normalize(-dirLightDir);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * objectColor;
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), shininess);
    vec3 specular = 0.5 * spec * vec3(1.0);
    vec3 ambient = 0.1 * objectColor;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(vec3 N, vec3 objectColor)
{
    vec3 L = normalize(pointLightPos - FragPos);
    float diff = max(dot(N, L), 0.0);
    float dist = length(pointLightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);
    vec3 ambient = 0.05 * objectColor;
    vec3 diffuse = diff * objectColor;
    return (ambient + diffuse) * attenuation;
}

vec3 CalcSpotLight(vec3 N, vec3 objectColor)
{
    vec3 L = normalize(spotLightPos - FragPos);
    float diff = max(dot(N, L), 0.0);
    float theta = dot(L, normalize(-spotLightDir));
    float epsilon = spotCutOff - spotOuterCutOff;
    float intensity = clamp((theta - spotOuterCutOff) / epsilon, 0.0, 1.0);
    float dist = length(spotLightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);
    vec3 ambient = 0.02 * objectColor;
    vec3 diffuse = diff * objectColor;
    return (ambient + diffuse) * attenuation * intensity;
}
