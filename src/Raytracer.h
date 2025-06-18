// include guard
#ifndef RAYTRACE_RENDER_WIDGET_H
#define RAYTRACE_RENDER_WIDGET_H

#include <vector>
#include <mutex>
#include <atomic>

// and include all of our own headers that we need
#include "ThreeDModel.h"
#include "RenderParameters.h"
#include "Scene.h"

class Raytracer 										
	{ 
	
	private:	
	// the geometric object to be rendered
    std::vector<ThreeDModel> *texturedObjects;
	// the render parameters to use
	RenderParameters *renderParameters;
	// An image to use as a framebuffer
    //A friendly Scene representation that we control
    Scene raytraceScene;

	public:
	// constructor
	Raytracer
			(
	 		// the geometric object to show
            std::vector<ThreeDModel> 		*newTexturedObject,
			// the render parameters to use
			RenderParameters 	*newRenderParameters
			);
	
	// destructor
	~Raytracer();
	
	void resize(int w, int h);
	void stopRaytracer();
	RGBAImage frameBuffer;

	Ray calculateRay(int pixelX, int pixelY, bool perspective);
	Homogeneous4 TraceAndShadeWithRay(Ray ray, int bounces, float reflectivity, bool hitLight);
	Ray reflectRay(Ray ray, Cartesian3 normal, Cartesian3 hitPoint);
	Ray refractRay(Ray ray, Cartesian3 normal, Cartesian3 hitPoint, float surfaceIOR, float currentIOR);
	float fresnel(float currentIOR, float surfaceIOR, Ray ray, Cartesian3 normal);
	Cartesian3 monteCarlo3DHemisphere(Cartesian3 normal);

	protected:

	// called every time the widget needs painting

	public:

    // routine that generates the image
    void Raytrace();
    //threading stuff
    void RaytraceThread();
    private:

	std::atomic<bool> raytracingRunning;
	std::atomic<bool> restartRaytrace;

	}; // class RaytraceRenderWidget

#endif
