/**
 * \file 
   A class which communicates with a steer motor via a slave device.
 * It relies heavily on the registers of lift motor one
 * immediately preceding those of lift motor two and
 * being identical to them!
 * 
 */


#ifndef __LIFT_H
#define __LIFT_H

#include "slave.h"
#include "motor.h"

/// lift motors are accessed by the regOffset+REGLL_ONE_ registers,
/// and there is only one lift motor on a slave device,
/// which must be a LL (lift/lift) board.

class LiftMotor : public Motor {
private:
    int motor; // which of the two lift motors, 0 or 1
    int regOffset; //!< the offset to be added to the register number!
    /// the parameter block
    PosMotorParams params;
    
    bool isAdjacencyViolated(float req);
    
public:
    /// constructor, specifying the slave we're talking to
    /// and which of the two lift motors
    LiftMotor(SlaveDevice *s,int m) : Motor(s){
        motor = m;
        // calculate the offset, if any
        regOffset = m * (REGLL_TWO_REQPOS-REGLL_ONE_REQPOS);
    }
    
    virtual void sendParams(){
        slave->startWrites();
        slave->writeFloat(regOffset+REGLL_ONE_PGAIN,params.pGain);
        slave->writeFloat(regOffset+REGLL_ONE_IGAIN,params.iGain);
        slave->writeFloat(regOffset+REGLL_ONE_DGAIN,params.dGain);
        slave->writeFloat(regOffset+REGLL_ONE_INTEGRALCAP,params.iCap);
        slave->writeFloat(regOffset+REGLL_ONE_INTEGRALDECAY,params.iDecay);
        slave->writeFloat(regOffset+REGLL_ONE_OVERCURRENTTHRESH,params.overCurrentThresh);
        slave->writeFloat(regOffset+REGLL_ONE_CALIBMIN,params.calibMin);
        slave->writeFloat(regOffset+REGLL_ONE_CALIBMAX,params.calibMax);
        slave->writeFloat(regOffset+REGLL_ONE_STALLCHECK,params.stallCheck);
        slave->writeFloat(regOffset+REGLL_ONE_DEADZONE,params.deadZone);
        slave->endWrites();
    }
        
    /// return the current parameter block for modification
    /// or examination
    virtual MotorParams *getParams(){
        return &params;
    }
    /// return the current parameter block for modification
    /// or examination
    PosMotorParams *getPosParams(){
        return &params;
    }
    
    /// send a new position request
    virtual void setRequired(float pos){
        if(pos>(params.calibMax-10) || pos<(params.calibMin+10))
            throw ConstraintException("required position out of range");
        if(isAdjacencyViolated(pos))
            throw ConstraintException("lift motor collision possibility");
           
        
        slave->startWrites();
        slave->writeFloat(regOffset+REGLL_ONE_REQPOS,pos);
        slave->endWrites();
        required = pos;
    }
    
    /// the wheel number - originally the code didn't need to know
    /// this, but the lift wheel constraints make it necessary. It's
    /// set from the rover code once everything else has been initialised.
    
    int wheelNumber;
};
    


#endif /* __LIFT_H */
