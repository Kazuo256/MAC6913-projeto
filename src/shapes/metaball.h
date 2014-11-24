#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_SHAPES_METABALL_H
#define PBRT_SHAPES_METABALL_H

// shapes/metaball.h*
#include "shape.h"

// Metaball Declarations
class Metaball : public Shape {
public:
    // Metaball Public Methods
    Metaball(const Transform *o2w, const Transform *w2o, bool ro, float rad,
           float zmin, float zmax, float phiMax);
    BBox ObjectBound() const;
    bool Intersect(const Ray &ray, float *tHit, float *rayEpsilon,
                   DifferentialGeometry *dg) const;
    bool IntersectP(const Ray &ray) const;
    float Area() const;
    Point Sample(float u1, float u2, Normal *ns) const;
    Point Sample(const Point &p, float u1, float u2, Normal *ns) const;
    float Pdf(const Point &p, const Vector &wi) const;
private:
    // Metaball Private Data
    float radius;
    float phiMax;
    float zmin, zmax;
    float thetaMin, thetaMax;
};


Metaball *CreateMetaballShape(const Transform *o2w, const Transform *w2o,
        bool reverseOrientation, const ParamSet &params);

#endif // PBRT_SHAPES_METABALL_H
