#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D u_image;

void main() {
	outColor = vec4(in_color, 1.0);
	outColor = vec4(1,1,1, 1.0);
	outColor = texture(u_image,uv);
	outColor = vec4(uv,0,1);
}