
// project/marchingcubes.cpp
#include "stdafx.h"
#include "project/marchingcubes.h"
#include "project/implicitsurface.h"
#include "shapes/trianglemesh.h"
#include "geometry.h"



TriangleMesh* ImplicitSurfaceToMesh(const Transform *o2w, const Transform *w2o,
        bool reverseOrientation,const ImplicitSurface *surface,
        const Vector &space, float step) {
    int width = space.x/step;
    int height = space.y/step;
    int depth = space.z/step;
    int nverts = width*height*depth - width*height - height*depth - width*depth;
    Point *V = new Point[nverts];
    Point o = Point(0,0,0) - space/2;
    for (int k = 1; k < depth; ++k)
        for (int i = 1; i < height; ++i)
            for (int j = 1; j < width-1; ++j) {
    }

    delete V;
    return NULL;
}

