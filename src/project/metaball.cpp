
//project/metaball.cpp
#include "stdafx.h"
#include "project/metaball.h"
#include "paramset.h"

#include <algorithm>
#include <iostream>

Metaball::Metaball(int nb, const Point *P, const float *R, const float *B)
        : nbumps(nb) {
    points = new Point[nb];
    std::copy(P, P+nb, points);
    radius = new float[nb];
    std::copy(R, R+nb, radius);
    blobbiness = new float[nb];
    std::copy(B, B+nb, blobbiness);
}

bool Metaball::Inside(const Point &p) const {
    float d = 0;
    for (int i = 0; i < nbumps; ++i) {
        Vector v = points[i] - p;
        float r2 = v.x*v.x + v.y*v.y + v.z*v.z;
        float B = -blobbiness[i];
        float R = radius[i];
        float x = B/(R*R)*r2 - B;
        d += exp(x);
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

