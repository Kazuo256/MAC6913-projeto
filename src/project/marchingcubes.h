
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_PROJECT_MARCHINGCUBES_H
#define PBRT_PROJECT_MARCHINGCUBES_H

// project/marchingcubes.h*

class TriangleMesh;
class ImplicitSurface;
class Transform;
class Vector;

TriangleMesh *ImplicitSurfaceToMesh(const Transform *o2w, const Transform *w2o,
    bool reverseOrientation, const ImplicitSurface *surface,
    const Vector &space, float step, bool smooth);

#endif

