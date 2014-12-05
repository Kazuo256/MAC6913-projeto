// shapes/metaball.cpp*
#include "stdafx.h"
#include "shapes/metaball.h"
#include "montecarlo.h"
#include "paramset.h"

// Metaball Method Definitions
Metaball::Metaball(const Transform *o2w, const Transform *w2o, bool ro,
                   int nbumps, const Point *centers, const float *radius,
                   const float *blobbiness)
    : Shape(o2w, w2o, ro) {
        
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

    // Let's find out the initial candidates
    vector<float> candidates;
    for (size_t i = 0; i < order.size(); ++i) {
        MetaballInstance *bump = instances[order[i]];
        // This is the t such that ray(t) is closest to p_i along the ray
        float t = Dot(bump->GetCenter() - ray.o, ray.d)/ray.d.LengthSquared();
        // If it is inside, use local solutions. Otherwise, use t itself.
        // Must be sure the candidates lie within the [mint, maxt] interval.
        if (Inside(ray(t))) {
            float t0, t1;
            if (bump->IntersectAll(ray, &t0, &t1)) {
                if (t0 > ray.mint && t1 < ray.maxt) {
                  candidates.push_back(t0);
                  candidates.push_back(t1);
                } else {
                    if (t0 < ray.maxt)
                        candidates.push_back(t0);
                    if (t1 > ray.mint)
                        candidates.push_back(t1);
                }
            }
        } else if (t > ray.mint && t < ray.maxt) {
            candidates.push_back(t);
        }
    }
    
    return false;
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

bool Metaball::Inside(const Point &p) const {
    float d = 0;
    for (size_t i = 0; i < instances.size(); ++i) {
        MetaballInstance *bump = instances[i];
        Vector v = bump->GetCenter() - p;
        float r2 = v.x*v.x + v.y*v.y + v.z*v.z;
        float B = -bump->GetBlobbiness();
        float R = bump->GetRadius();
        float x = B/(R*R)*r2 - B;
        d += exp(x);
    }
    return d > 1;
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


