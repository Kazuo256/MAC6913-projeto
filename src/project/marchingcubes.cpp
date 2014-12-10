
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

#ifndef NDEBUG
# define log printf
#else
# define log(...)
#endif

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
    for (int di = 0; di < 2; ++di)
        for (int dj = 0; dj < 2; ++dj)
            for (int dk = 0; dk < 2; ++dk, ++t) {
                int check = static_cast<int>(inside[GetVertIndex(i+di, j+dj, k+dk)]);
                mask |= check << t;
    }
    return mask;
}

// VoxelCase definition
class VoxelCase {
  public:
    virtual ~VoxelCase() {}
    virtual void Generate(vector<int> &inds, const VoxelHelper &helper, int i, int j, int k) = 0;
};

#define MAKECASE_NOPARAMS(name) \
  class name : public VoxelCase { \
    public: \
      void Generate(vector<int> &inds, const VoxelHelper &helper, int i, int j, int k); \
  }; \
  void name::Generate(vector<int> &inds, const VoxelHelper &helper, int i, int j, int k)

#define MAKECASE_2PARAMS(name, type, param1, param2) \
  class name : public VoxelCase { \
    public: \
      name(type p1, type p2) : param1(p1), param2(p2) {} \
      void Generate(vector<int> &inds, const VoxelHelper &helper, int i, int j, int k); \
    private: \
      type param1, param2; \
  }; \
  void name::Generate(vector<int> &inds, const VoxelHelper &helper, int i, int j, int k)

#define MAKECASE_3PARAMS(name, type, param1, param2, param3) \
  class name : public VoxelCase { \
    public: \
      name(type p1, type p2, type p3) : param1(p1), param2(p2), param3(p3) {} \
      void Generate(vector<int> &inds, const VoxelHelper &helper, int i, int j, int k); \
    private: \
      type param1, param2, param3; \
  }; \
  void name::Generate(vector<int> &inds, const VoxelHelper &helper, int i, int j, int k)

MAKECASE_NOPARAMS(Case0) { /* Does nothing =D */ }

MAKECASE_3PARAMS(Case1, bool, di, dj, dk) {
    log("Case 1\n");
    inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
    inds.push_back(helper.GetXIndex(i + int(di), j, k + int(dk)));
}

MAKECASE_2PARAMS(Case2X, bool, di, dk) {
    log("Case 2X\n");
    inds.push_back(helper.GetYIndex(i, j, k + int(dk)));
    inds.push_back(helper.GetZIndex(i + int(di), j, k));
    inds.push_back(helper.GetZIndex(i + int(di), j + 1, k));
    inds.push_back(helper.GetZIndex(i + int(di), j + 1, k));
    inds.push_back(helper.GetYIndex(i, j + 1, k + int(dk)));
    inds.push_back(helper.GetYIndex(i, j, k + int(dk)));
}

MAKECASE_2PARAMS(Case2Y, bool, dj, dk) {
    log("Case 2Y\n");
    inds.push_back(helper.GetXIndex(i, j, k + int(dk)));
    inds.push_back(helper.GetZIndex(i, j + int(dj), k));
    inds.push_back(helper.GetZIndex(i + 1, j + int(dj), k));
    inds.push_back(helper.GetZIndex(i + 1, j + int(dj), k));
    inds.push_back(helper.GetXIndex(i + 1, j, k + int(dk)));
    inds.push_back(helper.GetXIndex(i, j, k + int(dk)));
}

MAKECASE_2PARAMS(Case2Z, bool, di, dj) {
    log("Case 2Z\n");
    inds.push_back(helper.GetXIndex(i + int(di), j, k));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + 1));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + 1));
    inds.push_back(helper.GetXIndex(i + int(di), j, k + 1));
    inds.push_back(helper.GetXIndex(i + int(di), j, k));
}

class Case3 : public VoxelCase {
  public:
    Case3(int t, bool i, bool j, bool k) : trans(t), di(i), dj(j), dk(k) {}
    void Generate(vector<int> &inds, const VoxelHelper &helper, int i, int j, int k) {
        log("Case 3\n");
        Case1(di, dj, dk).Generate(inds, helper, i, j, k);
        bool b[3] = { !di, !dj, !dk };
        b[trans] = !b[trans];
        Case1(b[0], b[1], b[2]).Generate(inds, helper, i, j, k);
    }
  private:
    int trans;
    bool di, dj, dk;
};

MAKECASE_3PARAMS(Case4, bool, di, dj, dk) {
    log("Case 4\n");
    Case1(di, dj, dk).Generate(inds, helper, i, j, k);
    Case1(!di, !dj, !dk).Generate(inds, helper, i, j, k);
}

MAKECASE_3PARAMS(Case5X, bool, di, dj, dk) {
    log("Case 5X\n");
    // Middle triangle
    inds.push_back(helper.GetXIndex(i + int(di), j, k + int(dk)));
    inds.push_back(helper.GetXIndex(i + int(di), j, k + int(!dk)));
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(dk)));
    // Ramp
    inds.push_back(helper.GetXIndex(i + int(di), j, k + int(!dk)));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(!dk)));
    inds.push_back(helper.GetZIndex(i + int(!di), j + int(dj), k));
    inds.push_back(helper.GetZIndex(i + int(!di), j + int(dj), k));
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(dk)));
    inds.push_back(helper.GetXIndex(i + int(di), j, k + int(!dk)));
}

MAKECASE_3PARAMS(Case5Y, bool, di, dj, dk) {
    log("Case 5Y\n");
    // Middle triangle
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(dk)));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(!dk)));
    // Ramp
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(dk)));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
    inds.push_back(helper.GetXIndex(i + int(di), j, k + int(!dk)));
    inds.push_back(helper.GetXIndex(i + int(di), j, k + int(!dk)));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(!dk)));
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(dk)));
}

MAKECASE_3PARAMS(Case5Z, bool, di, dj, dk) {
    log("Case 5Z\n");
    // Middle triangle
    inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
    inds.push_back(helper.GetZIndex(i + int(!di), j + int(dj), k));
    // Ramp
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(dk)));
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(dk)));
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(dk)));
    inds.push_back(helper.GetZIndex(i + int(!di), j + int(dj), k));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
}

MAKECASE_NOPARAMS(Case8X) {
    log("Case 8X\n");
    inds.push_back(helper.GetXIndex(i, j, k));
    inds.push_back(helper.GetXIndex(i, j, k + 1));
    inds.push_back(helper.GetXIndex(i + 1, j, k + 1));
    inds.push_back(helper.GetXIndex(i + 1, j, k + 1));
    inds.push_back(helper.GetXIndex(i + 1, j, k));
    inds.push_back(helper.GetXIndex(i, j, k));
}

MAKECASE_NOPARAMS(Case8Y) {
    log("Case 8Y\n");
    inds.push_back(helper.GetYIndex(i, j, k));
    inds.push_back(helper.GetYIndex(i, j, k + 1));
    inds.push_back(helper.GetYIndex(i, j + 1, k + 1));
    inds.push_back(helper.GetYIndex(i, j + 1, k + 1));
    inds.push_back(helper.GetYIndex(i, j + 1, k));
    inds.push_back(helper.GetYIndex(i, j, k));
}

MAKECASE_NOPARAMS(Case8Z) {
    log("Case 8Z\n");
    inds.push_back(helper.GetZIndex(i, j, k));
    inds.push_back(helper.GetZIndex(i, j + 1, k));
    inds.push_back(helper.GetZIndex(i + 1, j + 1, k));
    inds.push_back(helper.GetZIndex(i + 1, j + 1, k));
    inds.push_back(helper.GetZIndex(i + 1, j, k));
    inds.push_back(helper.GetZIndex(i, j, k));
}

class Cases {
  public:
    ~Cases() {
        for (int i = 0; i < 256; ++i)
            if (cases[i])
                delete cases[i];
        cases.clear();
    }
    void Init() {
        if (!cases.empty()) return;
        cases.resize(256, NULL);
        // Cases definition
        cases[0]    = NULL;
        cases[1]    = new Case1(false, false, false);
        cases[2]    = new Case1(false, false, true);
        cases[3]    = new Case2Z(false, false);           // 1+2
        cases[4]    = new Case1(false, true, false);
        cases[5]    = new Case2X(false, false);           // 1+4
        cases[6]    = new Case3(0, false, false, true);   // 2+4
        cases[7]    = new Case5Y(false, false, false);    // 2+1+4
        cases[8]    = new Case1(false, true, true);
        cases[9]    = new Case3(0, false, false, false);  // 1+8
        cases[10]   = new Case2X(false, true);            // 2+8
        cases[11]   = new Case5Y(false, false, true);     // 8+2+1
        cases[12]   = new Case2Z(false, true);            // 4+8
        cases[13]   = new Case5Y(false, true, false);     // 1+4+8
        cases[14]   = new Case5Y(false, true, true);      // 4+8+2
        cases[15]   = new Case8Y;                         // 1+2+4+8
        cases[16]   = new Case1(true, false, false);
        cases[17]   = new Case2Y(false, false);           // 1+16
        cases[18]   = new Case3(1, false, false, true);   // 2+16
        cases[19]   = new Case5X(false, false, false);    // 2+1+16
        cases[20]   = new Case3(2, false, true, false);   // 4+16
        cases[21]   = new Case5Z(false, false, false);    // 16+1+4
        cases[24]   = new Case4(false, true, true);       // 8+16
        cases[32]   = new Case1(true, false, true);
        cases[33]   = new Case3(1, false, false, false);  // 1+32
        cases[34]   = new Case2Y(false, true);            // 2+32
        cases[35]   = new Case5X(false, false, true);     // 32+2+1
        cases[36]   = new Case4(false, true, false);      // 4+32
        cases[40]   = new Case3(2, false, true, true);    // 8+32
        cases[42]   = new Case5Z(false, false, true);     // 32+2+8
        cases[48]   = new Case2Z(true, false);            // 16+32
        cases[49]   = new Case5X(true, false, false);     // 1+16+32
        cases[50]   = new Case5X(true, false, true);      // 16+32+2
        cases[51]   = new Case8X;                         // 1+2+16+32
        cases[64]   = new Case1(true, true, false);
        cases[65]   = new Case3(2, false, false, false);  // 1+64
        cases[66]   = new Case4(false, false, true);      // 2+64
        cases[68]   = new Case2Y(true, false);            // 4+64
        cases[69]   = new Case5Z(false, true, false);     // 1+4+64
        cases[72]   = new Case3(1, false, true, true);    // 8+64
        cases[76]   = new Case5X(false, true, false);     // 8+4+64
        cases[80]   = new Case2X(true, false);            // 16+64
        cases[81]   = new Case5Z(true, false, false);     // 64+16+1
        cases[84]   = new Case5Z(true, true, false);      // 4+64+16
        cases[85]   = new Case8Z;                         // 1+4+16+64
        cases[96]   = new Case3(0, true, false, true);    // 32+64
        cases[112]  = new Case5Y(true, false, false);     // 32+16+64
        cases[128]  = new Case1(true, true, true);
        cases[129]  = new Case4(false, false, false);     // 1+128
        cases[130]  = new Case3(2, false, false, true);   // 2+128
        cases[132]  = new Case3(1, false, true, false);   // 4+128
        cases[136]  = new Case2Y(true, true);             // 8+128
        cases[138]  = new Case5Z(false, true, true);      // 2+8+128
        cases[140]  = new Case5X(false, true, true);      // 128+8+4
        cases[144]  = new Case3(0, true, false, false);   // 16+128
        cases[160]  = new Case2X(true, true);             // 32+128
        cases[162]  = new Case5Z(true, false, true);      // 128+32+2
        cases[168]  = new Case5Z(true, true, true);       // 8+128+32
        cases[176]  = new Case5Y(true, false, true);      // 128+32+16
        cases[192]  = new Case2Z(true, true);             // 64+128
        cases[196]  = new Case5X(true, true, false);      // 4+64+128
        cases[200]  = new Case5X(true, true, true);       // 64+128+8
        cases[208]  = new Case5Y(true, true, false);      // 16+64+128
        cases[224]  = new Case5Y(true, true, true);       // 64+128+32
        cases[255]  = NULL;
    }
    VoxelCase *operator[](int mask) const {
        return cases[mask];
    }
  private:
    vector<VoxelCase*> cases;
};

Cases cases;

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
                inside[ind] = surface->Inside(o + step*Vector(j, i, k));
    }
    cases.Init();
    vector<int> inds;
    for (int k = 0; k < depth-1; ++k)
        for (int i = 0; i < height-1; ++i)
            for (int j = 0; j < width-1; ++j) {
                int c = helper.CheckCase(inside, i, j, k);
                VoxelCase *which = cases[c] ? cases[c] : cases[255-c];
                if (which) {
                    log("Check %d (%d, %d, %d)/", c, i, j, k);
                    Point p = o + step*Vector(j, i, k);
                    log("(%.2f, %.2f, %.2f): ", p.x, p.y, p.z);
                    which->Generate(inds, helper, i, j, k);
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

