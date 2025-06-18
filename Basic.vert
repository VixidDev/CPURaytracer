#version 330 core

// Input vertex data, different for all executions of this shader
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;

// Output data; will be interpolated for each fragment
out vec2 UV;
out vec4 vertexPosition_cameraspace;
out vec3 eyeDirection_cameraspace;
out vec3 normal_cameraspace;

// Values that stay constant for whole mesh
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;

void main() {
    // Output position of the vertex, in clip space : MVP * position
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

    // UV of the vertex
    UV = vertexUV;

    // Model to camera = ModelView
    normal_cameraspace = normalize((V * M * vec4(vertexNormal_modelspace, 0)).xyz);

    // Position of the vertex, in worldspace : M * position
    vertexPosition_cameraspace = (V * M  * vec4(vertexPosition_modelspace, 1));

    // Eye vector. Eye is at 0 given this is camera space.
    eyeDirection_cameraspace = normalize((vec4(0, 0, 0, 1) - vertexPosition_cameraspace).xyz);
}