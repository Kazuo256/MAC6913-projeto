
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_PROJECT_MARCHINGCUBES_H
#define PBRT_PROJECT_MARCHINGCUBES_H

// project/marchingcubes.h*

class TriangleMesh;
class ImplicitSurface;

TriangleMesh* ImplicitSurfaceToMesh(const ImplicitSurface *surface);

#endif

