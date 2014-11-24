
//project/metaball.cpp
#include "stdafx.h"
#include "project/metaball.h"
#include "paramset.h"

Metaball::Metaball(int nb, const Point *P, const float *R, const float *B)
        : nbumps(nb) {
    points = new Point[nb];
    memcpy(points, P, nb);
    radius = new float[nb];
    memcpy(radius, R, nb);
    blobbiness = new float[nb];
    memcpy(blobbiness, B, nb);
}

bool Metaball::Inside(const Point &p) const {
    float d = 0;
    for (int i = 0; i < nbumps; ++i) {
        Vector v = points[i] - p;
        float r2 = v.x*v.x + v.y*v.y + v.z*v.z;
        float B = blobbiness[i];
        float R = radius[i];
        d += exp(B/(R*R)*r2 - B);
    }
    return d > 1;
}

Metaball* CreateMetaball(const ParamSet &params) {
    int nbumps, nradius, nblobs;
    const Point *P = params.FindPoint("P", &nbumps); 
    const float *R = params.FindFloat("R", &nradius);
    const float *B = params.FindFloat("B", &nblobs);
    if (nbumps != nradius || nbumps != nblobs) {
        Error("Number of points, radius and blobbiness must match");
        return NULL;
    }
    return new Metaball(nbumps, P, R, B);
}

