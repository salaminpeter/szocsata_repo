#pragma once

char VertexShaderPerPixel[] =
"#version 300 es\n"
"layout(location = 0) in vec4 vPosition;\n"
"layout(location = 1) in vec2 vTexCoord;\n"
"layout(location = 2) in vec4 vNormal;\n"
"out vec2 fTexCoord;\n"
"out vec4 NormalVector; \n"
"out vec4 FragmentPosition; \n"
"uniform mat4 MVP; \n"
"void main() {\n"
"	FragmentPosition = vPosition;\n"
"	NormalVector = vNormal;\n"
"	fTexCoord = vTexCoord;\n"
"	gl_Position = MVP * vPosition;\n"
"}";


char FragmentShaderPerPixel[] =
"#version 300 es\n"
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"in vec4 NormalVector;\n"
"in vec4 FragmentPosition;\n"
"in vec2 fTexCoord;\n"
"out vec4 fragColour;\n"
"uniform sampler2D texSampler;\n"
"uniform vec4 LightPosition; \n"
"uniform float DistanceDivider; \n"
"void main() {\n"
"	float Distance = length(LightPosition - FragmentPosition) / DistanceDivider;"
"	vec4 LightVector = normalize(LightPosition - FragmentPosition);\n"
"	float LightIntensity = max(dot(LightVector, NormalVector), 0.); \n"
"	LightIntensity = LightIntensity * (1.0 / (Distance * Distance));\n"
"	LightIntensity = clamp(LightIntensity, 0.35, 1.0);\n"
"	fragColour  = (texture(texSampler, fTexCoord) * LightIntensity);\n"
"}";

char VertexShaderBoard[] =
"#version 300 es\n"
"layout(location = 0) in vec4 vPosition;\n"
"layout(location = 1) in vec2 vTexCoord;\n"
"layout(location = 2) in vec4 vNormal;\n"
"out vec2 fTexCoord;\n"
"out float LightIntensity; \n"
"uniform mat4 MVP; \n"
"uniform vec4 LightPosition; \n"
"void main() {\n"
"	float Distance = length(LightPosition - vPosition) / 13.0;\n"
"	vec4 LightVector = normalize(LightPosition - vPosition);\n"
"	LightIntensity = max(dot(LightVector, vNormal), 0.);\n"
"	LightIntensity = LightIntensity * (1.0 / (Distance * Distance));\n"
"	LightIntensity = min(1.0, LightIntensity);\n"
"	fTexCoord = vTexCoord;\n"
"	gl_Position = MVP * vPosition;\n"
"}";

char FragmentShaderBoard[] =
"#version 300 es\n"
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"in vec2 fTexCoord;\n"
"in float LightIntensity;\n"
"out vec4 fragColour;\n"
"uniform sampler2D texSampler;\n"
"void main() {\n"
"	fragColour = texture(texSampler, fTexCoord) * LightIntensity;\n"
"}";

char VertexShaderSelection[] =
"#version 300 es\n"
"layout(location = 0) in vec4 vPosition;\n"
"uniform mat4 MVP; \n"
"void main() {\n"
"	gl_Position = MVP * vPosition;\n"
"}";

char FragmentShaderSelection[] =
"#version 300 es\n"
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"uniform vec4 Color;\n"
"out vec4 fragColour;\n"
"void main() {\n"
"	fragColour = Color;\n"
"}";

char VertexShaderTextured[] =
"#version 300 es\n"
"layout(location = 0) in vec4 vPosition;\n"
"layout(location = 1) in vec2 vTexCoord;\n"
"out vec2 fTexCoord;\n"
"out vec4 modColor;\n"
"uniform mat4 MVP; \n"
"uniform vec2 TexturePos; \n"
"uniform vec4 ModifyColor;\n"
"void main() {\n"
"	fTexCoord = vTexCoord + TexturePos;\n"
"	modColor = ModifyColor;\n"
"	gl_Position = MVP * vPosition;\n"
"}";

char FragmentShaderTextured[] =
"#version 300 es\n"
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"in vec2 fTexCoord;\n"
"in vec4 modColor;\n"
"out vec4 fragColour;\n"
"uniform sampler2D texSampler;\n"
"void main() {\n"
"   fragColour = texture(texSampler,fTexCoord) * modColor;\n"
"}";

char FragmentShaderTexturedTransparent[] =
"#version 300 es\n"
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"in vec2 fTexCoord;\n"
"out vec4 fragColour;\n"
"uniform sampler2D texSampler;\n"
"void main() {\n"
"   fragColour = vec4(texture(texSampler,fTexCoord).rgb, 0.5);\n"
"}";
