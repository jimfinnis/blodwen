/**
 * \file 
 * Code dealing with motors in a general way - many
 * of these are extended by specific lift, steer and drive motor
 * classes.
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __MOTOR_H
#define __MOTOR_H
#include "roverexcept.h"

/// constraint exception
class ConstraintException : public RoverException {
public:
    ConstraintException(const char *m){
        strcpy(msg,m);
    }
};

/// base motor parameter class - extended
/// for particular motor types

struct MotorParams {
    float pGain;//!< proportional gain
    float iGain;//!< integral gain
    float dGain;//!< differential gain
    float iCap;//!< cap on integral error
    float iDecay;//!< integral error multiplied by this each loop
    float overCurrentThresh; //!< overcurrent threshold
    float stallCheck; //!< control level at which stall/fault checks kick in
    float deadZone; //!< dead zone for error - below this magnitude, error is considered to be zero
    
    MotorParams(){
        reset();
    }
    
    virtual void reset(){
        pGain = iGain = dGain = 0;
        iCap = iDecay = 0;
        overCurrentThresh = 10; //small
        stallCheck = 255; // STALL CHECK DISABLED
        deadZone = 1;
    }
};

/// extended motor parameters for a positional motor, includes calibration data etc.
struct PosMotorParams : public MotorParams {
    float calibMin; //!< 0 on pot is mapped to this
    float calibMax; //!< max on pot is mapped to this
    virtual void reset(){
        MotorParams::reset();
        calibMin = -100;
        calibMax = 100;
    }
};


/// motor class - extended by particular motor types

class Motor {
protected:
    /// the slave device to which I am communicating
    SlaveDevice *slave; //!< the slave device to which I'm communicating
    float required; //!< the value I'm required to get to
    
public:
    Motor(SlaveDevice *s){
        slave = s;
        required = 0;
    }
    
    /// return the base parameter data; if you want the specific
    /// subclass, use getPosParams() or getSpeedParams() (which doesn't
    /// exist yet)
    virtual MotorParams *getParams()=0;
    
    /// send an updated copy of the parameters
    virtual void sendParams()=0;
    
    /// return the last required value
    float getRequired(){
        return required;
    }
    
    /// set the required value
    virtual void setRequired(float req)=0;
};
    
        
    
#endif /* __MOTOR_H */
