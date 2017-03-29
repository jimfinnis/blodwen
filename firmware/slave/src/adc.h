/**
 * \file 
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __ADC_H
#define __ADC_H

/// read data type for current data
#define TYPE_CURRENT 0
/// read data type for position data
#define TYPE_POSITION 1

#define DEFAULT_ADC_INTERVAL 10

/// this is the interface for classes which should get ADC readings
class ADCListener {
public:
    virtual void onADC(int type, int reading)=0;
};

/// ADC handler class - used to simplify the matter
/// of several different kinds of ADC strategy for
/// different board setups.

class ADCReader {
    
    /// an array of read actions, max of 8.
    struct Read {
        int channel; //!< which ADC to read
        int type; //!< the read type (so we can differentiate if a listener has multiple reads)
        ADCListener *listener; //!< where to send it
    } reads[8];
    
    /// number of successful ADC conversions for debugging
    int adc_ct;
    /// the number of read actions we have so far
    int readCt;
    /// the current read
    int curRead;
    /// ADC state - waiting for ADC to complete, counting down to next tick
    enum {ADC_WAITING, ADC_COUNTDOWN} adcstate;
    
    /// countdown between ticks
    int adcCountdown;
    /// number of polls between end of old read and 
    /// start of new one
    int adcInterval;
    
public:
    ADCReader(){
        readCt=0;
        adcInterval = DEFAULT_ADC_INTERVAL;
    }
    
    /// start the ADC process - INTERRUPTS MUST BE DISABLED
    void init();
    
    /// poll the ADC and handle data if ready
    void poll();
        
    
    /// set interval between ADC reads
    void setInterval(int i=DEFAULT_ADC_INTERVAL){
        adcInterval = i;
    }
    
    /// add a new ADC read - will read channel i and
    /// when it completes, will inform the given listener.
    /// The type field is essentially optional data - sometimes
    /// a listener has multiple reads.
    
    void addRead(int channel,int type, ADCListener *listener){
        reads[readCt].channel = channel;
        reads[readCt].type = type;
        reads[readCt++].listener = listener;
    }
};

#endif /* __ADC_H */
