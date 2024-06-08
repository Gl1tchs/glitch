#ifndef SKY_LIGHT_GLSL
#define SKY_LIGHT_GLSL

#include "material.glsl"

struct SkyLight {
    vec3 direction;
    vec3 color;
};

vec3 calc_sky_light(SkyLight p_light, Material p_material, vec3 p_normal,
    vec3 p_view_dir) {
    vec3 light_dir = normalize(-p_light.direction);
    vec3 halfway_dir = normalize(light_dir + p_view_dir);

    float diff = max(dot(p_normal, light_dir), 0.0);
    float spec = pow(max(dot(p_view_dir, halfway_dir), 0.0), p_material.shininess);

    vec3 ambient = 0.3 * p_light.color;
    vec3 diffuse = p_light.color * diff;
    vec3 specular = spec * p_light.color;

    return (ambient + diffuse + specular) * p_material.diffuse;
}

#endif
