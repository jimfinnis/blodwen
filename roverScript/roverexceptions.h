/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __ROVEREXCEPTIONS_H
#define __ROVEREXCEPTIONS_H

using namespace angort;

// this is the rover exception type (new for later Angorts)
#define EX_ROVER "ex$rover"

/// an exception which is a subclass of Angort's Exception.
class RoverInvalidException : public Exception{
public:
    RoverInvalidException():Exception(EX_ROVER,"rover not connected"){}
};

/// another exception, used when 'throw' is called.
class ScriptException : public Exception {
public:
    ScriptException(const char *s):Exception(s){}
};


#endif /* __ROVEREXCEPTIONS_H */
