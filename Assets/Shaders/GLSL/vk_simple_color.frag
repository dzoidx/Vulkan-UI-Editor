#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 1) uniform sampler sam;
layout (set = 0, binding = 2) uniform texture2D textures[8];

layout(push_constant) uniform PushConsts {
	int textureIdx;
} pushConsts;

layout (location=0) in vec4 inColour;
layout (location=1) in vec2 fragUV;
layout (location=0) out vec4 outFragColor;

void main()
{
	if(pushConsts.textureIdx < 0)
		outFragColor = inColour;
	else
		outFragColor = inColour * texture(sampler2D(textures[pushConsts.textureIdx], sam), fragUV);
}