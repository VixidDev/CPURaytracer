#ifndef RAY_H
#define RAY_H

#include "Cartesian3.h"

class Ray
{


public:
    enum Type{primary,secondary, shadow};
    Ray(Cartesian3 og,Cartesian3 dir,Type rayType);
    Cartesian3 origin;
    Cartesian3 direction;
    Type ray_type;

};

#endif // RAY_H
