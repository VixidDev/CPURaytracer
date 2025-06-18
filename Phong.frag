#version 330 core

#define MAX_NR_POINT_LIGHTS 3

// Interolated values from the vertex shaders
in vec2 UV;
in vec4 vertexPosition_cameraspace;
in vec3 eyeDirection_cameraspace;
in vec3 normal_cameraspace;

// Output data
out vec3 colour;

struct Material {
    vec3 emissiveColour;
    vec3 diffuseColour;
    vec3 ambientColour;
    vec3 specularColour;
    float shininess;
};

struct PointLight {
    vec3 position;
    vec3 colour;
};

uniform mat4 V;
uniform mat4 M;
uniform Material meshMaterial;
uniform PointLight[MAX_NR_POINT_LIGHTS] lights;
uniform int nLights;

void main() {
    colour = meshMaterial.emissiveColour;

    for (int i = 0; i < nLights; i++) {
        vec3 n = normalize(normal_cameraspace);
        vec4 lp = V * M * vec4(lights[i].position, 1);
        vec3 l = normalize(lp.xyz / lp.w - vertexPosition_cameraspace.xyz / vertexPosition_cameraspace.w);
        vec3 e = normalize(eyeDirection_cameraspace);

        // Diffuse
        float cosTheta = clamp(dot(n, l), 0, 1);
        vec3 diffuse = meshMaterial.diffuseColour * lights[i].colour * cosTheta;
        vec3 ambient = meshMaterial.ambientColour * lights[i].colour;

        // Specular
        vec3 B = normalize(l + e);
        float cosB = clamp(dot(n, B), 0, 1);
        cosB = clamp(pow(cosB, meshMaterial.shininess), 0, 1);
        cosB = cosB * cosTheta * (meshMaterial.shininess + 2) / (2 * radians(180.0f));
        vec3 specular = meshMaterial.specularColour * lights[i].colour * cosB;
        
        colour = colour + ambient + diffuse + specular;
    }
}