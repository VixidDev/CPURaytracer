#version 420

layout(binding = 0) uniform sampler2D raytracerImage;

in vec2 v2fTexCoord;

out vec3 colour;

void main() {
    colour = texture(raytracerImage, v2fTexCoord).rgb;
}