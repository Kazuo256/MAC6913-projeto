
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_PROJECT_METABALL_H
#define PBRT_PROJECT_METABALL_H

// project/metaball.h*
#include "project/implicitsurface.h"

class MetaballSurface : public ImplicitSurface {
  public:
    MetaballSurface(int nb, const Point *P, const float *R, const float *B);
    virtual ~MetaballSurface() {}
    bool Inside(const Point &p) const;
    float Distance(const Point &p) const;
    Normal Gradient(const Point &p) const;
  private:
    float Density(const Point &p) const;
    int nbumps;
    Point *points;
    float *radius;
    float *blobbiness;
};

MetaballSurface* CreateMetaballSurface(const ParamSet &params);

#endif
