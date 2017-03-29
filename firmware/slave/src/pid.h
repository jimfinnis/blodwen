/**
 * \file Describes a PID controller wrapper around a motor.
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __PID_H
#define __PID_H

#include "motor.h"

/// PID controller wrapping a motor controller.
class PIDController : public MotorController {
    /// previous actual value
    float prevActual;
    
    /// previous time, for interval calculation
    unsigned long prevTime;
    
protected:
    /// Works out the interval between updates, and calculates
    /// the PID values.
    /// Work out the interval between this update and the previous
    /// one, updating various values. If the clock has wrapped around,
    /// return false to indicate this - the calling update method should
    /// return immediately.
    
    bool calculatePIDCorrection(){
        unsigned long now = micros();
        if(now<prevTime){ // skip the update if micros wrapped around
            prevTime = now;
            return false;
        }
            
        unsigned long micInterval = now-prevTime;
        milliInterval = (int)(micInterval/1000L);
        prevTime = now; // Another rather important line...
            
        float interval = ((float)micInterval) * 1.0e-6f; // interval in seconds
        
        // get error
        error = required - actual;
        // dead zone
        if(error<0 && error>-deadZone ||
           error>0 && error<deadZone)error=0;
            
        // add to integral
        errorIntegral += error;
        errorIntegral *= integralDecay;
        
        if(errorIntegral>integralCap)
            errorIntegral=integralCap;
        if(errorIntegral<-integralCap)
            errorIntegral=-integralCap;
        
        // calculate derivative of MOTOR POSITION - see the wikipedia
        // page for why I do this rather than the more obvious error
        // deriv. Note that if the set point is the same, it amounts to
        // the same thing :)
        // NOTE THE NEGATION!
            
        errorDerivative = -(actual-prevActual);
        prevActual = actual;
            
        return true;
    }
    
public:
    
    /// required value
    float required;
    /// actual value
    float actual;
    
    /// time between last tick and one before that (ticks occur when the
    /// encoder has updates its data) (milliseconds)
    int milliInterval;
    
    /// proportional gain
    float pGain;
    /// integral gain
    float iGain;
    /// differential gain
    float dGain;
    /// error value
    float error;
    /// integral of error for I-term calculation
    float errorIntegral;
    /// derivative of error for D-term calculation - *really* it's the position derivative!
    float errorDerivative;
    
    /// the value at which the error integral is clamped - writable
    float integralCap;
    
    /// the error integral's decay term
    float integralDecay;
    
    /// below this, the error is assumed to be zero
    float deadZone;
    
    
    /// set up pin assignments etc.
    void init(int id, 
              int enablePin,
              int posPin, 
              int negPin){
        MotorController::init(id,enablePin,posPin,negPin); // motor init
        
        // gains initially zero
        pGain = 0.0f;
        iGain = 0.0f;
        dGain = 0.0f;
        
        prevActual = 0;
        actual = 0;
        
        errorIntegral = 0;
        integralCap=0;
        integralDecay=0;
        overCurrentThreshold=300;
        milliInterval=0;
        required=0;
        deadZone=0;
        prevTime = micros();
    }
    
};
    

#endif /* __PID_H */
