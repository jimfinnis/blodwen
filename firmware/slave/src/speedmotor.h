/**
 * \file Speed controller wrapper around motor and quad encoder
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __SPEEDMOTOR_H
#define __SPEEDMOTOR_H

#include "pid.h"
#include "quadencoder.h"
#include "adc.h"

/// decay factor for the error integral
#define ERROR_INTEGRAL_DECAY	0.92f
/// decay factor for the motor speed (to ensure that once the motor is stopped we don't have too much current)
/// which only comes into play when the required speed is zero
#define MOTOR_SPEED_DECAY	0.96f

/// Controller for a motor whose speed we want to manage, which is
/// attached to a quadrature encoder. Usage:
/// - initialise, providing the enable, +ve and -ve pin numbers and encoder channel pins
/// - call update() periodically
/// - call tickEncoder() on the encoder channel A rising edge interrupt
/// - oh, and set the parameters for the gains

class SpeedMotorController : public PIDController, public ADCListener {
private:
    
    /// my encoder
    QuadEncoder e;
    
    /// speed value - general
    float ctl;
    
public:
    
    /// set up pin assignments
    void init(int id, 
              int enablePin,
              int posPin, 
              int negPin, 
              int encA, int encB){
        PIDController::init(id,enablePin,posPin,negPin); // motor init
        ctl=0;
    }
    
    void update(){
        if(e.update()){ // re-read the encoder and do the algorithm if we got a new reading
            actual = e.getTickFreq();
            
            if(state.exception){
                // set required speed to zero,
                // it'll need to be sent again from the master
                // after reset.
                required=0;
            }
            
            // we still do the update if in exception, but we don't
            // set the control to the motor in that case.
            if(calculatePIDCorrection()){
            
                // calculate correction
                float t = pGain*error+iGain*errorIntegral+dGain*errorDerivative;
            
                // apply to motor speed
                ctl+= t;
            
                if(required<0.001f && required>-0.001f)
                    ctl *= MOTOR_SPEED_DECAY; // to gradually drop the motor current
                if(!state.exception)
                    setSpeed(ctl);
            }
            checkForException();
        }
        // get this for the lower level current etc. checks
        actualSpeed=actual;
        if(actualSpeed<0)actualSpeed=-actualSpeed;
    }
    
    void tickEncoder(){
        e.tick();
    }
    
    /// this gets the odometry from the encoder and shifts it down
    uint16_t getOdometry(){
        return (uint16_t)(e.getOdometry()>>8L);
    }
    
    void resetOdometry(){
        e.resetOdometry();
    }
    
    /// set the current - this is called from the ADC system.
    
    virtual void onADC(int type, int p){
        if(type == TYPE_CURRENT)
            setCurrent(p);
    }
    
};


#endif /* __SPEEDMOTOR_H */
