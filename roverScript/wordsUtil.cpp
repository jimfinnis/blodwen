/**
 * @file
 * Utility words
 * 
 */

#include "angort.h"
#include "../pc/rover.h"
#include "roverexceptions.h"

extern void setsigs(bool allowInterrupt);
extern Rover *r;
extern struct timespec progstart;
extern double time_diff(timespec start, timespec end);
extern unsigned long updateTickLength;

%name util

%word delay (n --) delay for n microseconds
{
    float t = a->popval()->toFloat();
    t *= 1.0e6;
    setsigs(true); // make sure we can interrupt during the delay
    usleep((useconds_t)t);
    setsigs(false);
}    


%word time (--time) get time since start of program in seconds
{
    struct timespec t;
    
    clock_gettime(CLOCK_MONOTONIC,&t);
    double diff=time_diff(progstart,t);
    Types::tFloat->set(a->pushval(),diff);
}


%word setupdatetick (time --) set length of update tick in microseconds
{
    unsigned long t = a->popval()->toFloat();
    updateTickLength = t;
}

%word panic (string --) throw a ScriptException and leave the program
{
    char buf[256];
    strncpy(buf,a->popval()->toString().get(),256);
    throw ScriptException(buf);
}

inline void asyncrun(const char *f,...){
    va_list args;
    va_start(args,f);
    char buf[400];
    vsnprintf(buf,256,f,args);
    
    if(!fork()){
        strcat(buf," >/dev/null 2>/dev/null");
        system(buf);
        exit(0);
    }
    va_end(args);
}

%word say (string --) use espeak to say something
{
    char buf[1024];
    strncpy(buf,a->popval()->toString().get(),1024);
    asyncrun("espeak %s",buf);
}

%word play (filename --) use aplay to play a sound file
{
    char buf[1024];
    strncpy(buf,a->popval()->toString().get(),1024);
    asyncrun("aplay %s",buf);
}
/*
%commentedword cleantex (--) delete the latex dump file
{
    unlink("listtex.tex");
}


%commentedword listtex (modulename --) dump all words of a module as LaTeX
{
    const char *q = strdup(a->popval()->toString().get());
    
    FILE *f = fopen("listtex.tex","a");
    fprintf(f,
            "\\textbf{Module: %s}\\vspace*{0.5em}\n\\\\\n"
            "\\begin{tabular}{|p{1in}|p{2in}|p{3in}|}\\hline \n",q);
    
    Namespace *m = a->findOrCreateModule(q);
    StringMapIterator<NativeFunc> iter(&m->funcs);
    for(iter.first();!iter.isDone();iter.next()){
        const char *name = iter.current()->key;
        char *spec = strdup(a->getSpec(name));
        
        char *brk = strchr(spec,')')+1;
        *brk++ = 0;
        fprintf(f,"%s & %s & %s \\\\\n",name,spec,brk);
        
        free(spec);
    }
    fprintf(f,"\\hline \\end{tabular}\n\\\\\n\\vspace*{1em}\\\\\n");
    fclose(f);
    free(q);
}    
*/
