/**
 * \file
 * Code for the rover simulator, which simulates at the serial comms
 * level for development without a rover.
 *
 */


#ifndef __SIM_H
#define __SIM_H


/// run a simulator tick every 0.1 seconds
#define SIMTICKLENGTH 0.1f




/// the rover simulator is an implementation of the simulator interface.
class RoverSimulator : public Simulator {
public:
    RoverSimulator();
    ~RoverSimulator();
    
    /// set the factor by which the motor speed is multiplied to
    /// generate a current value. The default is 0.07.
    static void setSimCurrentFactor(float t){
        simCurrentFactor = t;
    }
    
    /// get the factor by which the motor speed is multiplied to
    /// generate a current value. The default is 0.07.
    static float getSimCurrentFactor(){
        return simCurrentFactor;
    }
    /// returns how many chars read, ct is max amount
    virtual int read(char *buf,int ct);
    /// return -ve on error (which should be never in a simulator)
    virtual int write(const char *s,int ct);
    
    /// run the sim
    virtual void update();
    
    /// process pending commands
    virtual void poll();
    
    
private:    
    
    static float simCurrentFactor;
    
    /// the simulated drive motors
    class MotorSim *drive[6];
    /// the simulated steer motors
    class MotorSim *steer[6];
    /// the simulated lift motors
    class MotorSim *lift[6];
    
    /// the time accumulator
    float timeSoFar;
    
    /// simulate the rover, where t is a time interval. This is
    /// added to an accumulator, and when it goes over a simtick,
    /// we run the simulation.
    void simulate(double t);
    /// simulate the motors for a single simtick
    void simMotors(double t);
    
};




#endif /* __SIM_H */
