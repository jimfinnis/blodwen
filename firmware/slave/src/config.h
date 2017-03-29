/**
 * \file Config data, including pin definitions in terms of Arduino
 * mappings for the  pins on the Sparkfun serial motor driver, and
 * comms configuration (serial/I2C)
 *
 * \author $Author$
 * \date $Date$
 * 
 */


#ifndef __CONFIG_H
#define __CONFIG_H

#include "hwconfig.h"
#include "liftorsteer.h"

// analog input pins for motor current reads

#define SENSE1	0
#define SENSE2	1

// overcurrent LED output pins (digital)

#define M1LED	A2
#define M2LED	A3

// motor output control pins

#define M1POS 2
#define M1NEG 3
#define M1PWM 9
#define M2PWM 10

#define M2POS 4
#define M2NEG 7

// encoder input pins for drive motors

#define ENCA 0
#define ENCB 1



#endif /* __CONFIG_H */
