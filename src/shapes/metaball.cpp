// shapes/metaball.cpp*
#include "stdafx.h"
#include "shapes/metaball.h"
#include "montecarlo.h"
#include "paramset.h"

// Metaball Method Definitions
Metaball::Metaball(const Transform *o2w, const Transform *w2o, bool ro,
                   int nbumps, const Point *centers, const float *radius,
                   const float *blobbiness, float err)
    : Shape(o2w, w2o, ro), error(err) {
        
    for (int i = 0; i < nbumps; ++i)
        instances.push_back(new MetaballInstance(o2w, w2o, ro, centers[i], radius[i],
                                                 blobbiness[i]));
        
//    Transform *o = new Transform();
//    *o = Translate(Vector(0.1, 0.1, 0.1)) * *o2w;
//    Transform *w = new Transform();
//    *w = *w2o * Translate(Vector(-0.1, -0.1, -0.1));
//    instances.push_back(new MetaballInstance(o, w, ro, rad, z0, z1, pm));
}

Metaball::~Metaball() {
    for (size_t i = 0; i < instances.size(); ++i)
        delete instances[i];
    instances.clear();
}


BBox Metaball::ObjectBound() const {

    BBox box = instances[0]->ObjectBound();
    
    for (size_t i = 1; i < instances.size(); i++) {
        BBox instanceBox = instances[i]->ObjectBound();
        box = Union(box, instanceBox);
    }
    
    return box;
}

struct CompareOp {
    CompareOp(const vector<MetaballInstance*> &the_instances, const Vector &the_dir)
        : instances(the_instances), dir(the_dir) {}

    bool operator()(int lhs_idx, int rhs_idx) {
        MetaballInstance *lhs = instances[lhs_idx],
                         *rhs = instances[rhs_idx];
        float l = Dot(lhs->GetCenter()-Point(), dir);
        float r = Dot(rhs->GetCenter()-Point(), dir);
        return l < r;
    }

    const vector<MetaballInstance*> &instances;
    Vector                          dir;
};


bool Metaball::Intersect(const Ray &r, float *tHit, float *rayEpsilon,
                       DifferentialGeometry *dg) const {
    
    Ray ray;
    (*WorldToObject)(r, &ray);
    vector<int> order(instances.size(), 0);

    for (size_t i = 0; i < instances.size(); i++) {
        order[i] = i;
        //if (instances[i]->Intersect(r, tHit, rayEpsilon, dg)) {
        //    return true;
        //}
    }

    // Order bumps along the ray direction
    stable_sort(order.begin(), order.end(), CompareOp(instances, ray.d));

    // Let's find the initial candidate
    float tn = ray.mint,
          tf = .0f;
    float vn = ValueAt(ray(tn)),
          vf = .0f;
    size_t i;
    for (i = 0; i < order.size(); ++i) {
        MetaballInstance *bump = instances[order[i]];
        // This is the t such that ray(t) is closest to p_i along the ray
        tf = Dot(bump->GetCenter() - ray.o, ray.d)/ray.d.LengthSquared();
        // Found no intersection
        if (tf >= ray.maxt) return false;
        // If t intersects with this specific bump, use the local solution
        if (bump->ValueAt(ray(tf)) > 1) {
          bump->IntersectFirst(ray, &tf);
          vf = ValueAt(ray(tf));
          break;
        }
        // Otherwise, use this t if it is inside globally
        vf = ValueAt(ray(tf));
        if (vf > 1) break;
        // Nothing worked, go to next one
        tn = tf;
        vn = vf;
    }
    if (i >= order.size()) return false;

    float t, v;
    if (fabsf(vn - 1) < fabsf(vf - 1))
        t = tn, v = vn; 
    else
        t = tf, v = vf;

    //while (fabsf(v - 1) > error) {

    //}
    
    return true;
}


bool Metaball::IntersectP(const Ray &r) const {
    
    for (size_t i = 0; i < instances.size(); i++) {
        if (instances[i]->IntersectP(r)) {
            return true;
        }
    }
    
    return false;
}


float Metaball::Area() const {

    float area = 0;
    
    for (size_t i = 0; i < instances.size(); i++) {
        area += instances[i]->Area();
    }
    
    return area;
}

float Metaball::ValueAt(const Point &p) const {
    float d = 0;
    for (size_t i = 0; i < instances.size(); ++i)
        d += instances[i]->ValueAt(p);
    return d;
}

bool Metaball::Inside(const Point &p) const {
    return ValueAt(p) > 1;
}

Metaball *CreateMetaballShape(const Transform *o2w, const Transform *w2o,
        bool reverseOrientation, const ParamSet &params) {

    int nbumps, nradius, nblobs;
    const Point *P = params.FindPoint("P", &nbumps); 
    const float *R = params.FindFloat("R", &nradius);
    const float *B = params.FindFloat("B", &nblobs);
    if (nbumps != nradius || nbumps != nblobs) {
        Error("Number of points, radius and blobbiness must match");
        return NULL;
    }
    return new Metaball(o2w, w2o, reverseOrientation, nbumps, P, R, B);
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


