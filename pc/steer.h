/**
 * \file
 * a class which communicates with a steer motor via a slave device
 */


#ifndef __STEER_H
#define __STEER_H

#include "slave.h"
#include "motor.h"

/// steer motors are accessed by the REGDS_STEER_ registers,
/// and there is only one steer motor on a slave device,
/// which must be a DS (drive/steer) board.

class SteerMotor : public Motor{
    /// the parameter block
    PosMotorParams params;

public:
    /// constructor, specifying the slave we're talking to
    SteerMotor(SlaveDevice *s) : Motor(s){}
    
    virtual void sendParams(){
        slave->startWrites();
        slave->writeFloat(REGDS_STEER_PGAIN,params.pGain);
        slave->writeFloat(REGDS_STEER_IGAIN,params.iGain);
        slave->writeFloat(REGDS_STEER_DGAIN,params.dGain);
        slave->writeFloat(REGDS_STEER_INTEGRALCAP,params.iCap);
        slave->writeFloat(REGDS_STEER_INTEGRALDECAY,params.iDecay);
        slave->writeFloat(REGDS_STEER_OVERCURRENTTHRESH,params.overCurrentThresh);
        slave->writeFloat(REGDS_STEER_CALIBMIN,params.calibMin);
        slave->writeFloat(REGDS_STEER_CALIBMAX,params.calibMax);
        slave->writeFloat(REGDS_STEER_STALLCHECK,params.stallCheck);
        slave->writeFloat(REGDS_STEER_DEADZONE,params.deadZone);
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
        
        slave->startWrites();
        slave->writeFloat(REGDS_STEER_REQPOS,pos);
        slave->endWrites();
        required = pos;
    }
};
    


#endif /* __STEER_H */
