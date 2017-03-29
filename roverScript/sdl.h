/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __SDL_H
#define __SDL_H

#include "angort.h"

class SDLException : public Exception{
public:
    SDLException(const char *s):Exception(s){}
    SDLException():Exception(){}
};

/// using a class as a namespace
class SDL {
public:
    static void init();
    static void quit();
    static int loadFile(const char *name);
    static void displayFile(int n);
    static void flip();
    static void col(int r,int g,int b);
    static void box(int x,int y,int w,int h);
};

#endif /* __SDL_H */
