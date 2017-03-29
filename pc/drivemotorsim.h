/**
 * \file
 *Drive motor simulator
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __MOTORSIM_H
#define __MOTORSIM_H

/// smoothing factor for filter which converts required to actual
/// for drive motors. It's Brown's simple exponential smoothing.
#define DRIVEMOTORLPF 0.05f


/// simple simulator class for the motors

class MotorSim {
    static MotorSim *motors[6];
    
    /// actual speed of drive motor - filtered version of required speed.
    float actual;
    
    inline float interp(float a,float b,float t){
        return t*b + (1.0f-t)*a;
    }
    
    /// update a motor given the average required speed
    void update(float avgReq){
        
        // update the actual speed using Brown's simple exponential
        // smoothing (something I've used for years but never knew
        // the name of!)
        actual = DRIVEMOTORLPF*required + (1.0f-DRIVEMOTORLPF)*actual;
        
        // the stuff below here is just to work out a fake temperature,
        // for some TAROS2013 experiments.
        
        // calc difference from average speed
        float diff = req-avgReq;
        
        // perform some function to get the load. Here, it's the
        // speed * 0.001 (so 500 gives 0.5) plus the diff * 0.002 (so
        // 200 gives 0.4)
        float load = req / 700.0f + diff * 0.002;
        
        static int qqqq=0;
        if(qqqq++>300){
//            printf("Motor : req %10f   diff %10f  load %10f\n",
//                   req,diff,load);
            qqqq=0;
        }
        
        // and now perform the heating function - assuming the tick length is
        // 1s. The formula is ax+b, where a is the decay constant and b is
        // the heating.
        temp = temp * 0.99783;
        // interpolate between some cooling value and the value obtained from
        // experiment 'base'
        temp = temp + 0.0366342*load*(1.0f+rand*0.5f);
    }
    

public:
    MotorSim(int i){
        temp=2.5f;
        req=0;
        motors[i]=this;
        actual=0;
        rand = ((float)i)/3.0f;
//        rand = (i==3)?1:-1; // uncomment for one bad motor
    }
    
    
    float temp;
    float req;
    float rand;
    
    void setReq(float r){
        req = r;
    }
    
    static void updateAll(){
        // calculate average required speed of all motors
        float avgReq=0;
        for(int i=0;i<6;i++)
            avgReq += motors[i]->req;
        avgReq/=6.0f;
        
        // then update the motor
        for(int i=0;i<6;i++)
            motors[i]->update(avgReq);
    }
    
    
};

#endif /* __MOTORSIM_H */
