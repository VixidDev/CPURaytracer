#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Homogeneous4.h"
#include "Ray.h"
#include "Material.h"
#include "RGBAImage.h"

class Triangle
{


public:
    int triangle_id;
    Homogeneous4 verts[3];
    Homogeneous4 normals[3];
    Homogeneous4 colors[3];
    Cartesian3 uvs[3];

    Material *shared_material;

    Triangle();
    void validate(int id);
    bool isValid();

    float intersect(Ray ray);
    Cartesian3 barycentric(Cartesian3 o);
    Homogeneous4 phong(Homogeneous4 lightPos, Homogeneous4 lightColour, Cartesian3 intersection, bool inShadow);

};

#endif // TRIANGLE_H
