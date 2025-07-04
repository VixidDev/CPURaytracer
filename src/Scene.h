#ifndef SCENE_H
#define SCENE_H

#include "Homogeneous4.h"
#include "ThreeDModel.h"
#include <vector>
#include "Ray.h"
#include "Triangle.h"
#include "Material.h"

class Scene
{
public:

   struct CollisionInfo {
    Triangle tri;
    float t;
   };

   CollisionInfo closestTriangle(Ray r);

    std::vector<ThreeDModel>* objects;
    RenderParameters* rp;
    Material *default_mat;

    std::vector<Triangle> triangles;

    Scene(std::vector<ThreeDModel> *texobjs,RenderParameters *renderp);
    void updateScene();
    Matrix4 getModelview();
};

#endif // SCENE_H
