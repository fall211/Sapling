
//
// VERTEX SHADER - 3D Mesh with multi-light Blinn-Phong lighting + TBN for normal mapping
//
@vs mesh3d_vs

layout(binding=0) uniform mesh3d_vs_params {
    mat4 mvp;
    mat4 model;
    vec4 normal_mat_c0;  // column 0 of mat3(transpose(inverse(model)))
    vec4 normal_mat_c1;  // column 1
    vec4 normal_mat_c2;  // column 2
};

in vec3 position0;
in vec3 normal0;
in vec2 texcoord0;
in vec4 tangent0;  // xyz = tangent, w = handedness

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_uv;
out vec3 frag_tangent;
out vec3 frag_bitangent;

void main() {
    vec4 world_pos = model * vec4(position0, 1.0);
    frag_pos = world_pos.xyz;

    mat3 normal_mat = mat3(
        normal_mat_c0.xyz,
        normal_mat_c1.xyz,
        normal_mat_c2.xyz
    );
    frag_normal = normalize(normal_mat * normal0);
    frag_tangent = normalize(normal_mat * tangent0.xyz);
    frag_bitangent = cross(frag_normal, frag_tangent) * tangent0.w;

    frag_uv = texcoord0;
    gl_Position = mvp * vec4(position0, 1.0);
}
@end


//
// FRAGMENT SHADER - Multi-light Blinn-Phong with normal/emissive/roughness maps
//
@fs mesh3d_fs

layout(binding=1) uniform mesh3d_fs_params {
    vec4 camera_pos_and_ambient;    // xyz = camera position, w = ambient strength
    vec4 base_color;                // rgba base color
    vec4 material_props;            // x = specular strength, y = shininess, z = emissive strength, w = unused
    vec4 light_pos_type[8];         // xyz = position/direction, w = type (0=directional, 1=point)
    vec4 light_color_range[8];      // xyz = color*intensity, w = range (for point lights)
    vec4 light_count;               // x = num active lights, yzw = unused
};

layout(binding=0) uniform texture2D diffuse_tex;
layout(binding=1) uniform texture2D normal_tex;
layout(binding=2) uniform texture2D emissive_tex;
layout(binding=3) uniform texture2D roughness_tex;
layout(binding=0) uniform sampler default_sampler;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_uv;
in vec3 frag_tangent;
in vec3 frag_bitangent;

out vec4 col_out;

void main() {
    vec3 camera_pos = camera_pos_and_ambient.xyz;
    float ambient_strength = camera_pos_and_ambient.w;
    float specular_strength = material_props.x;
    float shininess = material_props.y;
    float emissive_strength = material_props.z;

    vec4 tex_col = texture(sampler2D(diffuse_tex, default_sampler), frag_uv);

    // TBN matrix for normal mapping
    mat3 TBN = mat3(normalize(frag_tangent), normalize(frag_bitangent), normalize(frag_normal));
    vec3 normal_sample = texture(sampler2D(normal_tex, default_sampler), frag_uv).rgb;
    normal_sample = normal_sample * 2.0 - 1.0;
    vec3 norm = normalize(TBN * normal_sample);

    // emissive
    vec3 emissive = texture(sampler2D(emissive_tex, default_sampler), frag_uv).rgb * emissive_strength;

    // roughness — invert to get a smoothness-like shininess multiplier
    float roughness = texture(sampler2D(roughness_tex, default_sampler), frag_uv).r;
    float effective_shininess = shininess * max(1.0 - roughness, 0.01);

    vec3 view_dir = normalize(camera_pos - frag_pos);

    int num_lights = int(light_count.x);

    // accumulate lighting from all active lights
    vec3 total_diffuse = vec3(0.0);
    vec3 total_specular = vec3(0.0);
    vec3 ambient_color = vec3(1.0);

    for (int i = 0; i < 8; i++) {
        if (i >= num_lights) break;

        vec3 l_color = light_color_range[i].xyz;
        float l_type = light_pos_type[i].w;
        float attenuation = 1.0;
        vec3 light_dir;

        if (l_type < 0.5) {
            // directional light
            light_dir = normalize(light_pos_type[i].xyz);
            if (i == 0) ambient_color = l_color;
        } else {
            // point light
            vec3 to_light = light_pos_type[i].xyz - frag_pos;
            float dist = length(to_light);
            light_dir = -normalize(to_light);
            float range = light_color_range[i].w;
            attenuation = 1.0 / (1.0 + (dist * dist) / (range * range));
        }

        // diffuse
        float diff = max(dot(norm, -light_dir), 0.0);
        total_diffuse += diff * l_color * attenuation;

        // specular (Blinn-Phong)
        vec3 halfway = normalize(-light_dir + view_dir);
        float spec = pow(max(dot(norm, halfway), 0.0), effective_shininess);
        total_specular += specular_strength * spec * l_color * attenuation;
    }

    vec3 ambient = ambient_strength * ambient_color;
    vec3 result = (ambient + total_diffuse + total_specular) * tex_col.rgb * base_color.rgb + emissive;
    col_out = vec4(result, tex_col.a * base_color.a);
}
@end

@program mesh3d mesh3d_vs mesh3d_fs
