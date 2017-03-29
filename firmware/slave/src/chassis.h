/**
 * \file 
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __CHASSIS_H
#define __CHASSIS_H

#include "adc.h"

/// listener for ADC events on the chassis pot
class Chassis : public ADCListener {
public:
    /// the last read position
    int pos;
    /// listener method called when ADC updates
    virtual void onADC(int type, int p){
        pos = p;
    }
};



#endif /* __CHASSIS_H */
