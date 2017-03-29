/**
 * @file
 * Abstract class for all simulated motors.
 */

#ifndef __MOTORSIM_H
#define __MOTORSIM_H


/// this class runs all simulated motors - it's extremely simple,
/// using Brown smoothing (aka exponential weighted moving average,
/// or a IIR (RC) filter.)

class MotorSim {
    /// the actual motor speed or position
    float actual;
    
    /// the smoothing factor for this motor
    float smoothing;
    
    /// the simulated current flow
    float current;

public:
    /// the required motor speed or position
    float required;
    
    float getActual(){
        return actual;
    }
    
    /// pass in a smoothing factor - the higher this is, 
    /// the more of the input is used so the quicker the
    /// motor responds. Between 0 and 1.
    MotorSim(float _smoothing){
        smoothing = _smoothing;
        actual=0;
        required=0;
        current=0;
    }
    
    void update(){
        actual = required*smoothing + actual*(1.0f-smoothing);
        float c = fabs(required*RoverSimulator::getSimCurrentFactor());
        // and cap it
        c = (1-powf(1.02f,-c))*50.0f;
        
        // make this work with hysteresis
        current = 0.1f*c + 0.9f*current;
        
    }
    
    /// set the simulated current flow
    virtual float getSimCurrent(){
        return current;
    }
        
};



#endif /* __MOTORSIM_H */
