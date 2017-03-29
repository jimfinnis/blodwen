/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */
#include <signal.h>
#include <stdlib.h>

#include <stdio.h>

#include <SDL.h>
#include <SDL/SDL_image.h>

#include "sdl.h"

static SDL_Surface *screen=NULL;
#define NUMFILES 32
static SDL_Surface* files[NUMFILES];

void SDL::quit(){
    if(screen)
        SDL_Quit();
}

void SDL::init(){
    SDL_Init(SDL_INIT_VIDEO);
    for(int i=0;i<NUMFILES;i++)
        files[i]=NULL;
    
    screen = SDL_SetVideoMode(800,480,24,SDL_NOFRAME);
    if(!screen){
        throw SDLException("cannot set video mode");
    }
    
    SDL_Flip(screen);
    SDL_Flip(screen);
}

int SDL::loadFile(const char *name){
    if(!screen)return -1;
    int i;
    for(i=0;i<NUMFILES;i++)
        if(!files[i])break;
    if(i==NUMFILES)
        throw SDLException("too many files");
    
    SDL_Surface *tmp = IMG_Load(name);
    if(!tmp)
        throw SDLException().set("cannot load %s",name);
    
    SDL_Surface *img = SDL_DisplayFormat(tmp);
    SDL_FreeSurface(tmp);
    files[i]=img;
    return i;
}

void SDL::displayFile(int n){
    if(!screen)return;
    if(files[n]<0)
        throw SDLException().set("invalid file %d",n);
    
    SDL_Surface *img = files[n];
    SDL_Rect src,dst;
    src.x=src.y=0;
    src.w = img->w;
    src.h = img->h;
    dst.x=dst.y=0;
    dst.w=dst.h=0; // ignored, apparently
    SDL_BlitSurface(img,&src,screen,&dst);
}

void SDL::flip(){
    if(!screen)return;
    SDL_Flip(screen);
}

static uint8_t colr,colg,colb;
void SDL::col(int r,int g,int b){
    colr=r;colg=g;colb=b;
    
}

void SDL::box(int x,int y,int w,int h){
    if(!screen)return;
    SDL_LockSurface(screen);
    int bpp = screen->format->BytesPerPixel;
    for(int j=0;j<h;j++,y++){
        uint8_t *p = (uint8_t*)screen->pixels+y*screen->pitch+x*bpp;
        for(int i=0;i<w;i++){
            p[0] = colb;
            p[1] = colg;
            p[2] = colr;
            p+=bpp;
        }
    }
    SDL_UnlockSurface(screen);
}

