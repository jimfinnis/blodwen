/** -!- cmode; c++ -!- 

 * \file Interfacing the motors with the I2C comms
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include "config.h"
#include "../../common/state.h"
#include "slave_i2c.h"
#include "speedmotor.h"
#include "posmotor.h"
#include "chassis.h"
#include <avr/wdt.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#if DRIVESTEER
extern PositionMotorController steerMotor;
extern SpeedMotorController driveMotor;
extern Chassis chassis;
#else
extern PositionMotorController lift1,lift2;
#endif
extern MotorController *motors[];


extern char ledCounts[];

/// Process register write events

class MyI2CListener : public I2CSlaveListener {
    /// called when i2c changes a register
    virtual void changeEvent(int reg,uint16_t val) {
        float t;
#if DRIVESTEER
        const Register *regs=registerTable_DS;
#else
        const Register *regs=registerTable_LL;
#endif
        
        // if this is a substantive register which actually
        // affects the motors, do not allow a change if we are
        // in the exception state. CHANGE: we do allow reg changes
        // in exception now!
        //if(reg>REG_DEBUG && state.exception)return;
        
        switch(reg){
            
            // general purpose registers
        
        case REG_DEBUGLED:
            if(val<2)
                ledCounts[val]=100;
            break;
        case REG_EXCEPTIONDATA:
            // notification from the master that another slave
            // has a problem
            state.raiseException(NULL,EX_REMOTESLAVE);
            break;
        case REG_DISABLEDEXCEPTIONS:
            state.setDisabledExceptions(val);
            break;
        case REG_PING:
#if DRIVESTEER
            ledCounts[0]=20;
#else
            ledCounts[1]=20;
#endif
            break;
        case REG_RESET:
            I2CSlave::setRegisterInt(REG_RESET,0); // clear it back!
            if(val & RESET_HARD){ // HARD RESET
                // basically we just spin around this loop
                // until the watchdog notices!
                while(1){
                    delay(1);
                }
            }
            if(val & RESET_EXCEPTIONS) // reset exceptions
                state.reset();
#if DRIVESTEER
            if(val & RESET_ODO)
                driveMotor.resetOdometry();
#endif
            break;
            
#if DRIVESTEER
            // drive motor registers
            
        case REGDS_DRIVE_REQSPEED:
            driveMotor.required=regs[reg].unmap(val);
            break;
        case REGDS_DRIVE_PGAIN:
            driveMotor.pGain=regs[reg].unmap(val);
            break;
        case REGDS_DRIVE_IGAIN:
            driveMotor.iGain=regs[reg].unmap(val);
            break;
        case REGDS_DRIVE_DGAIN:
            driveMotor.dGain=regs[reg].unmap(val);
            break;
        case REGDS_DRIVE_INTEGRALCAP:
            driveMotor.integralCap=regs[reg].unmap(val);
            break;
        case REGDS_DRIVE_STALLCHECK:
            driveMotor.stallCheck=regs[reg].unmap(val);
            break;
        case REGDS_DRIVE_DEADZONE:
            driveMotor.deadZone=regs[reg].unmap(val);
            break;
        case REGDS_DRIVE_OVERCURRENTTHRESH:
            driveMotor.overCurrentThreshold=regs[reg].unmap(val);
            break;
        case REGDS_DRIVE_INTEGRALDECAY:
            driveMotor.integralDecay=regs[reg].unmap(val);
            break;
            
            
            // speed motor registers
            
        case REGDS_STEER_REQPOS:
            steerMotor.required=regs[reg].unmap(val);
            break;
        case REGDS_STEER_PGAIN:
            steerMotor.pGain = regs[reg].unmap(val);
            break;
        case REGDS_STEER_IGAIN:
            steerMotor.iGain = regs[reg].unmap(val);
            break;
        case REGDS_STEER_DGAIN:
            steerMotor.dGain= regs[reg].unmap(val);
            break;
        case REGDS_STEER_INTEGRALCAP:
            steerMotor.integralCap= regs[reg].unmap(val);
            break;
        case REGDS_STEER_STALLCHECK:
            steerMotor.stallCheck=regs[reg].unmap(val);
            break;
        case REGDS_STEER_DEADZONE:
            steerMotor.deadZone=regs[reg].unmap(val);
            break;
        case REGDS_STEER_OVERCURRENTTHRESH:
            steerMotor.overCurrentThreshold= regs[reg].unmap(val);
            break;
        case REGDS_STEER_INTEGRALDECAY:
            steerMotor.integralDecay= regs[reg].unmap(val);
            break;
        case REGDS_STEER_CALIBMIN:
            steerMotor.calibMin = regs[reg].unmap(val);
            break;
        case REGDS_STEER_CALIBMAX:
            steerMotor.calibMax = regs[reg].unmap(val);
            break;
#else
            // now the lift motors
            
        case REGLL_ONE_REQPOS:
            lift1.required=regs[reg].unmap(val);
            break;
        case REGLL_ONE_PGAIN:
            lift1.pGain = regs[reg].unmap(val);
            break;
        case REGLL_ONE_IGAIN:
            lift1.iGain = regs[reg].unmap(val);
            break;
        case REGLL_ONE_DGAIN:
            lift1.dGain= regs[reg].unmap(val);
            break;
        case REGLL_ONE_INTEGRALCAP:
            lift1.integralCap= regs[reg].unmap(val);
            break;
        case REGLL_ONE_OVERCURRENTTHRESH:
            lift1.overCurrentThreshold= regs[reg].unmap(val);
            break;
        case REGLL_ONE_INTEGRALDECAY:
            lift1.integralDecay= regs[reg].unmap(val);
            break;
        case REGLL_ONE_CALIBMIN:
            lift1.calibMin = regs[reg].unmap(val);
            break;
        case REGLL_ONE_CALIBMAX:
            lift1.calibMax = regs[reg].unmap(val);
            break;
        case REGLL_ONE_DEADZONE:
            lift1.deadZone=regs[reg].unmap(val);
            break;
        case REGLL_TWO_REQPOS:
            lift2.required=regs[reg].unmap(val);
            break;
        case REGLL_TWO_PGAIN:
            lift2.pGain = regs[reg].unmap(val);
            break;
        case REGLL_TWO_IGAIN:
            lift2.iGain = regs[reg].unmap(val);
            break;
        case REGLL_TWO_DGAIN:
            lift2.dGain= regs[reg].unmap(val);
            break;
        case REGLL_TWO_INTEGRALCAP:
            lift2.integralCap= regs[reg].unmap(val);
            break;
        case REGLL_TWO_OVERCURRENTTHRESH:
            lift2.overCurrentThreshold= regs[reg].unmap(val);
            break;
        case REGLL_TWO_INTEGRALDECAY:
            lift2.integralDecay= regs[reg].unmap(val);
            break;
        case REGLL_TWO_CALIBMIN:
            lift2.calibMin = regs[reg].unmap(val);
            break;
        case REGLL_TWO_CALIBMAX:
            lift2.calibMax = regs[reg].unmap(val);
            break;
        case REGLL_TWO_STALLCHECK:
            lift2.stallCheck=regs[reg].unmap(val);
            break;
        case REGLL_TWO_DEADZONE:
            lift2.deadZone=regs[reg].unmap(val);
            break;
#endif        
        default:break;
        }
    }
};

static MyI2CListener listener;


void initI2C(int addr){
    
    
    // all registers are reset by default to zero as an unsigned int, i.e. the bottom
    // of the range for registers which are float-mapped.
    // Here we have to make sure the register maps the actual value.
#if DRIVESTEER    
    I2CSlave::init(addr,registerTable_DS);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_REQSPEED,driveMotor.required);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_PGAIN,driveMotor.pGain);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_IGAIN,driveMotor.iGain);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_DGAIN,driveMotor.dGain);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_INTEGRALCAP,driveMotor.integralCap);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_INTEGRALDECAY,driveMotor.integralDecay);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_OVERCURRENTTHRESH,driveMotor.overCurrentThreshold);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_CURRENT,driveMotor.getCurrent());
    I2CSlave::setRegisterFloat(REGDS_DRIVE_STALLCHECK,driveMotor.stallCheck);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_DEADZONE,driveMotor.deadZone);
    
    I2CSlave::setRegisterFloat(REGDS_STEER_REQPOS,steerMotor.required);
    I2CSlave::setRegisterFloat(REGDS_STEER_PGAIN,steerMotor.pGain);
    I2CSlave::setRegisterFloat(REGDS_STEER_IGAIN,steerMotor.iGain);
    I2CSlave::setRegisterFloat(REGDS_STEER_DGAIN,steerMotor.dGain);
    I2CSlave::setRegisterFloat(REGDS_STEER_INTEGRALCAP,steerMotor.integralCap);
    I2CSlave::setRegisterFloat(REGDS_STEER_INTEGRALDECAY,steerMotor.integralDecay);
    I2CSlave::setRegisterFloat(REGDS_STEER_CURRENT,steerMotor.getCurrent());
    I2CSlave::setRegisterFloat(REGDS_STEER_CALIBMIN,steerMotor.calibMin);
    I2CSlave::setRegisterFloat(REGDS_STEER_CALIBMAX,steerMotor.calibMax);
    I2CSlave::setRegisterFloat(REGDS_STEER_STALLCHECK,driveMotor.stallCheck);
    I2CSlave::setRegisterFloat(REGDS_STEER_DEADZONE,steerMotor.deadZone);
    I2CSlave::setRegisterInt(REGDS_DRIVE_ODO,driveMotor.getOdometry());
    
#else
    I2CSlave::init(addr,registerTable_LL);
    I2CSlave::setRegisterFloat(REGLL_ONE_REQPOS,lift1.required);
    I2CSlave::setRegisterFloat(REGLL_ONE_PGAIN,lift1.pGain);
    I2CSlave::setRegisterFloat(REGLL_ONE_IGAIN,lift1.iGain);
    I2CSlave::setRegisterFloat(REGLL_ONE_DGAIN,lift1.dGain);
    I2CSlave::setRegisterFloat(REGLL_ONE_INTEGRALCAP,lift1.integralCap);
    I2CSlave::setRegisterFloat(REGLL_ONE_INTEGRALDECAY,lift1.integralDecay);
    I2CSlave::setRegisterFloat(REGLL_ONE_CURRENT,lift1.getCurrent());
    I2CSlave::setRegisterFloat(REGLL_ONE_CALIBMIN,lift1.calibMin);
    I2CSlave::setRegisterFloat(REGLL_ONE_CALIBMAX,lift1.calibMax);
    I2CSlave::setRegisterFloat(REGLL_ONE_STALLCHECK,lift1.stallCheck);
    I2CSlave::setRegisterFloat(REGLL_ONE_DEADZONE,lift1.deadZone);
    
    I2CSlave::setRegisterFloat(REGLL_TWO_REQPOS,lift2.required);
    I2CSlave::setRegisterFloat(REGLL_TWO_PGAIN,lift2.pGain);
    I2CSlave::setRegisterFloat(REGLL_TWO_IGAIN,lift2.iGain);
    I2CSlave::setRegisterFloat(REGLL_TWO_DGAIN,lift2.dGain);
    I2CSlave::setRegisterFloat(REGLL_TWO_INTEGRALCAP,lift2.integralCap);
    I2CSlave::setRegisterFloat(REGLL_TWO_INTEGRALDECAY,lift2.integralDecay);
    I2CSlave::setRegisterFloat(REGLL_TWO_CURRENT,lift2.getCurrent());
    I2CSlave::setRegisterFloat(REGLL_TWO_CALIBMIN,lift2.calibMin);
    I2CSlave::setRegisterFloat(REGLL_TWO_CALIBMAX,lift2.calibMax);
    I2CSlave::setRegisterFloat(REGLL_TWO_STALLCHECK,lift2.stallCheck);
    I2CSlave::setRegisterFloat(REGLL_TWO_DEADZONE,lift2.deadZone);
#endif    
    I2CSlave::addListener(&listener);
}

void I2CLoop(){
    static unsigned long q = 0;
    uint16_t status;
          
    // read I2C data and handle register changes
    I2CSlave::poll();
    
#if DRIVESTEER
    // update all motors
    driveMotor.update();
    steerMotor.update();
#else
    lift1.update();
    lift2.update();
#endif
    wdt_reset(); // reset the watchdog
    
#if DRIVESTEER    
    status = ST_DS;
    // drive motor monitoring
    I2CSlave::setRegisterFloat(REGDS_DRIVE_CURRENT,driveMotor.getCurrent());
    I2CSlave::setRegisterFloat(REGDS_DRIVE_ACTUALSPEED,driveMotor.actual);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_ERROR,driveMotor.error);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_CONTROL,driveMotor.control);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_INTERVALCTRL,driveMotor.milliInterval);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_ERRORINTEGRAL,driveMotor.errorIntegral);
    I2CSlave::setRegisterFloat(REGDS_DRIVE_ERRORDERIV,driveMotor.errorDerivative);
    I2CSlave::setRegisterInt(REGDS_DRIVE_ODO,driveMotor.getOdometry());
    
    I2CSlave::setRegisterInt(REG_DEBUG,0);
    
    // steer motor monitoring
    I2CSlave::setRegisterFloat(REGDS_STEER_CURRENT,steerMotor.getCurrent());
    I2CSlave::setRegisterFloat(REGDS_STEER_ACTUALPOS,steerMotor.actual);
    I2CSlave::setRegisterFloat(REGDS_STEER_ERROR,steerMotor.error);
    I2CSlave::setRegisterFloat(REGDS_STEER_CONTROL,steerMotor.control);
    I2CSlave::setRegisterFloat(REGDS_STEER_INTERVALCTRL,steerMotor.milliInterval);
    I2CSlave::setRegisterFloat(REGDS_STEER_ERRORINTEGRAL,steerMotor.errorIntegral);
    I2CSlave::setRegisterFloat(REGDS_STEER_ERRORDERIV,steerMotor.errorDerivative);
    
    I2CSlave::setRegisterFloat(REGDS_CHASSIS,chassis.pos);
#else
    status = ST_LL;
    I2CSlave::setRegisterFloat(REGLL_ONE_CURRENT,lift1.getCurrent());
    I2CSlave::setRegisterFloat(REGLL_ONE_ACTUALPOS,lift1.actual);
    I2CSlave::setRegisterFloat(REGLL_ONE_ERROR,lift1.error);
    I2CSlave::setRegisterFloat(REGLL_ONE_CONTROL,lift1.control);
    I2CSlave::setRegisterFloat(REGLL_ONE_INTERVALCTRL,lift1.milliInterval);
    I2CSlave::setRegisterInt(REG_DEBUG,lift1.milliInterval);
    I2CSlave::setRegisterFloat(REGLL_ONE_ERRORINTEGRAL,lift1.errorIntegral);
    I2CSlave::setRegisterFloat(REGLL_ONE_ERRORDERIV,lift1.errorDerivative);
    I2CSlave::setRegisterFloat(REGLL_TWO_CURRENT,lift2.getCurrent());
    I2CSlave::setRegisterFloat(REGLL_TWO_ACTUALPOS,lift2.actual);
    I2CSlave::setRegisterFloat(REGLL_TWO_ERROR,lift2.error);
    I2CSlave::setRegisterFloat(REGLL_TWO_CONTROL,lift2.control);
    I2CSlave::setRegisterFloat(REGLL_TWO_INTERVALCTRL,lift2.milliInterval);
    I2CSlave::setRegisterFloat(REGLL_TWO_ERRORINTEGRAL,lift2.errorIntegral);
    I2CSlave::setRegisterFloat(REGLL_TWO_ERRORDERIV,lift2.errorDerivative);
#endif
    
    // if there is an exception, say so
    if(state.exception)
        status |= ST_EXCEPTION;
    
    // general registers
    I2CSlave::setRegisterInt(REG_STATUS,status);
    I2CSlave::setRegisterInt(REG_TIMER,millis());
    
    // motorID of exception is 5 if there is no motor associated
    // with the exception
    int motorid = state.exMotor?state.exMotor->getID() : 5;
    I2CSlave::setRegisterInt(REG_EXCEPTIONDATA,
                             (motorid<<8)|
                             state.exType);
    
    unsigned long t = micros();
    if(t>q)
        I2CSlave::setRegisterInt(REG_INTERVALI2C,t-q);
    q=t;
}
