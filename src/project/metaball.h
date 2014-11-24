
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_PROJECT_METABALL_H
#define PBRT_PROJECT_METABALL_H

// project/metaball.h*
#include "project/implicitsurface.h"

class Metaball : public ImplicitSurface {
  public:
    Metaball() {}
    virtual ~Metaball() {}
    bool Inside(const Point& p) const;
};

Metaball* CreateMetaball(const ParamSet &params);

#endif
