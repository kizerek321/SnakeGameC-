#define _CRT_SECURE_NO_WARNINGS
#include "functions.h"

struct Snake {

};
void printInfo ( SDL_Surface * screen , SDL_Surface *charset, double worldTime, double fps, int czerwony , int niebieski, char* text ) {
	SDLUtils::DrawRectangle ( screen , 4 , 4 , SCREEN_WIDTH - 8 , 36 , czerwony , niebieski );
		//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
	sprintf ( text , "Szablon drugiego zadania, czas trwania = %.1lf s  %.0lf klatek / s" , worldTime , fps );
	SDLUtils::DrawString ( screen , screen->w / 2 - strlen ( text ) * 8 / 2 , 10 , text , charset );
	//	      "Esc - exit, \030 - faster, \031 - slower"
	sprintf ( text , "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie" );
	SDLUtils::DrawString ( screen , screen->w / 2 - strlen ( text ) * 8 / 2 , 26 , text , charset );
}

void loop () {

}

int main( int argc , char ** argv )
{
	int t1 , t2 , quit , frames , rc;
	double delta , worldTime , fpsTimer , fps , distance , etiSpeed;
	SDL_Event event;
	SDL_Surface * screen , * charset;
	SDL_Surface * monkey , * block , * black , * ladder;
	SDL_Texture * scrtex;
	SDL_Window * window;
	SDL_Renderer * renderer;

	if ( SDL_Init ( SDL_INIT_EVERYTHING ) != 0 ) {
		printf ( "SDL_Init error: %s\n" , SDL_GetError () );
		return 1;
	}

// tryb pełnoekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer ( SCREEN_WIDTH , SCREEN_HEIGHT , 0 ,
									   &window , &renderer );
	if ( rc != 0 ) {
		SDL_Quit ();
		printf ( "SDL_CreateWindowAndRenderer error: %s\n" , SDL_GetError () );
		return 1;
	};

	SDL_SetHint ( SDL_HINT_RENDER_SCALE_QUALITY , "linear" );
	SDL_RenderSetLogicalSize ( renderer , SCREEN_WIDTH , SCREEN_HEIGHT );
	SDL_SetRenderDrawColor ( renderer , 0 , 0 , 0 , 255 );

	SDL_SetWindowTitle ( window , "DONKEY KONG" );


	screen = SDL_CreateRGBSurface ( 0 , SCREEN_WIDTH , SCREEN_HEIGHT , 32 ,
									0x00FF0000 , 0x0000FF00 , 0x000000FF , 0xFF000000 );

	scrtex = SDL_CreateTexture ( renderer , SDL_PIXELFORMAT_ARGB8888 ,
								 SDL_TEXTUREACCESS_STREAMING ,
								 SCREEN_WIDTH , SCREEN_HEIGHT );


	  // wyłączenie widoczności kursora myszy
	SDL_ShowCursor ( SDL_DISABLE );

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP ( "./cs8x8.bmp" );
	if ( charset == NULL ) {
		printf ( "SDL_LoadBMP(cs8x8.bmp) error: %s\n" , SDL_GetError () );
		SDL_FreeSurface ( screen );
		SDL_DestroyTexture ( scrtex );
		SDL_DestroyWindow ( window );
		SDL_DestroyRenderer ( renderer );
		SDL_Quit ();
		return 1;
	};
	SDL_SetColorKey ( charset , true , 0x000000 );

	monkey = SDL_LoadBMP ( "./eti.bmp" );
	if ( monkey == NULL ) {
		printf ( "SDL_LoadBMP(eti.bmp) error: %s\n" , SDL_GetError () );
		SDL_FreeSurface ( charset );
		SDL_FreeSurface ( screen );
		SDL_DestroyTexture ( scrtex );
		SDL_DestroyWindow ( window );
		SDL_DestroyRenderer ( renderer );
		SDL_Quit ();
		return 1;
	};


	char text[128];
	int czarny = SDL_MapRGB ( screen->format , 0x00 , 0x00 , 0x00 );
	int zielony = SDL_MapRGB ( screen->format , 0x00 , 0xFF , 0x00 );
	int czerwony = SDL_MapRGB ( screen->format , 0xFF , 0x00 , 0x00 );
	int niebieski = SDL_MapRGB ( screen->format , 0x11 , 0x11 , 0xCC );

	t1 = SDL_GetTicks ();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	distance = 0;
	etiSpeed = 0;

	while ( !quit ) {
		t2 = SDL_GetTicks ();
		delta = ( t2 - t1 ) * 0.001;
		t1 = t2;

		worldTime += delta;

		distance += etiSpeed * delta;

		SDL_FillRect ( screen , NULL , czarny );

		SDLUtils::DrawSurface ( screen , monkey , SCREEN_WIDTH / 2 + distance , SCREEN_HEIGHT / 2 );

		fpsTimer += delta;
		if ( fpsTimer > 0.5 ) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};

	// tekst informacyjny / info text
		printInfo (screen, charset, worldTime, fps, czerwony, niebieski, text);
		SDL_UpdateTexture ( scrtex , NULL , screen->pixels , screen->pitch );
//		SDL_RenderClear(renderer);
		SDL_RenderCopy ( renderer , scrtex , NULL , NULL );
		SDL_RenderPresent ( renderer );

		// obsługa zdarzeń (o ile jakieś zaszły) / handling of events (if there were any)
		while ( SDL_PollEvent ( &event ) ) {
			switch ( event.type ) {
				case SDL_KEYDOWN:
					if ( event.key.keysym.sym == SDLK_ESCAPE ) quit = 1;
					else if ( event.key.keysym.sym == SDLK_RIGHT ) etiSpeed = 50;
					else if ( event.key.keysym.sym == SDLK_LEFT ) etiSpeed = -50;
					else if ( event.key.keysym.sym == SDLK_SPACE ) etiSpeed = -50;
					else if ( event.key.keysym.sym == SDLK_n ) etiSpeed = -50;
					break;
				case SDL_KEYUP:
					etiSpeed = 0;
					break;
				case SDL_QUIT:
					quit = 1;
					break;
			};
		};
		frames++;
	};

// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface ( charset );
	SDL_FreeSurface ( screen );
	SDL_DestroyTexture ( scrtex );
	SDL_DestroyRenderer ( renderer );
	SDL_DestroyWindow ( window );

	SDL_Quit ();
	return 0;
}

