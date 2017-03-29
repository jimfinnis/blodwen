/**
 * \file Slave I2C declarations
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#ifndef __SLAVE_I2C_H
#define __SLAVE_I2C_H

// by default, we don't have changeEventImmediate() events.
#define IMMEDIATE_PERMITTED 0


#include "../../common/regsauto.h"

/// a listener to inherit from, listening for
/// registers changing their values in response to
/// master messages.

class I2CSlaveListener {
public:
    I2CSlaveListener *next; //!< next listener in chain
    
    I2CSlaveListener(){
        next = NULL;
    }
    
    /// called when a change occurs, as a poll in the main loop
    virtual void changeEvent(int reg,uint16_t val){};
#if defined(IMMEDIATE_PERMITTED)
    /// called when a change occurs, immediately as part of the interrupt!
    /// Be VERY CAREFUL putting things in this in your subclasses
    virtual void changeEventImmediate(int reg,uint16_t val){};
#endif
};

/// I2C slave. Data received stored in registers (so
/// it can be read) and a notification method is called
/// *outside* the interrupt, when poll() is called (to
/// ensure nothing bad happens sequence-wise.)
/// This is a namespace, because it's a singleton without
/// a ctor.

namespace I2CSlave {
    
/// Resets registers and sets callbacks,
/// initialises Wire (unlike the master code, which
/// does not!)
    
void init(int addr,const Register *table);

/// periodically check for changes to registers
void poll();

/// add a listener which will get called from poll()
/// when things change
void addListener(I2CSlaveListener *l);

/// set a register - takes no notice of size or mapping!
void setRegisterInt(int n, uint16_t v);

/// set a register to a float value, mapping if required
void setRegisterFloat(int n,float v);
}

              
#endif /* __SLAVE_I2C_H */
