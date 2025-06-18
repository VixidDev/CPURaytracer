#include "Triangle.h"
#include <math.h>
#include <algorithm>

Triangle::Triangle()
{
    triangle_id = -1;
    shared_material= nullptr;
}


void Triangle::validate(int id){
    triangle_id = id;
}

bool Triangle::isValid(){
    return triangle_id != -1;
}

float Triangle::intersect(Ray ray) {
    // Extract the vertices of a triangle
    Cartesian3 A = verts[0].Point();
    Cartesian3 B = verts[1].Point();
    Cartesian3 C = verts[2].Point();

    // Calculate the normal vector of a triangle
    Cartesian3 normal = (B - A).cross(C - A);

    // Calculate the intersection point of the ray with the triangular plane
    float t = (A - ray.origin).dot(normal) / ray.direction.dot(normal);

    // If t is less than zero, it means that the intersection point of the ray and the triangle plane is behind the ray origin.
    if (t < 0.0f) {
        return -1.0f;
    }

    // Calculate intersection point P
    Cartesian3 intersectionPoint = ray.origin + t * ray.direction;

    // Check if intersection point is inside triangle using half plane algorithm
    Cartesian3 edge1 = B - A;
    Cartesian3 edge2 = C - B;
    Cartesian3 edge3 = A - C;

    Cartesian3 normal1 = edge1.cross(intersectionPoint - A);
    Cartesian3 normal2 = edge2.cross(intersectionPoint - B);
    Cartesian3 normal3 = edge3.cross(intersectionPoint - C);

    // Make sure the intersection point is on the correct triangular half-plane
    if (normal1.dot(normal) >= 0.0f && normal2.dot(normal) >= 0.0f && normal3.dot(normal) >= 0.0f) {
        return t; // Within the triangle, returns the ray parameter t
    }

    return -1.0f; // not within triangle
}

Cartesian3 Triangle::barycentric(Cartesian3 o) {
    Cartesian3 bc;

    Cartesian3 p = this->verts[0].Vector();
    Cartesian3 q = this->verts[1].Vector();
    Cartesian3 r = this->verts[2].Vector();

    float areaPQR = ((q - p).cross(r - p)).length();
    float areaOPQ = ((o - p).cross(o - q)).length();
    float areaOQR = ((o - q).cross(o - r)).length();
    float areaORP = ((o - r).cross(o - p)).length();

    float alpha = areaOQR / areaPQR;
    float beta = areaORP / areaPQR;
    float gamma = areaOPQ / areaPQR;

    bc.x = alpha;
    bc.y = beta;
    bc.z = gamma;

    return bc;
}

Homogeneous4 Triangle::phong(Homogeneous4 lightPos, Homogeneous4 lightColour, Cartesian3 barycentric, bool inShadow) {
    if (inShadow) return Homogeneous4(0, 0, 0, 0);

    Cartesian3 normal = (barycentric.x * normals[0].Vector() + barycentric.y * normals[1].Vector() + barycentric.z * normals[2].Vector()).unit();
    Cartesian3 hitPos = barycentric.x * verts[0].Vector() + barycentric.y * verts[1].Vector() + barycentric.z * verts[2].Vector();
    Cartesian3 l = (lightPos.Point() - hitPos).unit();
    Cartesian3 e = Cartesian3(-hitPos.x, -hitPos.y, -hitPos.z).unit();

    // Diffuse
    float cosTheta = std::clamp(normal.dot(l), 0.0f, 1.0f);
    Cartesian3 diffuse = Cartesian3(
        shared_material->diffuse.x * lightColour.x,
        shared_material->diffuse.y * lightColour.y,
        shared_material->diffuse.z * lightColour.z) * cosTheta;
    //Cartesian3 ambient = Cartesian3(
    //    shared_material->ambient.x * lightColour.x,
    //    shared_material->ambient.y * lightColour.y,
    //    shared_material->ambient.z * lightColour.z);

    // Specular
    Cartesian3 B = (l + e).unit();
    float cosB = std::clamp(normal.dot(B), 0.0f, 1.0f);
    cosB = std::clamp(std::pow(cosB, shared_material->shininess), 0.0f, 1.0f);
    cosB = cosB * cosTheta * (shared_material->shininess + 2.0f) / (2.0f * 3.14159265359f);
    Cartesian3 specular = Cartesian3(
        shared_material->specular.x * lightColour.x,
        shared_material->specular.y * lightColour.y,
        shared_material->specular.z * lightColour.z) * cosB;

    //if (inShadow) return Homogeneous4(ambient);
    return Homogeneous4(diffuse + specular);
}