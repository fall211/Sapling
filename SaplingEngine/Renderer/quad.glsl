

//
// VERTEX SHADER
//
@vs vs
in vec4 position0;
in vec4 color0;
in vec2 uv0;
in vec4 color_override0;

out vec4 color;
out vec2 uv;
out vec4 color_override;


void main() {
	gl_Position = position0;
	color = color0;
	uv = uv0;
	color_override = color_override0;
}
@end


//
// FRAGMENT SHADER
//
@fs fs
layout(binding=0) uniform texture2D tex0;
layout(binding=0) uniform sampler default_sampler;

in vec4 color;
in vec2 uv;
in vec4 color_override;

out vec4 col_out;

void main() {

    vec4 tex_col = texture(sampler2D(tex0, default_sampler), uv);

	// int tex_index = int(bytes.x * 255.0);
	
	// vec4 tex_col = vec4(1.0);
	// if (tex_index == 0) {
	// 	tex_col = texture(sampler2D(tex0, default_sampler), uv);
	// } 
	// else if (tex_index == 1) {
	// 	// this is text, it's only got the single .r channel so we stuff it into the alpha
	// 	tex_col.a = texture(sampler2D(tex1, default_sampler), uv).r;
	// }
	
	col_out = tex_col;
	col_out *= color;
	
	col_out.rgb = mix(col_out.rgb, color_override.rgb, color_override.a);
}
@end

@program quad vs fs
