
/* Simple program:  Create a blank window, wait for keypress, quit.

   Please see the SDL documentation for details on using the SDL API:
   /Developer/Documentation/SDL/docs.html
*/
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "SDL.h"

#include "SkApplication.h"
#include "SkWindow.h"

int main(int argc, char *argv[]) {
	Uint32 initflags = SDL_INIT_VIDEO | SDL_INIT_TIMER;
	SDL_Surface *screen;
	Uint8  video_bpp = 0;
	Uint32 videoflags = SDL_SWSURFACE;
    SDL_Event event;

	/* Initialize the SDL library */
	if ( SDL_Init(initflags) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",
			SDL_GetError());
		exit(1);
	}

	/* Set 640x480 video mode */
	screen=SDL_SetVideoMode(640,480, video_bpp, videoflags);
        if (screen == NULL) {
		fprintf(stderr, "Couldn't set 640x480x%d video mode: %s\n",
                        video_bpp, SDL_GetError());
		SDL_Quit();
		exit(2);
	}
    
    application_init();
    SkOSWindow* skwin = create_sk_window(screen);

	for (;;) {
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                goto QUIT;
            default:
                skwin->handleSDLEvent(event);
                break;
        }
    }
	
QUIT:
    application_term();
	/* Clean up the SDL library */
	SDL_Quit();
	return(0);
}
