#ifndef SKY_LIGHT_GLSL
#define SKY_LIGHT_GLSL

#include "material.glsl"

struct SkyLight {
    vec3 direction;
    vec3 color;
};

vec3 calc_sky_light(SkyLight light, Material material, vec3 normal,
    vec3 view_dir) {
    vec3 light_dir = normalize(-light.direction);
    vec3 halfway_dir = normalize(light_dir + view_dir);

    float diff = max(dot(normal, light_dir), 0.0);
    float spec = pow(max(dot(view_dir, halfway_dir), 0.0), material.shininess);

    vec3 ambient = 0.3 * light.color;
    vec3 diffuse = light.color * diff;
    vec3 specular = spec * light.color;

    return (ambient + diffuse + specular) * material.diffuse;
}

#endif
