#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_SHAPES_METABALLINSTANCE_H
#define PBRT_SHAPES_METABALLINSTANCE_H

// shapes/metaballInstance.h*
#include "shape.h"

// Metaball Declarations
class MetaballInstance : public Shape {
public:
    // MetaballInstance Public Methods
    MetaballInstance(const Transform *o2w, const Transform *w2o, bool ro, Point cent,
                     float rad, float blob);
    BBox ObjectBound() const;
    bool Intersect(const Ray &ray, float *tHit, float *rayEpsilon,
                   DifferentialGeometry *dg) const;
    bool IntersectP(const Ray &ray) const;
    float Area() const;
    Point Sample(float u1, float u2, Normal *ns) const;
    Point Sample(const Point &p, float u1, float u2, Normal *ns) const;
    float Pdf(const Point &p, const Vector &wi) const;

    Point GetCenter() const { return center; }
    float GetRadius() const { return radius; }
    float GetBlobbiness() const { return blobbiness; }
    float ValueAt(const Point &p) const;
    bool IntersectAll(const Ray &ray, float *t0, float *t1) const;
    bool IntersectFirst(const Ray &ray, float *t) const;
private:
    // MetaballInstance Private Data
    friend class Metaball;
    Point center;
    float radius;
    float blobbiness;
};


MetaballInstance *CreateMetaballInstanceShape(const Transform *o2w, const Transform *w2o,
                              bool reverseOrientation, const ParamSet &params);

#endif // PBRT_SHAPES_METABALLINSTANCE_H
