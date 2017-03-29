/**
 * \file Position controller wrapper around motor 
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __POSMOTOR_H
#define __POSMOTOR_H

#include "pid.h"
#include "adc.h"

/// decay factor for the error integral
#define ERROR_INTEGRAL_DECAY	0.92f

/// controller for a motor whose position we want to manage,
/// which is connected to a potentiometer. Usage:
/// - initialise, providing the enable, +ve and -ve pin numbers
/// - call update() periodically
/// - call onADC() when ADC completes on the potentiometer
/// - naturally, set the parameters

class PositionMotorController : public PIDController, public ADCListener {
    
    /// true when the actual position has been set from outside (via ADC)
    /// so we can run the control algorithm
    bool updated;
public:
    
    /// the value which map onto 0 in the position reading
    float calibMin;
    /// the value which map onto 1024 in the position reading
    float calibMax;
    
    /// set up pin assignments etc.
    void init(int id, 
              int enablePin,
              int posPin, 
              int negPin){
        PIDController::init(id,enablePin,posPin,negPin); // motor init
        updated = false;
        calibMin = -512;
        calibMax = 512;
    }
    
    /// run the update if the actual position has been set
    /// from outside.
    
    void update(){
        if(updated){
            updated = false;
            
            if(state.exception){
                // set required position to actual position,
                // it'll need to be sent again from the master
                // after reset.
                required=actual;
            }
            
            // we still do the update if in exception, but we don't
            // set the control to the motor in that case.
            if(calculatePIDCorrection()){
                // calculate correction and set as motor speed
                
                float ctl = pGain*error+iGain*errorIntegral+dGain*errorDerivative;
                if(!state.exception)
                    setSpeed(ctl);
            }
            actualSpeed = errorDerivative; // we can do this because it's not *really* the error derivative
            if(actualSpeed<0)actualSpeed=-actualSpeed;
            checkForException();
        }
    }
    
    /// set the position as read from the potentiometer,
    /// or the current - this is called from the ADC system.
    
    virtual void onADC(int type, int p){
        if(type == TYPE_CURRENT)
            setCurrent(p);
        else {
            // here, we map from the potentiometer reading to the calibrated
            // reading- assuming the pot is 0-1024
            
            actual = (float)p;
            actual *= (calibMax-calibMin);
            actual/=1024.0f;
            actual += calibMin;
            updated=true;
        }
    }
};



#endif /* __POSMOTOR_H */
