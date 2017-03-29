/**
 * \file Reads information from the eeprom, such as the I2C address
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "config.h"
#include "../../common/regsauto.h"

void readEEProm(){
    extern char i2c_addr;
    
    // just read the first 32 bytes of eeprom memory
    char buf[32];
    
    eeprom_read_block((void *)buf,(const void  *)0,32);
    
    
    
    // get the parameters and check them; if there's a problem, abort
    // and flash errors.
    
    i2c_addr = buf[0];
    if(i2c_addr<=0 || i2c_addr>=MAXDEVICES)
        goto error;
    
    
    return;
    
    // ERROR - some parameter or other was wrong.

error:
    for(;;){
        digitalWrite(M1LED,LOW);
        digitalWrite(M2LED,LOW);
        wdt_reset();
        delay(300);
        digitalWrite(M1LED,HIGH);
        wdt_reset();
        delay(300);
        digitalWrite(M1LED,LOW);
        digitalWrite(M2LED,HIGH);
        wdt_reset();
        delay(600);
    }
}
