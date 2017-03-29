/**
 * \file Code to control the Sparkfun motor driver.
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __MOTOR_H
#define __MOTOR_H

//#include "../../common/fixedptc.h"
#include "../../common/fixed16.h"
#include "../../common/state.h"

#define STATE_STOP 0
#define STATE_BRAKE 1
#define STATE_FORW 2
#define STATE_REV 3

/// this is entirely ad-hoc, on a per-application basis:
/// current higher than this is considered Nominal - the motor
/// should be turning.
#define NOMINAL_CURRENT fixed16_fromint(10)
/// this is entirely ad-hoc, on a per-application basis:
/// current between this and nominal is LOW, not enough current
/// to turn the motor but some is passing indicating that the motor drive
/// lines are intact.
#define LOW_CURRENT fixed16_fromint(1)

/// there has to be an exception state for this long before the exception
/// is actually raised (to deal with inertia, etc.)
#define EXCEPTIONTICKS 50

/// this class encapsulates a low-level motor controller

class MotorController : public ExceptionListener {
    
    // the pin assignments for this motor
    int enablePin,posPin,negPin;
    
    /// the current state (see states above)
    int curState;
    
    /// a unique identifier for this slave - 0 or 1
    int id;
    
    /// the most recent current value - this is set by
    /// setCurrent() externally; you have to do this. Note
    /// that this is filtered using a simple LPF, but with 
    /// different hysteresis parameters for rising and falling
    /// momentary currents. The momentary current, however, is used
    /// for overcurrent tests.
    
    fixed16 current;
    
    /// sets the state (hi-stop,lo-stop,forward,reverse) and sets
    /// the positive and negative drive pins accordingly; also
    /// sets the enable pin if a BRAKE or STOP is called for. Called
    /// by setSpeed() if the direction changes, and in overcurrent
    /// and other exceptions.
    
    void setState(int state){
        if(state!=curState){
            int pos,neg;
            switch(state){
            case STATE_STOP:
                pos = LOW;
                neg = LOW;
                analogWrite(enablePin,0);
                break;
            case STATE_BRAKE:
                pos = HIGH;
                neg = HIGH;
                analogWrite(enablePin,MAXDUTY);
                break;
            case STATE_FORW:
                pos = HIGH;
                neg = LOW;
                break;
            case STATE_REV:
                pos = LOW;
                neg = HIGH;
                break;
            }
            digitalWrite(posPin,pos);
            digitalWrite(negPin,neg);
            curState=state;
        }
    }
    
    
public:    
    /// the magnitude of the control value being sent to the motor
    int control;
    /// the magnitude of the actual speed (set by descendants)
    int actualSpeed;
    
    /// the rising hysteresis value for current monitoring
    fixed16 hysterUp;
    /// the falling hysteresis value for current monitoring
    fixed16 hysterDown;
    
    /// the control level at which the motor *should* have
    /// started moving - above this level, the stall/fault
    /// checks will kick in
    int stallCheck;

    /// the threshold of ADC reading above which the motor
    /// should cut out (initially DEFAULT_OVERCURRENT_THRESHOLD)
    int overCurrentThreshold;
    
    /// how long we've been in a potential error state
    int stallCheckTicks;
    
    /// the default overcurrent (this value comes from the
    /// original code, it's apparently about 1.5A)
    static const int DEFAULT_OVERCURRENT_THRESHOLD = 150;
    
    /// the maximum duty cycle value (typically 255)
    static const int MAXDUTY=255;
    
    /// the constructor - but you'll have to call init() as well.
    MotorController(){
        current = fixed16_rconst(0);
        curState = -1; // invalid state, will be set in init by setSpeed
        hysterUp = fixed16_rconst(0.1f);
        hysterDown = fixed16_rconst(0.01f);
        stallCheck=120;
        stallCheckTicks=0;
    }        
    
    /// set up the pins for the motor and set initial state.
    /// Not a constructor so we can control when it happens.
    
    void init(int i,int e,int p, int n){
        id = i;
        enablePin = e;
        posPin = p;
        negPin = n;
        overCurrentThreshold = DEFAULT_OVERCURRENT_THRESHOLD;
        
        digitalWrite(posPin,LOW);
        digitalWrite(negPin,LOW);
        setSpeed(0);
        
        /// the motor will get notified whenever an exception
        /// occurs, and will go into unbraked stop.
        state.addExceptionListener(this);
    }
    
    /// get the motor ident
    int getID(){
        return id;
    }
    
    /// set the duty cycle of the motor 0 to MAXDUTY, and set
    /// the direction accordingly.
    ///
    /// Returns false if we can't do that because we're in exception,
    /// in which case the slave will have to have the exception
    /// reset.
    
    bool setSpeed(int speed){
        control = speed<0?-speed:speed;
        if(control>MAXDUTY)control=MAXDUTY;
        setState(speed>=0 ? STATE_FORW : STATE_REV);
        analogWrite(enablePin,control);
        return true;
    }
    
    /// set the current for this motor - typically called
    /// by an ISR that's doing analog reads. Will cut off
    /// the motor and set the speed to zero if we get an
    /// overcurrent condition. The motor will then need
    /// to be reset. The current value is stored and can
    /// be read by getCurrent(), having been passed through an
    /// LPF with two different (rising and falling) parameters.
    
    void setCurrent(int c){
        if(c<0)c=0;
        
        fixed16 q = fixed16_fromint(c);
        fixed16 param;
        
        /// we do the overcurrent check on the IMMEDIATE current value
        if(c>overCurrentThreshold){
            if(control>250) 
                // if control is high, then it's a genuine overcurrent
                state.raiseException(this,EX_OVERCURRENT);
            else
                // otherwise it's more likely to be a short in the drive
                // wires
                state.raiseException(this,EX_SHORTOC);
        }
        if(q>current){
            param = hysterUp;
        } else {
            param = hysterDown;
        }
        /// What we're modelling here: current = ((1.0f-param)*current)+(q*param);
        
        fixed16 t = fixed16_fromint(1)-param;
        t = fixed16_mul(t,current);
        current = t+fixed16_mul(q,param);
    }
    
    /// check the control value, actual speed and current
    /// to determine whether or not to throw an exception. 
    /// The overcurrent case is the only case where speed>0,
    /// and that's dealt with in setCurrent() - 
    /// Note that is is ALL CURRENTLY DISABLED! I keep getting
    /// EX_STALL for no reason even when stallCheck is set to 255,
    /// which can't be possible, so I'm turning it all off to see
    /// if it still happens.
    void checkForException(){
        return; /// STALL/FAULT HANDLING DISABLED!
        int ex=0;
        if(actualSpeed < 1){
            if(control>stallCheck){
                if(current>NOMINAL_CURRENT)
                    ex=EX_STALL;
                else if(current>=LOW_CURRENT)
                    ex=EX_ENCODERFAULT;
                else 
                    ex=EX_DRIVEFAULT;
            } else {
                // this clause is iffy because of the 
                // slow rate of decay of current readings,
                // and because generally either we have a short
                // or we don't.
//                if(current>NOMINAL_CURRENT)
//                    ex=EX_SHORTNC;
            }
        }
        if(ex)
            stallCheckTicks++;
        else
            stallCheckTicks=0;
        
        if(ex && stallCheckTicks>EXCEPTIONTICKS)
            state.raiseException(this,ex);
    }
            
    
    /// get the current value, which is an envelope-followed
    /// version of the value passed into setCurrent() as
    /// a fixed point value.
    
    int getCurrent(){
        return fixed16_toint(current);
    }
    
    /// all exceptions result in the motor stopping. Updates
    /// may appear to change this, but shouldn't really, since 
    /// the control loop doesn't run when we're in exception
    virtual void onException(MotorController *m,char type){
        setState(STATE_STOP);
    }
};


#endif /* __MOTOR_H */
