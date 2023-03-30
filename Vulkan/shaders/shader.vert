#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)

vec2 positions[3] = vec2[](
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

layout(set = 0, binding = 0) uniform UBO {
	mat4 view;
	mat4 projection;
	mat4 viewProjection;
	mat4 world;
	float t;
	float time;
	float _Sign;
} cameraData;


layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv;
layout(location = 2) out float t;
layout(location = 3) out float time;
layout(location = 4) out float _Sign;

void main() 
{
	// mat4 world;
	// world[0]= vec4(1,0,0,0);
	// world[1]= vec4(0,1,0,0);
	// world[2]= vec4(0,0,1,0);
	// world[3]= vec4(0,0,-1,1);
	// world  = mat4
	// (1,0,0,0,
	//  0,1,0,0,	
	//  0,0,1,0,
	//  0,0,10,1
	// );
	gl_Position = cameraData.viewProjection*cameraData.world*vec4(in_pos, 0, 1.0);
	//gl_Position/=gl_Position.w;
	fragColor = in_color;
	vec2 inverted_uv = vec2(in_uv.x,in_uv.y);
	uv = inverted_uv;
	t = cameraData.t;
	time = cameraData.time;
	_Sign = cameraData._Sign;
}