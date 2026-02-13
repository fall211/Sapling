
//
// VERTEX SHADER - 3D Skinned Mesh with multi-light Blinn-Phong lighting + TBN
//
@vs mesh3d_skinned_vs

layout(binding=0) uniform mesh3d_skinned_vs_params {
    mat4 mvp;
    mat4 model;
    vec4 normal_mat_c0;  // column 0 of mat3(transpose(inverse(model)))
    vec4 normal_mat_c1;  // column 1
    vec4 normal_mat_c2;  // column 2
    mat4 bone_matrices[64];  // max 64 bones per mesh
};

in vec3 position0;
in vec3 normal0;
in vec2 texcoord0;
in vec4 tangent0;    // xyz = tangent, w = handedness
in vec4 bone_indices0;   // bone indices (as floats for GPU compatibility)
in vec4 bone_weights0;   // bone weights

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_uv;
out vec3 frag_tangent;
out vec3 frag_bitangent;

void main() {
    // Compute skinned position, normal, and tangent
    vec4 skinned_pos = vec4(0.0);
    vec3 skinned_normal = vec3(0.0);
    vec3 skinned_tangent = vec3(0.0);

    for (int i = 0; i < 4; i++) {
        int bone_idx = int(bone_indices0[i]);
        float weight = bone_weights0[i];
        if (weight > 0.0) {
            mat4 bone_transform = bone_matrices[bone_idx];
            skinned_pos += bone_transform * vec4(position0, 1.0) * weight;
            skinned_normal += mat3(bone_transform) * normal0 * weight;
            skinned_tangent += mat3(bone_transform) * tangent0.xyz * weight;
        }
    }

    vec4 world_pos = model * skinned_pos;
    frag_pos = world_pos.xyz;

    mat3 normal_mat = mat3(
        normal_mat_c0.xyz,
        normal_mat_c1.xyz,
        normal_mat_c2.xyz
    );
    frag_normal = normalize(normal_mat * skinned_normal);
    frag_tangent = normalize(normal_mat * skinned_tangent);
    frag_bitangent = cross(frag_normal, frag_tangent) * tangent0.w;

    frag_uv = texcoord0;
    gl_Position = mvp * skinned_pos;
}
@end


//
// FRAGMENT SHADER - Multi-light Blinn-Phong with normal/emissive/roughness maps (same as mesh3d.glsl)
//
@fs mesh3d_skinned_fs

layout(binding=1) uniform mesh3d_skinned_fs_params {
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

    // roughness
    float roughness = texture(sampler2D(roughness_tex, default_sampler), frag_uv).r;
    float effective_shininess = shininess * max(1.0 - roughness, 0.01);

    vec3 view_dir = normalize(camera_pos - frag_pos);

    int num_lights = int(light_count.x);

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
            light_dir = normalize(light_pos_type[i].xyz);
            if (i == 0) ambient_color = l_color;
        } else {
            vec3 to_light = light_pos_type[i].xyz - frag_pos;
            float dist = length(to_light);
            light_dir = -normalize(to_light);
            float range = light_color_range[i].w;
            attenuation = 1.0 / (1.0 + (dist * dist) / (range * range));
        }

        float diff = max(dot(norm, -light_dir), 0.0);
        total_diffuse += diff * l_color * attenuation;

        vec3 halfway = normalize(-light_dir + view_dir);
        float spec = pow(max(dot(norm, halfway), 0.0), effective_shininess);
        total_specular += specular_strength * spec * l_color * attenuation;
    }

    vec3 ambient = ambient_strength * ambient_color;
    vec3 result = (ambient + total_diffuse + total_specular) * tex_col.rgb * base_color.rgb + emissive;
    col_out = vec4(result, tex_col.a * base_color.a);
}
@end

@program mesh3d_skinned mesh3d_skinned_vs mesh3d_skinned_fs
