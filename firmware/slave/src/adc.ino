/**
 * \file ADC for current sensing
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include "motor.h"
#include "adc.h"

void ADCReader::init(){
    adcstate = ADC_WAITING;
    curRead = 0;
    
    ADMUX = reads[curRead].channel;
    
    // enable the ADC, autotriggering, not on interrupts
    // (we poll instead; we're using too many interrupts)
    ADCSRA = (1<<ADEN)| // enable ADC
          (0<<ADATE)| // not free-run
          (0<<ADIE)| // no interrupts
          (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); // prescaler=7 (irrrelevant, though)
    
    ADCSRA |= 1<<ADSC; // start first conversion
}


void ADCReader::poll(){
    switch(adcstate){
    case ADC_WAITING: // waiting for ADC completion
        if(ADCSRA & (1<<ADIF)){
            // complete!
            adc_ct++; // increment the tick
            int reading;
    
            // get the reading
            reading = ADCL;
            reading |= (ADCH<<8);
            
            // handle appropriately
            reads[curRead].listener->onADC(reads[curRead].type,reading);
            
            // update mux 
            curRead = (curRead+1) % readCt;
            
            // set up mux for next read
            ADMUX = reads[curRead].channel;
            
            ADCSRA &= ~(1<<ADIF); // clear finished flag
            
            // now wait for the countdown until next read
            adcCountdown = adcInterval;
            adcstate = ADC_COUNTDOWN;
        }
        break;
    case ADC_COUNTDOWN:
        if(!--adcCountdown){
            ADCSRA |= (1<<ADSC);	//Start the next conversion
            adcstate = ADC_WAITING;
        }
        break;
    }
}    
    
