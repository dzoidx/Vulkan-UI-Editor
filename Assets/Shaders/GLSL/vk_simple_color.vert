#version 450

layout (location=0) in vec4 position;
layout (location=1) in vec4 color;
layout (location=2) in vec2 uv;

layout (set = 0, binding = 0) uniform mats_
{
	mat4 view;
} mats;

out gl_PerVertex {
	vec4 gl_Position;
};

layout (location=0) out vec4 outColour;
layout (location=1) out vec2 fragUV;

void main()
{
    gl_Position = mats.view * vec4(position.xyz, 1.0);
	outColour = color;
	fragUV = uv;
}