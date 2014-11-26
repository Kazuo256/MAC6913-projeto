
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_PROJECT_IMPLICITSURFACE_H
#define PBRT_PROJECT_IMPLICITSURFACE_H

// project/marchingcubes.h*
#include "geometry.h"

class ImplicitSurface {
  public:
    ImplicitSurface() {}
    virtual ~ImplicitSurface() {}
    virtual bool Inside(const Point& p) const = 0;
};

#endif
