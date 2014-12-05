// shapes/metaball.cpp*
#include "stdafx.h"
#include "shapes/metaball.h"
#include "montecarlo.h"
#include "paramset.h"

// Metaball Method Definitions
Metaball::Metaball(const Transform *o2w, const Transform *w2o, bool ro,
               float rad, float z0, float z1, float pm)
    : Shape(o2w, w2o, ro) {
        
    //instances.push_back(new MetaballInstance(o2w, w2o, ro, rad, z0, z1, pm));
        
//    Transform *o = new Transform();
//    *o = Translate(Vector(0.1, 0.1, 0.1)) * *o2w;
//    Transform *w = new Transform();
//    *w = *w2o * Translate(Vector(-0.1, -0.1, -0.1));
//    instances.push_back(new MetaballInstance(o, w, ro, rad, z0, z1, pm));
}


BBox Metaball::ObjectBound() const {

    BBox box = instances[0]->ObjectBound();
    
    for (int i = 1; i < instances.size(); i++) {
        BBox instanceBox = instances[i]->ObjectBound();
        box = Union(box, instanceBox);
    }
    
    return box;
}


bool Metaball::Intersect(const Ray &r, float *tHit, float *rayEpsilon,
                       DifferentialGeometry *dg) const {
    
    for (int i = 0; i < instances.size(); i++) {
        if (instances[i]->Intersect(r, tHit, rayEpsilon, dg)) {
            return true;
        }
    }
    
    return false;
}


bool Metaball::IntersectP(const Ray &r) const {
    
    for (int i = 0; i < instances.size(); i++) {
        if (instances[i]->IntersectP(r)) {
            return true;
        }
    }
    
    return false;
}


float Metaball::Area() const {

    float area = 0;
    
    for (int i = 0; i < instances.size(); i++) {
        area += instances[i]->Area();
    }
    
    return area;
}


Metaball *CreateMetaballShape(const Transform *o2w, const Transform *w2o,
        bool reverseOrientation, const ParamSet &params) {
    
    float radius = params.FindOneFloat("radius", 1.f);
    float zmin = params.FindOneFloat("zmin", -radius);
    float zmax = params.FindOneFloat("zmax", radius);
    float phimax = params.FindOneFloat("phimax", 360.f);
    return new Metaball(o2w, w2o, reverseOrientation, radius,
                      zmin, zmax, phimax);
}


Point Metaball::Sample(float u1, float u2, Normal *ns) const {
    
    return instances[0]->Sample(u1, u2, ns);
}


Point Metaball::Sample(const Point &p, float u1, float u2,
                     Normal *ns) const {
    
    return instances[0]->Sample(p, u1, u2, ns);
}


float Metaball::Pdf(const Point &p, const Vector &wi) const {
    
    return instances[0]->Pdf(p, wi);
}


