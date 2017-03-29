/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __HORMONE_H
#define __HORMONE_H

#include <vector>
#include <math.h>

inline float sigmoid(float x){
    x=((x-0.5f)*12.0f);
    return 1.0f/(1.0f+exp(-x));
}
    

/// a "hormone" object, which describes the concentration of a hormone
/// at a given site. Several of these objects could actually describe
/// different concentrations of the same hormone in different places.

struct Hormone {
    friend class HormoneSet;
    
    float conc; //!< concentration
    float decay; //!< decay rate
    
    // array of places the hormone diffuses *to*
    // (these should be reciprocated to make sense)
    Hormone *diffuseTo[16];
    float diffuseRate[16];
    int nDiffuseTo;

    /// update the hormone - typically called from HormoneSet
    void update(){
        conc *= decay;
        float q = conc;
        for(int i=0;i<nDiffuseTo;i++){
            float v = q * diffuseRate[i];
            diffuseTo[i]->add(v);
            conc -= v;
        }
    }
public:
    
    /// initialize the hormone
    Hormone(){
        nDiffuseTo=0;
        conc = 0;
        decay=0.999f;
    }
    
    /// set the exponential decay rate
    void setDecay(float d){
        decay=d;
    }
    
    /// get the current concentration
    float get(){
        return conc;
    }
    
    /// this site is connected to another site where a hormone object
    /// representing the same hormone is present. Every tick, a little
    /// of this hormone is taken and added to that one. If autoRecip is
    /// true, a reciprocal link with the same rate is also created.
    void addDiff(Hormone *h,float rate,bool autoRecip=true){
        diffuseTo[nDiffuseTo]=h;
        diffuseRate[nDiffuseTo]=rate;
        nDiffuseTo++;
        
        if(autoRecip){
            h->addDiff(this,rate,false);
        }
    }
    
    /// add to the concentration
    void add(float v){
        conc += v;
    }
                      
    
};

/// a set of hormones.

class HormoneSet {
    /// the internal list
    std::vector<Hormone *> vec;
    
public:
    /// create and add a new hormone, returning it that parameters
    /// may be set.
    Hormone *create(){
        Hormone *h = new Hormone();
        vec.push_back(h);
        return h;
    }
    
    /// update all the hormones in the set.
    void update(){
        for(std::vector<Hormone *>::iterator it = vec.begin();it!=vec.end();++it){
            (*it)->update();
        }
    }
};

#endif /* __HORMONE_H */
