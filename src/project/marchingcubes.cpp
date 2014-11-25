
// project/marchingcubes.cpp
#include "stdafx.h"
#include "geometry.h"
#include "paramset.h"
#include "project/marchingcubes.h"
#include "project/implicitsurface.h"
#include "shapes/trianglemesh.h"

#include <algorithm>
#include <iostream>

namespace {

int getIndex(int i, int j, int k, int width, int height) {
    return k*width*height + i*width + j;
}

int CountInside(const vector<bool> &inside, int i, int j, int k, int width, int height) {
  int c = 0;
  c += inside[getIndex(i, j, k, width, height)];
  c += inside[getIndex(i+1, j, k, width, height)];
  c += inside[getIndex(i, j+1, k, width, height)];
  c += inside[getIndex(i+1, j+1, k, width, height)];
  c += inside[getIndex(i, j, k+1, width, height)];
  c += inside[getIndex(i+1, j, k+1, width, height)];
  c += inside[getIndex(i, j+1, k+1, width, height)];
  c += inside[getIndex(i+1, j+1, k+1, width, height)];
  return c;
}

}

TriangleMesh *ImplicitSurfaceToMesh(const Transform *o2w, const Transform *w2o,
        bool reverseOrientation, const ImplicitSurface *surface,
        const Vector &space, float step) {
    int width = space.x/step;
    int height = space.y/step;
    int depth = space.z/step;
    int nverts = width*height*depth;
    Point *P = new Point[nverts];
    vector<bool> inside(nverts, false);
    Point o = Point(0,0,0) - space/2;
    for (int k = 0; k < depth; ++k)
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j) {
                int ind = getIndex(i, j, k, width, height);
                P[ind] = o + Vector(j*step, i*step, k*step);
                inside[ind] = surface->Inside(P[ind]);
    }
    vector<int> inds;
    for (int k = 0; k < depth-1; ++k)
        for (int i = 0; i < height-1; ++i)
            for (int j = 0; j < width-1; ++j)
                if (CountInside(inside, i, j, k, width, height) > 0) {
                    // top
                    inds.push_back(getIndex(i, j, k+1, width, height));
                    inds.push_back(getIndex(i, j+1, k+1, width, height));
                    inds.push_back(getIndex(i+1, j+1, k+1, width, height));
                    inds.push_back(getIndex(i+1, j+1, k+1, width, height));
                    inds.push_back(getIndex(i+1, j, k+1, width, height));
                    inds.push_back(getIndex(i, j, k+1, width, height));
                    // front
                    inds.push_back(getIndex(i, j+1, k+1, width, height));
                    inds.push_back(getIndex(i+1, j+1, k+1, width, height));
                    inds.push_back(getIndex(i+1, j+1, k, width, height));
                    inds.push_back(getIndex(i+1, j+1, k, width, height));
                    inds.push_back(getIndex(i, j+1, k, width, height));
                    inds.push_back(getIndex(i, j+1, k+1, width, height));
                    // left
                    inds.push_back(getIndex(i, j, k+1, width, height));
                    inds.push_back(getIndex(i, j+1, k+1, width, height));
                    inds.push_back(getIndex(i, j+1, k, width, height));
                    inds.push_back(getIndex(i, j+1, k, width, height));
                    inds.push_back(getIndex(i, j, k, width, height));
                    inds.push_back(getIndex(i, j, k+1, width, height));
                    // right
                    inds.push_back(getIndex(i+1, j+1, k+1, width, height));
                    inds.push_back(getIndex(i+1, j, k+1, width, height));
                    inds.push_back(getIndex(i+1, j, k, width, height));
                    inds.push_back(getIndex(i+1, j, k, width, height));
                    inds.push_back(getIndex(i+1, j+1, k, width, height));
                    inds.push_back(getIndex(i+1, j+1, k+1, width, height));
                    // back
                    inds.push_back(getIndex(i+1, j, k+1, width, height));
                    inds.push_back(getIndex(i, j, k+1, width, height));
                    inds.push_back(getIndex(i, j, k, width, height));
                    inds.push_back(getIndex(i, j, k, width, height));
                    inds.push_back(getIndex(i+1, j, k, width, height));
                    inds.push_back(getIndex(i+1, j, k+1, width, height));
                    // bottom
                    inds.push_back(getIndex(i+1, j, k, width, height));
                    inds.push_back(getIndex(i, j, k, width, height));
                    inds.push_back(getIndex(i, j+1, k, width, height));
                    inds.push_back(getIndex(i, j+1, k, width, height));
                    inds.push_back(getIndex(i+1, j+1, k, width, height));
                    inds.push_back(getIndex(i+1, j, k, width, height));
    }
    std::cout << "TRIANGLES: " << inds.size()/3 << std::endl;
    int *vi = new int[inds.size()];
    std::copy(inds.begin(), inds.end(), vi);
    ParamSet params;
    params.AddPoint("P", P, nverts);
    params.AddInt("indices", vi, inds.size());
    TriangleMesh *mesh = CreateTriangleMeshShape(o2w, w2o, reverseOrientation,
                                                 params, NULL);
    delete P;
    delete vi;
    return mesh;
}

