/**
 * \file 
 * Handles exceptions which occur on the slaves.
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __STATE_H
#define __STATE_H

/// a listener for exceptions
class ExceptionListener {
    friend class State;
    ExceptionListener *next;
#if defined(MASTER)
    virtual void onException(char slave,char motor,char type){}
#else
    virtual void onException(class MotorController *,char type){}
#endif
    virtual void onExceptionReset(){}
};

/// an overcurrent exception
#define EX_OVERCURRENT 1
/// we've only just started up
#define EX_BOOT 2
/// another slave somewhere has had an exception, and the master
/// has told us
#define EX_REMOTESLAVE 3
/// we're possibly shorted: we're not moving, control is lowish, but current is high
#define EX_SHORTNC 4
/// we're not moving, current is high, control is high
#define EX_STALL 5
/// we're not moving, current is low but there, control is high
#define EX_ENCODERFAULT 6
/// we're not moving, current is zero, control is high
#define EX_DRIVEFAULT 7
/// like an overcurrent, but with low control
#define EX_SHORTOC 8

/// the slave's state - exceptions, etc. This file gets a little messy
/// because it behaves rather differently on slave and master (due to
/// how the device throwing the exception is identified differently)
/// but it should be easy to understand: on a slave, the throwing entity
/// is a motor controller identified by pointer; on the master, it's
/// a slave/motor number pair.
class State {
private:
    /// the disabled exceptions
    int disabledExceptions;
public:
    
    /// we start up in an exception
    State(){
        exception = true;
        exType = EX_BOOT;
#if defined(MASTER)
        exSlave = 0;
        exMotor = 0;
#else
        exMotor = NULL;
#endif
        exListenerHead=NULL;
        disabledExceptions = 0;
    }
    
    /// true if we are in the exception state
    bool exception;
    
#if defined(MASTER)
    int exSlave; //!< which slave
    int exMotor; //!< which motor on the slave
#else
    /// the motor causing the exception
    class MotorController *exMotor;
#endif
    /// the exception type
    char exType;
    
    /// the exceptions in the bit field passed in will be disabled
    /// on this processor
    void setDisabledExceptions(int b){
        disabledExceptions = b;
    }
        
    
    /// raise an exception - ID is the ID of the motor (0 or 1)
    /// and type is the exception type - note that you cannot raise
    /// an exception if you're already in the exception state..
#if defined(MASTER)
    void raiseException(char s, char m,char type){
        if(exception)return; // can't raise if already in state
        if((1<<type) & disabledExceptions)
            return;  // exception is disabled
        for(ExceptionListener *p=exListenerHead;p;p=p->next){
            p->onException(s,m,type);
        }
        exSlave=s;
#else
    void raiseException(class MotorController *m,char type){
        if(exception)return; // can't raise if already in state
        if((1<<type) & disabledExceptions)
            return;  // exception is disabled
        for(ExceptionListener *p=exListenerHead;p;p=p->next){
            p->onException(m,type);
        }
#endif
        exType = type;
        exMotor = m;
        exception = true; // only set this afterwards so we can check for existing states!
    }
    
    void addExceptionListener(ExceptionListener *p){
        p->next = exListenerHead;
        exListenerHead = p;
    }
    
    void reset(){
        for(ExceptionListener *p=exListenerHead;p;p=p->next)
            p->onExceptionReset();
        exception = false;
    }
    
    ExceptionListener *exListenerHead;
};


/// the state singleton
extern State state;

#endif /* __STATE_H */
