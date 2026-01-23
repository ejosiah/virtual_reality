#ifndef LIGHT_GLSL
#define LIGHT_GLSL

#define LIGHT_TYPE_DIRECTION 0
#define LIGHT_TYPE_POSITION  1

struct Light {
    vec3 color;
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    int type;
};


vec3 computeRadiance(Light light, Surface surface, vec3 viewPosition) {
    vec3 N = normalize(surface.normal);
    vec3 V = normalize(viewPosition - surface.point);

    vec3 L;
    float dist2 = 1.0;

    if (light.type == LIGHT_TYPE_DIRECTION) {
        L = normalize(light.direction);
        dist2 = 1.0;
    } else { // LIGHT_TYPE_POSITION
        vec3 toLight = light.position - surface.point;
        dist2 = max(dot(toLight, toLight), 1e-6);
        L = toLight * inversesqrt(dist2);
    }

    float NdotL = max(dot(N, L), 0.0);

    vec3 H = normalize(V + L);
    float specPow = max(surface.shininess, 0.0);
    float specTerm = (NdotL > 0.0) ? pow(max(dot(N, H), 0.0), specPow) : 0.0;

    float spot = 1.0;
    if (light.type == LIGHT_TYPE_POSITION && light.outerCutOff > 0.0) {
        vec3 S = normalize(light.direction);
        // angle between spotlight axis and direction from light -> surface
        float theta = dot(normalize(surface.point - light.position), S);

        float inner = light.cutOff;
        float outer = light.outerCutOff;

        // supports either cos-angle cutoffs or raw angles (in radians) if user passed >1 values
        if (inner > 1.0 || outer > 1.0) {
            inner = cos(inner);
            outer = cos(outer);
        }

        float eps = max(inner - outer, 1e-6);
        spot = clamp((theta - outer) / eps, 0.0, 1.0);
    }

    float attenuation = (light.type == LIGHT_TYPE_POSITION) ? (1.0 / dist2) : 1.0;

    vec3 ambient  = dot(surface.ambient, surface.ambient) != 0 ? surface.ambient : surface.diffuse;
    vec3 diffuse  = surface.diffuse  * NdotL;
    vec3 specular = surface.specular * specTerm;

    vec3 radiance = (diffuse + specular) * light.color * attenuation * spot;
    radiance += ambient * 0.1 + surface.emission;

    return radiance;
}

#endif // LIGHT_GLSL