#include <math.h>
#include <thread>
#include <random>
#include <omp.h>
#include <algorithm>
// include the header file
#include "Raytracer.h"

#define PI 3.14159265359f

#define N_THREADS 16
#define N_LOOPS 600
#define N_BOUNCES 10
#define TERMINATION_FACTOR 0.35f
#define MONTE_CARLO_RAYS 1
#define ANTI_ALIAS_SAMPLES 1

// constructor
Raytracer::Raytracer(std::vector<ThreeDModel> *newTexturedObject, RenderParameters *newRenderParameters):
    texturedObjects(newTexturedObject),
    renderParameters(newRenderParameters),
    raytraceScene(texturedObjects,renderParameters)
    { 
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        restartRaytrace = false;
        raytracingRunning = false;
    }     


Raytracer::~Raytracer()
    { 
    // all of our pointers are to data owned by another class
    // so we have no responsibility for destruction
    }                                                                  


// called every time the widget is resized
void Raytracer::resize(int w, int h)
    { // RaytraceRenderWidget::resizeGL()
    // resize the render image
    frameBuffer.Resize(w, h);
    } // RaytraceRenderWidget::resizeGL()
    
void Raytracer::stopRaytracer() {
    restartRaytrace = true;
    while (raytracingRunning) {
        std::chrono::milliseconds timespan(10);
        std::this_thread::sleep_for(timespan);
    }
    restartRaytrace = false;
}

inline
float linear_from_srgb(std::uint8_t aValue) noexcept
{
    float const fvalue = float(aValue) / 255.f;

    if (fvalue < 0.04045f)
        return (1.f / 12.92f) * fvalue;

    return std::pow((1.f / 1.055f) * (fvalue + 0.055f), 2.4f);

}

inline
std::uint8_t linear_to_srgb(float aValue) noexcept
{
    if (aValue < 0.0031308f)
        return std::uint8_t(255.f * 12.92f * aValue + 0.5f);
    return std::uint8_t(255.f * (1.055f * std::pow(aValue, 1.f / 2.4f) - 0.055f) + 0.5f);
}

void Raytracer::RaytraceThread()
{
    //Tutorial code here!
    for (int j = 0; j < frameBuffer.height; j++) {
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < frameBuffer.width; i++) {
            Homogeneous4 colour;

            // Anti-aliasing
            for (int s = 0; s < ANTI_ALIAS_SAMPLES; s++) {
                // Calculate initial ray
                Ray ray = calculateRay(i, j, !renderParameters->orthoProjection);
                // Raytrace
                colour = colour + TraceAndShadeWithRay(ray, N_BOUNCES, 1.0f, false);
            }

            colour = colour / float(ANTI_ALIAS_SAMPLES);
            

            // Clamp colours to 0->1
            colour.x = std::clamp(colour.x, 0.0f, 1.0f);
            colour.y = std::clamp(colour.y, 0.0f, 1.0f);
            colour.z = std::clamp(colour.z, 0.0f, 1.0f);
            colour.w = std::clamp(colour.w, 0.0f, 1.0f);

            frameBuffer[j][i] = RGBAValue(
                linear_to_srgb(colour.x),
                linear_to_srgb(colour.y),
                linear_to_srgb(colour.z),
                255);

        }
    }
    if (restartRaytrace) {
        raytracingRunning = false;
        return;
    }

    raytracingRunning = false;
}

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0, 1);

Homogeneous4 Raytracer::TraceAndShadeWithRay(Ray ray, int bounces, float currentIOR, bool hitLight) {
    Homogeneous4 colour(0.0f, 0.0f, 0.0f, 0.0f);
    
    // If we ran out of bounces return black colour
    if (bounces <= 0) return colour;

    // Do russian roulette to possibly terminate rays,
    // only do on secondary rays to not possibly lose much detail
    if (renderParameters->monteCarloEnabled && ray.ray_type != Ray::Type::primary && distribution(generator) < TERMINATION_FACTOR)
        return colour;

    // Follow ray to find closest triangle
    Scene::CollisionInfo ci = raytraceScene.closestTriangle(ray);

    // If we hit something
    if (ci.t > 0.0f) {
        // Calculate where our ray hit
        Cartesian3 hitPoint = ray.origin + ray.direction * ci.t;
        // Find barycentric coordinates of where the ray hit the triangle
        Cartesian3 bary = ci.tri.barycentric(hitPoint);
        // Calculate interpolated normal on triangle
        Cartesian3 normal = (bary.x * ci.tri.normals[0].Vector() + bary.y * ci.tri.normals[1].Vector() + bary.z * ci.tri.normals[2].Vector()).unit();

        // Hit material properties
        float surfaceReflectivity = ci.tri.shared_material->reflectivity;
        float surfaceTransparency = ci.tri.shared_material->transparency;
        // If the triangle we hit has an IOR matching our current IOR then is it most likely the case we are exiting that object and going to air
        float IOR = (currentIOR == ci.tri.shared_material->indexOfRefraction) ? 1.0f : ci.tri.shared_material->indexOfRefraction;

        // Do NEE by tracking if the hit material is light and we have not yet hit a light for this ray's path
        // and if so then we return the lights emissive colour
        if (ci.tri.shared_material->isLight() && !hitLight) {
            hitLight = true;
            return ci.tri.shared_material->emissive;
        }

        if (renderParameters->interpolationRendering)
            return Homogeneous4(std::abs(normal.x), std::abs(normal.y), std::abs(normal.z), 255);

        if (renderParameters->phongEnabled) {

            for (Light* l : renderParameters->lights) {
                // Transform light position to view space
                Homogeneous4 transformedLightPos = raytraceScene.getModelview() * (renderParameters->monteCarloEnabled ? l->GetPosition() : l->GetPositionCenter());

                bool inShadow = false;

                // Do shadows
                if (renderParameters->shadowsEnabled) {
                    // Calculate direction to light and normalise
                    Cartesian3 dirToLight = (transformedLightPos.Point() - hitPoint).unit();
                    // Offset hit point based on the triangle's normal
                    Cartesian3 biasedHitPoint = hitPoint + normal * 0.001f;
                    // Create ray
                    Ray shadowRay(biasedHitPoint, dirToLight, Ray::Type::shadow);

                    // Fire ray into scene and get closest triangle
                    Scene::CollisionInfo ci2 = raytraceScene.closestTriangle(shadowRay);

                    // If we hit something and it is closer than the light we are looping over, then we are in shadow
                    if (ci2.t > 0 && ci2.t < (transformedLightPos.Point() - biasedHitPoint).length())
                        inShadow = true;
                }

                colour = colour + ci.tri.phong(transformedLightPos, l->GetColor(), bary, inShadow);
            }

            // Reflection and refraction cases, only run either if fresnel rendering is also off
            if (!renderParameters->fresnelRendering && renderParameters->reflectionEnabled && surfaceReflectivity > 0.0f) {
                Ray reflectedRay = reflectRay(ray, normal, hitPoint);

                return surfaceReflectivity * TraceAndShadeWithRay(reflectedRay, --bounces, currentIOR, hitLight) + (1 - surfaceReflectivity) * colour;
            }

            if (!renderParameters->fresnelRendering && renderParameters->refractionEnabled && surfaceTransparency > 0.0f) {
                Ray refractedRay = refractRay(ray, normal, hitPoint, IOR, currentIOR);

                return surfaceTransparency * TraceAndShadeWithRay(refractedRay, --bounces, IOR, hitLight) + (1 - surfaceTransparency) * colour;
            }

            // Fresnel rendering
            if (renderParameters->fresnelRendering && (surfaceReflectivity > 0.0f || surfaceTransparency > 0.0f)) {
                // Get fresnel multiplier
                float fresnelMult = fresnel(currentIOR, IOR, ray, normal);

                // Get respective reflectivity and transparency
                float reflectivity = surfaceReflectivity * fresnelMult;
                float transparency = surfaceTransparency * (1 - fresnelMult);

                // Cast both reflect and refract rays
                Ray reflectedRay = reflectRay(ray, normal, hitPoint);
                Ray refractedRay = refractRay(ray, normal, hitPoint, IOR, currentIOR);

                // Caclulate colour
                return reflectivity * TraceAndShadeWithRay(reflectedRay, --bounces, currentIOR, hitLight) + transparency * TraceAndShadeWithRay(refractedRay, --bounces, IOR, hitLight); // last trace call use IOR maybe?
            }

            // Indirect lighting (ambient)
            if (renderParameters->monteCarloEnabled) {
                Homogeneous4 indirectColour(0, 0, 0, 1);
                for (int i = 0; i < MONTE_CARLO_RAYS; i++) { // Setting MONTE_CARLO_RAYS to greater than 1 makes the scene very black and dark not too sure why
                    // Sample random position in hemisphere
                    Cartesian3 randomDir = monteCarlo3DHemisphere(normal).unit();
                    Ray monteCarloRay(hitPoint + randomDir * 0.0001f, randomDir, Ray::Type::secondary);

                    // Trace montecarlo ray
                    Homogeneous4 endColor = TraceAndShadeWithRay(monteCarloRay, --bounces, currentIOR, hitLight);
                    
                    Scene::CollisionInfo ci2 = raytraceScene.closestTriangle(monteCarloRay);
                    if (ci2.t > 0.0f) {
                        // Get hit montecarlo hit position
                        Cartesian3 hitP = monteCarloRay.origin + monteCarloRay.direction * ci2.t;
                        // Get the shading for this point based on the colour we received with some part of the ambient(?)
                        indirectColour = indirectColour + ci.tri.phong(hitP, endColor, bary, false).modulate(ci.tri.shared_material->ambient);
                    }
                }
                // Divide by our PDF
                float pdf = MONTE_CARLO_RAYS / (2 * PI);
                indirectColour = indirectColour / pdf;

                // Add it to final colour
                colour = colour + indirectColour;
            }
            // If montecarlo is not enabled just use ambient colour for indirect lighting
            else {
                colour = colour + ci.tri.shared_material->ambient;
            }
        }
    }

    return colour;
}

Ray Raytracer::reflectRay(Ray ray, Cartesian3 normal, Cartesian3 hitPoint) {
    // Calculate ray direction
    Cartesian3 rayDir = (ray.direction - 2.0f * normal.dot(ray.direction) * normal).unit();
    return Ray(hitPoint + normal * 0.001f, rayDir, Ray::Type::secondary);
}

Ray Raytracer::refractRay(Ray ray, Cartesian3 normal, Cartesian3 hitPoint, float surfaceIOR, float currentIOR) {
    Cartesian3 incidentDir = ray.direction.unit();

    float cosi = incidentDir.dot(normal);
    float tempIncidentIOR = currentIOR;
    float tempRefractedIOR = surfaceIOR;
    Cartesian3 tempNorm = normal;

    if (cosi < 0.0f) {
        cosi = -cosi;
    }
    else {
        std::swap(tempIncidentIOR, tempRefractedIOR);
        tempNorm = normal * -1;
    }

    float IORratio = tempIncidentIOR / (tempRefractedIOR + 0.0001f);
    float k = 1.0f - IORratio * IORratio * (1.0f - cosi * cosi);

    // Check for TIR (Total Internal Reflection)
    if (k < 0.0f) {
        return reflectRay(ray, tempNorm, hitPoint);
    }

    return Ray(hitPoint + incidentDir * 0.001f, (IORratio * incidentDir + (IORratio * cosi - std::sqrt(k)) * tempNorm).unit(), Ray::Type::secondary);
}

// Fresnel Schlick Approximation
float Raytracer::fresnel(float currentIOR, float surfaceIOR, Ray ray, Cartesian3 normal) {
    Cartesian3 incidentDir = ray.direction.unit();

    float r = (currentIOR - surfaceIOR) / (currentIOR + surfaceIOR);
    float r0 = r * r;
    
    float cosi = incidentDir.dot(normal);
    float x = 1.0f - std::abs(cosi);

    return r0 + (1.0f - r0) * x * x * x * x * x;
}

Cartesian3 Raytracer::monteCarlo3DHemisphere(Cartesian3 normal) {
    // Get random x and y
    float x = distribution(generator);
    float y = distribution(generator);

    // Get angles from x and y
    float theta = std::acos(1 - x);
    float phi = 2 * PI * y;

    // Convert polar coordinates to cartesian coordinates
    Cartesian3 randomDir(
        std::sin(theta) * std::cos(phi),
        std::cos(theta),
        std::sin(theta) * std::sin(phi)
    );

    // Form basis
    Cartesian3 nt = Cartesian3(normal.z, 0, -normal.x);
    // Handle the case where normal = (0, 1, 0)
    if (nt.length() < 1e-10)
        nt = Cartesian3(1.0f, 0.0f, 0.0f);
    Cartesian3 ns = normal.cross(nt);

    // Make rotation matrix
    Matrix4 rotationMatrix;
    rotationMatrix.SetIdentity();
    // N_t column
    rotationMatrix[0][0] = nt.x;
    rotationMatrix[1][0] = nt.y;
    rotationMatrix[2][0] = nt.z;
    // N column
    rotationMatrix[0][1] = normal.x;
    rotationMatrix[1][1] = normal.y;
    rotationMatrix[2][1] = normal.z;
    // N_s column
    rotationMatrix[0][2] = ns.x;
    rotationMatrix[1][2] = ns.y;
    rotationMatrix[2][2] = ns.z;

    // Apply rotation to random dir to align with normal
    return rotationMatrix * randomDir;
}

Ray Raytracer::calculateRay(int pixelX, int pixelY, bool perspective) {
    Cartesian3 pos, rayDirection;

    // Anti-aliasing by getting random position in pixel
    float dx = renderParameters->monteCarloEnabled ? distribution(generator) : 0.5f;
    float dy = renderParameters->monteCarloEnabled ? distribution(generator) : 0.5f;

    int width = frameBuffer.width;
    int height = frameBuffer.height;

    float i_ndcs = ((((float)pixelX + dx) / (float)width) - 0.5) * 2;
    float j_ndcs = ((((float)pixelY + dy) / (float)height) - 0.5) * 2;

    float aspect = (float)width / (float)height;
    
    float x = i_ndcs;
    float y = j_ndcs;    

    if (perspective) {
        float tanOver2 = std::tan(renderParameters->fov / 2);
        x *= tanOver2;
        y *= tanOver2;

        x *= aspect;

        pos = Cartesian3(0, 0, 0);
        rayDirection = Cartesian3(x, y, 1);
        rayDirection = rayDirection.unit();
    } else {
        if (aspect > 1.0)
            x *= aspect;
        else
            y /= aspect;

        pos = Cartesian3(x, y, 0);
        rayDirection = Cartesian3(0, 0, 1);
    }

    return Ray(pos, rayDirection, Ray::Type::primary);
}


    // routine that generates the image
void Raytracer::Raytrace()
{ // RaytraceRenderWidget::Raytrace()
    stopRaytracer();
    //To make our lifes easier, lets calculate things on VCS.
    //So we need to process our scene to get a triangle soup in VCS.
    raytraceScene.updateScene();
    frameBuffer.clear(RGBAValue(0.0f, 0.0f, 0.0f,1.0f));
    std::thread raytracingThread(&Raytracer::RaytraceThread,this);
    raytracingThread.detach();
    raytracingRunning = true;
} // RaytraceRenderWidget::Raytrace()
    


