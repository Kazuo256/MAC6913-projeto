
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

// VoxelHelper definition
class VoxelHelper {
  public:
    VoxelHelper(int w, int h, int d)
        : width(w), height(h), depth(d) {}
    int GetNVerts() const;
    int GetNEdges() const;
    int GetVertIndex(int i, int j, int k) const;
    int GetZIndex(int i, int j, int k) const;
    int GetYIndex(int i, int j, int k) const;
    int GetXIndex(int i, int j, int k) const;
    int CheckCase(const vector<bool> &inside, int i, int j, int k) const;
  private:
    int width, height, depth;
};

int VoxelHelper::GetNVerts() const {
    return width*height*depth;
}

int VoxelHelper::GetNEdges() const {
    return width*height*(depth-1) + width*(height-1)*depth + (width-1)*height*depth;
}

int VoxelHelper::GetVertIndex(int i, int j, int k) const {
    return k*width*height + i*width + j;
}

int VoxelHelper::GetZIndex(int i, int j, int k) const {
    return k*width*height + i*width + j;
}

int VoxelHelper::GetYIndex(int i, int j, int k) const {
    return GetZIndex(height-1, width-1, depth-2) + i*width*depth + k*width + j;
}

int VoxelHelper::GetXIndex(int i, int j, int k) const {
    return GetYIndex(height-2, width-1, depth-1) + j*height*depth + i*depth + k;
}

int VoxelHelper::CheckCase(const vector<bool> &inside, int i, int j, int k) const {
    int mask = 0;
    int t = 0;
    for (int dk = 0; dk < 2; ++dk)
        for (int di = 0; di < 2; ++di)
            for (int dj = 0; dj < 2; ++dj, ++t) {
                int check = static_cast<int>(inside[GetVertIndex(i+di, j+dj, k+dk)]);
                mask |= check << t;
    }
    return mask;
}

// VoxelCase definition
class VoxelCase {
  public:
    virtual ~VoxelCase() {}
    virtual void operator()(const VoxelHelper &helper, int i, int j, int k) = 0;
};

} // unnamed namespace

TriangleMesh *ImplicitSurfaceToMesh(const Transform *o2w, const Transform *w2o,
        bool reverseOrientation, const ImplicitSurface *surface,
        const Vector &space, float step) {
    int width = space.x/step;
    int height = space.y/step;
    int depth = space.z/step;
    VoxelHelper helper(width, height, depth);
    Point *P = new Point[helper.GetNEdges()];
    vector<bool> inside(helper.GetNVerts(), false);
    Point o = Point(0,0,0) - space/2;
    for (int k = 0; k < depth; ++k)
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j) {
                if (k+1 < depth) {
                    int ind = helper.GetZIndex(i, j, k);
                    P[ind] = o + step*(Vector(j, i, k) + Vector(j, i, k+1))/2.f;
                }
                if (i+1 < height) {
                    int ind = helper.GetYIndex(i, j, k);
                    P[ind] = o + step*(Vector(j, i, k) + Vector(j, i+1, k))/2.f;
                }
                if (j+1 < width) {
                    int ind = helper.GetXIndex(i, j, k);
                    P[ind] = o + step*(Vector(j, i, k) + Vector(j+1, i, k))/2.f;
                }
                int ind = helper.GetVertIndex(i, j, k);
                inside[ind] = surface->Inside(P[ind]);
    }
    vector<int> inds;
    for (int k = 0; k < depth-1; ++k)
        for (int i = 0; i < height-1; ++i)
            for (int j = 0; j < width-1; ++j) {
                int c = helper.CheckCase(inside, i, j, k);
                if (c > 0 && c < 8) {
                    // top
                    inds.push_back(helper.GetXIndex(i, j, k));
                    inds.push_back(helper.GetZIndex(i, j+1, k));
                    inds.push_back(helper.GetXIndex(i, j, k+1));
                    inds.push_back(helper.GetXIndex(i, j, k+1));
                    inds.push_back(helper.GetZIndex(i, j, k));
                    inds.push_back(helper.GetXIndex(i, j, k));
                }
            }
    std::cout << "TRIANGLES: " << inds.size()/3 << std::endl;
    int *vi = new int[inds.size()];
    std::copy(inds.begin(), inds.end(), vi);
    ParamSet params;
    params.AddPoint("P", P, helper.GetNEdges());
    params.AddInt("indices", vi, inds.size());
    TriangleMesh *mesh = CreateTriangleMeshShape(o2w, w2o, reverseOrientation,
                                                 params, NULL);
    delete P;
    delete vi;
    return mesh;
}

