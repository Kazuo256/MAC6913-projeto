
// project/marchingcubes.cpp
#include "stdafx.h"
#include "geometry.h"
#include "paramset.h"
#include "project/marchingcubes.h"
#include "project/implicitsurface.h"
#include "shapes/trianglemesh.h"
#include "progressreporter.h"

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
    int GetIndex(int t, int a, int b, int c) const;
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

int VoxelHelper::GetIndex(int t, int a, int b, int c) const {
    switch(t) {
      case 0:
        return GetYIndex(a, b, c);
      case 1:
        return GetXIndex(b, a, c);
      case 2:
        return GetXIndex(b, c, a);
      default:
        Error("Bad index!\n");
        return -1;
    }
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

#define MAKECASE_1PARAM(name, type, param) \
  class name : public VoxelCase { \
    public: \
      name(type p) : param(p) {} \
      void Generate(vector<int> &inds, const VoxelHelper &helper, int i, int j, int k); \
    private: \
      type param; \
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

class Case3e : public VoxelCase {
  public:
    Case3e(int t, bool i, bool j, bool k) : trans(t), di(i), dj(j), dk(k) {}
    void Generate(vector<int> &inds, const VoxelHelper &helper, int i, int j, int k) {
        log("Case 3e\n");
        bool b[3] = { !di, !dj, !dk };
        b[trans] = !b[trans];
        int t = trans;
        // There must be a better way to do this
        if (t == 0) {
            // 2 -> 0
            inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
            inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
            inds.push_back(helper.GetYIndex(i, j + int(b[1]), k + int(b[2])));
            // 0 -> 1 -> 2
            inds.push_back(helper.GetYIndex(i, j + int(b[1]), k + int(b[2])));
            inds.push_back(helper.GetXIndex(i + int(b[0]), j, k + int(b[2])));
            inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
            // other side
            // 1 -> 0
            inds.push_back(helper.GetXIndex(i + int(di), j, k + int(dk)));
            inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
            inds.push_back(helper.GetYIndex(i, j + int(b[1]), k + int(b[2])));
            // 0 -> 2 -> 1
            inds.push_back(helper.GetYIndex(i, j + int(b[1]), k + int(b[2])));
            inds.push_back(helper.GetZIndex(i + int(b[0]), j + int(b[1]), k));
            inds.push_back(helper.GetXIndex(i + int(di), j, k + int(dk)));
        } else if (t == 1) {
            // 0 -> 1
            inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
            inds.push_back(helper.GetXIndex(i + int(di), j, k + int(dk)));
            inds.push_back(helper.GetXIndex(i + int(b[0]), j, k + int(b[2])));
            // 1 -> 2 -> 0
            inds.push_back(helper.GetXIndex(i + int(b[0]), j, k + int(b[2])));
            inds.push_back(helper.GetZIndex(i + int(b[0]), j + int(b[1]), k));
            inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
            // other side
            // 2 -> 1
            inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
            inds.push_back(helper.GetXIndex(i + int(di), j, k + int(dk)));
            inds.push_back(helper.GetXIndex(i + int(b[0]), j, k + int(b[2])));
            // 1 -> 0 -> 2
            inds.push_back(helper.GetXIndex(i + int(b[0]), j, k + int(b[2])));
            inds.push_back(helper.GetXIndex(i + int(b[0]), j, k + int(b[2])));
            inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
        } else if (t == 2) {
            // 0 -> 2
            inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
            inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
            inds.push_back(helper.GetZIndex(i + int(b[0]), j + int(b[1]), k));
            // 2 -> 1 -> 0
            inds.push_back(helper.GetZIndex(i + int(b[0]), j + int(b[1]), k));
            inds.push_back(helper.GetXIndex(i + int(b[0]), j, k + int(b[2])));
            inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
            // other side
            // 1 -> 2
            inds.push_back(helper.GetXIndex(i + int(di), j, k + int(dk)));
            inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
            inds.push_back(helper.GetZIndex(i + int(b[0]), j + int(b[1]), k));
            // 2 -> 0 -> 1
            inds.push_back(helper.GetZIndex(i + int(b[0]), j + int(b[1]), k));
            inds.push_back(helper.GetYIndex(i, j + int(b[1]), k + int(b[2])));
            inds.push_back(helper.GetXIndex(i + int(di), j, k + int(dk)));
        } else Warning("Bad case 3e\n");
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
    
MAKECASE_3PARAMS(Case6X, bool, di, dj, dk) {
    Case1(di, dj, dk).Generate(inds, helper, i, j, k);
    Case2X(!di, !dk).Generate(inds, helper, i, j, k);
}

MAKECASE_3PARAMS(Case6Y, bool, di, dj, dk) {
    Case1(di, dj, dk).Generate(inds, helper, i, j, k);
    Case2Y(!dj, !dk).Generate(inds, helper, i, j, k);
}

MAKECASE_3PARAMS(Case6Z, bool, di, dj, dk) {
    Case1(di, dj, dk).Generate(inds, helper, i, j, k);
    Case2Z(!di, !dj).Generate(inds, helper, i, j, k);
}
    
MAKECASE_3PARAMS(Case7, bool, di, dj, dk) {
    log("Case 7\n");
    Case1(!di,  dj,  dk).Generate(inds, helper, i, j, k);
    Case1( di, !dj,  dk).Generate(inds, helper, i, j, k);
    Case1( di,  dj, !dk).Generate(inds, helper, i, j, k);
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
    
MAKECASE_2PARAMS(Case9, bool, di, dj) {
    log("Case 9\n");
    // Upper part
    inds.push_back(helper.GetXIndex(i + int(di), j, k + 1));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + 1));
    inds.push_back(helper.GetZIndex(i + int(!di), j + int(dj), k));
    inds.push_back(helper.GetZIndex(i + int(!di), j + int(dj), k));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
    inds.push_back(helper.GetXIndex(i + int(di), j, k + 1));
    // Lower part
    inds.push_back(helper.GetXIndex(i + int(!di), j, k));
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
    inds.push_back(helper.GetZIndex(i + int(!di), j + int(dj), k));
    inds.push_back(helper.GetXIndex(i + int(!di), j, k));
}
    
MAKECASE_1PARAM(Case10X, bool, di) {
    log("Case 10X\n");
    Case2X( di, false).Generate(inds, helper, i, j, k);
    Case2X(!di, true ).Generate(inds, helper, i, j, k);
}

MAKECASE_1PARAM(Case10Y, bool, dj) {
    log("Case 10Y\n");
    Case2Y( dj, false).Generate(inds, helper, i, j, k);
    Case2Y(!dj, true ).Generate(inds, helper, i, j, k);
}
    
MAKECASE_1PARAM(Case10Z, bool, di) {
    log("Case 10Z\n");
    Case2Z( di, false).Generate(inds, helper, i, j, k);
    Case2Z(!di, true ).Generate(inds, helper, i, j, k);
}
    
MAKECASE_3PARAMS(Case11XZ, bool, di, dj, dk) {
    log("Case 11XZ\n");
    // Central
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(!dk)));
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(dk)));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
    //
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(!dk)));
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(dk)));
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(dk)));
    //
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(dk)));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
    //
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(!dk)));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(!dk)));
}
    
MAKECASE_3PARAMS(Case11XY, bool, di, dj, dk) {
    log("Case 11XY\n");
    // Central
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(!dk)));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
    //
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(!dk)));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
    inds.push_back(helper.GetXIndex(i + int(di), j, k + int(!dk)));
    //
    inds.push_back(helper.GetZIndex(i + int(di), j + int(!dj), k));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(dk)));
    //
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(!dk)));
    inds.push_back(helper.GetYIndex(i, j + int(dj), k + int(dk)));
    inds.push_back(helper.GetZIndex(i + int(!di), j + int(dj), k));
}
    
MAKECASE_3PARAMS(Case11YZ, bool, di, dj, dk) {
    log("Case 11YZ\n");
    // Central
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(!dk)));
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(dk)));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
    //
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(!dk)));
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(dk)));
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(dk)));
    //
    inds.push_back(helper.GetXIndex(i + int(!di), j, k + int(dk)));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
    inds.push_back(helper.GetZIndex(i + int(!di), j + int(dj), k));
    //
    inds.push_back(helper.GetYIndex(i, j + int(!dj), k + int(!dk)));
    inds.push_back(helper.GetZIndex(i + int(di), j + int(dj), k));
    inds.push_back(helper.GetXIndex(i + int(di), j, k + int(!dk)));
}
    
class Cases {
  public:
    ~Cases() {
        for (size_t i = 0; i < cases.size(); ++i)
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
        cases[22]   = new Case7(false, false, false);     // 2+4+16
        cases[23]   = new Case9(false, false);            // 1+2+4+16
        cases[24]   = new Case4(false, true, true);       // 8+16
        cases[27]   = new Case11XY(false, false, true);   // 8+2+1+16
        cases[29]   = new Case11YZ(false, false, false);  // 16+1+4+8
        cases[32]   = new Case1(true, false, true);
        cases[33]   = new Case3(1, false, false, false);  // 1+32
        cases[34]   = new Case2Y(false, true);            // 2+32
        cases[35]   = new Case5X(false, false, true);     // 32+2+1
        cases[36]   = new Case4(false, true, false);      // 4+32
        cases[39]   = new Case11XY(false, false, false);  // 4+1+2+32
        cases[40]   = new Case3(2, false, true, true);    // 8+32
        cases[41]   = new Case7(false, false, true);      // 1+8+32
        cases[42]   = new Case5Z(false, false, true);     // 32+2+8
        cases[46]   = new Case11YZ(false, false, true);   // 32+2+4+8
        cases[48]   = new Case2Z(true, false);            // 16+32
        cases[49]   = new Case5X(true, false, false);     // 1+16+32
        cases[50]   = new Case5X(true, false, true);      // 16+32+2
        cases[51]   = new Case8X;                         // 1+2+16+32
        cases[53]   = new Case11XZ(false, false, false);  // 4+1+16+32
        cases[58]   = new Case11XZ(false, false, true);   // 8+2+16+32
        cases[60]   = new Case10Z(false);                 // 4+8+16+32
        cases[64]   = new Case1(true, true, false);
        cases[65]   = new Case3(2, false, false, false);  // 1+64
        cases[66]   = new Case4(false, false, true);      // 2+64
        cases[68]   = new Case2Y(true, false);            // 4+64
        cases[69]   = new Case5Z(false, true, false);     // 1+4+64
        cases[71]   = new Case11YZ(false, true, false);   // 64+4+1+2
        cases[72]   = new Case3(1, false, true, true);    // 8+64
        cases[73]   = new Case7(true, false, false);      // 1+8+64
        cases[76]   = new Case5X(false, true, false);     // 8+4+64
        cases[77]   = new Case9(false, true);             // 4+1+8+64
        cases[78]   = new Case11XY(false, true, true);    // 2+8+4+64
        cases[80]   = new Case2X(true, false);            // 16+64
        cases[81]   = new Case5Z(true, false, false);     // 64+16+1
        cases[83]   = new Case11XZ(true, false, false);   // 64+16+1+2
        cases[84]   = new Case5Z(true, true, false);      // 4+64+16
        cases[85]   = new Case8Z;                         // 1+4+16+64
        cases[90]   = new Case10X(false);                 // 2+8+16+64
        cases[92]   = new Case11XZ(true, true, false);    // 16+64+4+8
        cases[96]   = new Case3(0, true, false, true);    // 32+64
        cases[97]   = new Case7(false, true, false);      // 1+32+64
        cases[102]  = new Case10Y(false);                 // 2+4+32+64
        cases[104]  = new Case7(true, true, true);        // 8+32+64
        cases[112]  = new Case5Y(true, false, false);     // 32+16+64
        cases[113]  = new Case9(true, false);             // 16+1+32+64
        cases[114]  = new Case11XY(true, false, false);   // 64+16+2+32
        cases[116]  = new Case11YZ(true, true, false);    // 4+64+16+32
        cases[128]  = new Case1(true, true, true);
        cases[129]  = new Case4(false, false, false);     // 1+128
        cases[130]  = new Case3(2, false, false, true);   // 2+128
        cases[132]  = new Case3(1, false, true, false);   // 4+128
        cases[134]  = new Case7(true, false, true);       // 2+4+128
        cases[136]  = new Case2Y(true, true);             // 8+128
        cases[138]  = new Case5Z(false, true, true);      // 2+8+128
        cases[139]  = new Case11YZ(false, true, true);    // 128+8+1+2
        cases[140]  = new Case5X(false, true, true);      // 128+8+4
        cases[141]  = new Case11XY(false, true, false);   //1+4+8+128
        cases[144]  = new Case3(0, true, false, false);   // 16+128
        cases[146]  = new Case7(false, true, true);       // 2+16+128
        cases[148]  = new Case7(true, true, false);       // 4+16+128
        cases[153]  = new Case10Y(true);                  // 1+16+8+128
        cases[160]  = new Case2X(true, true);             // 32+128
        cases[162]  = new Case5Z(true, false, true);      // 128+32+2
        cases[163]  = new Case11XZ(true, false, true);    // 128+32+1+2
        cases[165]  = new Case10X(true);                  // 1+4+32+128
        cases[168]  = new Case5Z(true, true, true);       // 8+128+32
        cases[172]  = new Case11XZ(true, true, true);     // 32+128+4+8
        cases[176]  = new Case5Y(true, false, true);      // 128+32+16
        cases[177]  = new Case11XY(true, false, true);    // 128+32+1+16
        cases[184]  = new Case11YZ(true, true, true);     // 8+128+16+32
        cases[192]  = new Case2Z(true, true);             // 64+128
        cases[195]  = new Case10Z(false);                 // 1+2+64+128
        cases[197]  = new Case11XZ(false, true, false);   // 1+4+64+128
        cases[196]  = new Case5X(true, true, false);      // 4+64+128
        cases[200]  = new Case5X(true, true, true);       // 64+128+8
        cases[202]  = new Case11XZ(false, true, true);    // 2+8+64+128
        cases[208]  = new Case5Y(true, true, false);      // 16+64+128
        cases[209]  = new Case11YZ(true, false, false);   // 1+16+64+128
        cases[212]  = new Case9(true, true);              // 64+4+16+128
        cases[224]  = new Case5Y(true, true, true);       // 64+128+32
        cases[226]  = new Case11YZ(true, false, true);    // 2+32+64+128
        cases[228]  = new Case11XY(true, true, true);     // 32+128+4+64
        cases[255]  = NULL;
        // Special complementary cases
        cases[255-6]    = new Case3e(0, false, false, true);
        cases[255-9]    = new Case3e(0, false, false, false);
        cases[255-18]   = new Case3e(1, false, false, true);
        cases[255-20]   = new Case3e(2, false, true, false);
        cases[255-33]   = new Case3e(1, false, false, false);
        cases[255-40]   = new Case3e(2, false, true, true);
        cases[255-65]   = new Case3e(2, false, false, false);
        cases[255-72]   = new Case3e(1, false, true, true);
        cases[255-96]   = new Case3e(0, true, false, true);
        cases[255-130]  = new Case3e(2, false, false, true);
        cases[255-132]  = new Case3e(1, false, true, false);
        cases[255-144]  = new Case3e(0, true, false, false);
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
        const Vector &space, float step, bool smooth) {
    int width = space.x/step;
    int height = space.y/step;
    int depth = space.z/step;
    VoxelHelper helper(width, height, depth);
    Point *P = new Point[helper.GetNEdges()];
    Normal *N = NULL;
    if (smooth)
        N = new Normal[helper.GetNEdges()];
    vector<bool> inside(helper.GetNVerts(), false);
    Point o = Point(0,0,0) - space/2;
    ProgressReporter reporter(2*depth-1, "Marching cubes");
    for (int k = 0; k < depth; ++k, reporter.Update())
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j) {
                Point p = o + step*Vector(j, i, k);
                float dist = surface->Distance(p);
                if (k+1 < depth) {
                    int ind = helper.GetZIndex(i, j, k);
                    Point q = p + step*Vector(0.f, 0.f, 1.f);
                    float l = dist/(dist + surface->Distance(q));
                    P[ind] = (1.f - l)*p + l*q;
                    if (smooth)
                        N[ind] = surface->Gradient(P[ind]);
                }
                if (i+1 < height) {
                    int ind = helper.GetYIndex(i, j, k);
                    Point q = p + step*Vector(0.f, 1.f, 0.f);
                    float l = dist/(dist + surface->Distance(q));
                    P[ind] = (1.f - l)*p + l*q;
                    if (smooth)
                        N[ind] = surface->Gradient(P[ind]);
                }
                if (j+1 < width) {
                    int ind = helper.GetXIndex(i, j, k);
                    Point q = p + step*Vector(1.f, 0.f, 0.f);
                    float l = dist/(dist + surface->Distance(q));
                    P[ind] = (1.f - l)*p + l*q;
                    if (smooth)
                        N[ind] = surface->Gradient(P[ind]);
                }
                int ind = helper.GetVertIndex(i, j, k);
                inside[ind] = surface->Inside(o + step*Vector(j, i, k));
    }
    cases.Init();
    vector<int> inds;
    for (int k = 0; k < depth-1; ++k, reporter.Update())
        for (int i = 0; i < height-1; ++i)
            for (int j = 0; j < width-1; ++j) {
                int c = helper.CheckCase(inside, i, j, k);
                VoxelCase *which = cases[c] ? cases[c] : cases[255-c];
                if (which) {
                    log("Check %d (%d, %d, %d)/", c, i, j, k);
                    Point p = o + step*Vector(j, i, k);
                    log("(%.2f, %.2f, %.2f): ", p.x, p.y, p.z);
                    which->Generate(inds, helper, i, j, k);
                } else if (c != 0 && c != 255)
                    Warning("Missing marching cubes case %d or %d\n", c, 255-c);
            }
    reporter.Done();
    std::cout << "TRIANGLES: " << inds.size()/3 << std::endl;
    int *vi = new int[inds.size()];
    std::copy(inds.begin(), inds.end(), vi);
    ParamSet params;
    params.AddPoint("P", P, helper.GetNEdges());
    if (smooth)
        params.AddNormal("N", N, helper.GetNEdges());
    params.AddInt("indices", vi, inds.size());
    TriangleMesh *mesh = CreateTriangleMeshShape(o2w, w2o, reverseOrientation,
                                                 params, NULL);
    delete P;
    delete vi;
    return mesh;
}

