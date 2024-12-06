#define _CRT_SECURE_NO_WARNINGS
#include "functions.h"
#include "Vector.h"

struct Snake {
	SDL_Surface * headUp , * headDown , * headLeft , * headRight;
	SDL_Surface * bodyVertical, *bodyHorizontal;
	SDL_Surface * tailUp , * tailDown , * tailLeft , * tailRight;
	SDL_Surface * turnLeftUp , * turnLeftDown , * turnRightUp , * turnRightDown;
	SDL_Surface * CurrentHead , * CurrentTail, *CurrentBody;
	Vector<int>x , y;
	int bodySize = 3;
	int directionX = 0; // 1 - right, -1 - left
	int directionY = -1; // 1 - down, -1 - up
	int halfOfWidth = 12;
};

struct Game {
	int SCREEN_WIDTH = 480;
	int SCREEN_HEIGHT = 516;
	int RECTANGLE_HEIGHT = 36;
	int startTime = 0;
	int endTime = 0;
	int score = 0;
	int frames = 0;
	int quit = 0;
	int gameInitialize = 0;
	int black , red , blue, grey;
	double deltaTime = 0;
	double worldTime = 0;
	double fpsTimer = 0;
	double fps = 0;
	SDL_Event event;
	SDL_Surface * screen , * charset;
	SDL_Texture * scrtex;
	SDL_Window * window;
	SDL_Renderer * renderer;
};

struct Food {
	SDL_Surface * redFood , *blueFood;
};

struct Board {
	int BOARD_ROWS = 20;
	int BOARD_COLS = 20;
	int gridSize = 24;
	Vector<Vector<int>> area; // 0 -free , 1 - snake, 2 - food, 3 - wall
};

void printBoard ( Game & game, Board&board ) {
	SDLUtils::DrawRectangle ( game.screen , 0 , game.RECTANGLE_HEIGHT , game.SCREEN_WIDTH , game.SCREEN_HEIGHT - game.RECTANGLE_HEIGHT , game.red , game.black );
	for ( int row = 0; row <= board.BOARD_ROWS; row++ ) {
		SDLUtils::DrawLine ( game.screen , game.RECTANGLE_HEIGHT , game.RECTANGLE_HEIGHT + board.gridSize + row*board.gridSize , board.gridSize * board.BOARD_COLS , 1 , 0 , game.grey );
	}
	for ( int col = 0; col <= board.BOARD_COLS; col++ ) {
		SDLUtils::DrawLine ( game.screen , board.gridSize*col , game.RECTANGLE_HEIGHT , board.gridSize * board.BOARD_ROWS , 0 , 1 , game.grey );
	}
}

void printInfo (Game&game) {
	char text[128];
	SDLUtils::DrawRectangle ( game.screen , 0 , 0 , game.SCREEN_WIDTH , game.RECTANGLE_HEIGHT , game.red , game.blue );
	sprintf ( text , "time = %.1lf s  fps = %.0lf " , game.worldTime , game.fps );
	SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 10 , text , game.charset );
	sprintf ( text , "Esc - exit, w - up, a - left, d - right, s - down" );
	SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 26 , text , game.charset );
}

void setSnake ( Snake & snake,Game&game ) {
	snake.CurrentBody = snake.bodyVertical;
	snake.CurrentHead = snake.headUp;
	snake.CurrentTail = snake.tailUp;
	for ( int i = 0; i < snake.bodySize; i++ ) {
		snake.x.push ( game.SCREEN_WIDTH / 2 + snake.halfOfWidth);
		snake.y.push ( game.SCREEN_HEIGHT / 2 + i*snake.CurrentBody->h + snake.halfOfWidth );
	}
}

void moveSnake ( Snake & snake , Game & game ) {
	for ( int i = snake.bodySize - 1; i > 0; i-- ) {
		snake.x.push ( snake.x.get ( i - 1 ) );
		snake.y.push ( snake.y.get ( i - 1 ) );
	}
	snake.x.push ( snake.x.get ( 0 ) + snake.directionX * snake.CurrentHead->w );
	snake.y.push ( snake.y.get ( 0 ) + snake.directionY * snake.CurrentHead->h );
}

void printSnake ( Game & game , Snake & snake ) {
	for ( int i = 0; i < snake.bodySize; i++ ) {
		if ( i == 0 ) {
			SDLUtils::DrawSurface ( game.screen , snake.CurrentHead , snake.x.get(i) , snake.y.get(i) );
		}
		else if ( i == snake.bodySize - 1 ) {
			SDLUtils::DrawSurface ( game.screen , snake.CurrentTail , snake.x.get(i) , snake.y.get(i));
		}
		else {
			SDLUtils::DrawSurface ( game.screen , snake.CurrentBody , snake.x.get(i) , snake.y.get (i));
		}
	}
}

void loop (Game&game, Snake&snake, Board&board) {
	game.startTime = SDL_GetTicks ();
	setSnake ( snake , game );
	while ( !game.quit ) {
		game.endTime = SDL_GetTicks ();
		game.deltaTime = ( game.endTime - game.startTime ) * 0.001;
		game.startTime = game.endTime;

		game.worldTime += game.deltaTime;

		SDL_FillRect ( game.screen , NULL , game.black );


		game.fpsTimer += game.deltaTime;
		if ( game.fpsTimer > 0.5 ) {
			game.fps = game.frames * 2;
			game.frames = 0;
			game.fpsTimer -= 0.5;
		};
		printInfo ( game );
		printBoard ( game, board );
		printSnake ( game , snake );
		SDL_UpdateTexture ( game.scrtex , NULL , game.screen->pixels , game.screen->pitch );

		SDL_UpdateTexture ( game.scrtex , NULL , game.screen->pixels , game.screen->pitch );
//		SDL_RenderClear(renderer);
		SDL_RenderCopy ( game.renderer , game.scrtex , NULL , NULL );
		SDL_RenderPresent ( game.renderer );
		while ( SDL_PollEvent ( &game.event ) ) {
			switch ( game.event.type ) {
				case SDL_KEYDOWN:
					if ( game.event.key.keysym.sym == SDLK_ESCAPE ) game.quit = 1;
					break;
				case SDL_QUIT:
					game.quit = 1;
					break;
			};
		};
		game.frames++;
	};
}

void settingColors (Game&game) {
	game.black = SDL_MapRGB ( game.screen->format , 0x00 , 0x00 , 0x00 );
	game.red = SDL_MapRGB ( game.screen->format , 0xFF , 0x00 , 0x00 );
	game.blue = SDL_MapRGB ( game.screen->format , 0x11 , 0x11 , 0xCC );
	game.grey = SDL_MapRGB ( game.screen->format , 128 , 128 , 128 ); // Grey with RGB (128,128,128)
}

void loadSnake (Game&game, Snake&snake) {
	snake.headUp = SDL_LoadBMP ( "./snakebmp/headTop.bmp" );
	snake.headDown = SDL_LoadBMP ( "./snakebmp/headDown.bmp" );
	snake.headLeft = SDL_LoadBMP ( "./snakebmp/headLeft.bmp" );
	snake.headRight = SDL_LoadBMP ( "./snakebmp/snakeRight.bmp" );
	snake.tailUp = SDL_LoadBMP ( "./snakebmp/tailTop.bmp" );
	snake.tailDown = SDL_LoadBMP ( "./snakebmp/tailDown.bmp" );
	snake.tailLeft = SDL_LoadBMP ( "./snakebmp/tailLeft.bmp" );
	snake.tailRight = SDL_LoadBMP ( "./snakebmp/tailRight.bmp" );
	snake.bodyHorizontal = SDL_LoadBMP ( "./snakebmp/bodyHorizontally.bmp" );
	snake.bodyVertical = SDL_LoadBMP ( "./snakebmp/bodyVertical.bmp" );
	snake.turnLeftUp = SDL_LoadBMP ( "./snakebmp/turnLeftUp.bmp" );
	snake.turnLeftDown = SDL_LoadBMP ( "./snakebmp/turnLeftDown.bmp" );
	snake.turnRightUp = SDL_LoadBMP ( "./snakebmp/turnRightUp.bmp" );
	snake.turnRightDown = SDL_LoadBMP ( "./snakebmp/turnRightDown.bmp" );

	if ( snake.headUp == NULL || snake.headDown == NULL || snake.headLeft == NULL || snake.headRight == NULL||
		 snake.tailUp == NULL || snake.tailDown == NULL || snake.tailLeft == NULL || snake.tailRight == NULL||
		 snake.bodyHorizontal == NULL || snake.bodyVertical == NULL ||
		 snake.turnLeftDown == NULL || snake.turnLeftUp == NULL || snake.turnRightDown == NULL || snake.turnRightUp == NULL) {
		printf ( "loading snake error: %s\n" , SDL_GetError () );
		SDL_FreeSurface ( game.charset );
		SDL_FreeSurface ( game.screen );
		SDL_DestroyTexture ( game.scrtex );
		SDL_DestroyWindow ( game.window );
		SDL_DestroyRenderer ( game.renderer );
		SDL_Quit ();
		exit(0);
	};
}

void loadFoodCharset (Game&game, Food&food) {
	game.charset = SDL_LoadBMP ( "./cs8x8.bmp" );
	food.blueFood = SDL_LoadBMP ( "./snakebmp/blueFood.bmp" );
	food.redFood = SDL_LoadBMP ( "./snakebmp/redFood.bmp" );
	if ( game.charset == NULL || food.blueFood==NULL||food.redFood == NULL) {
		printf ( "charset or food loading error: %s\n" , SDL_GetError () );
		SDL_FreeSurface ( game.screen );
		SDL_DestroyTexture ( game.scrtex );
		SDL_DestroyWindow ( game.window );
		SDL_DestroyRenderer ( game.renderer );
		SDL_Quit ();
		exit(0);
	};
	SDL_SetColorKey ( game.charset , true , 0x000000 );
}

void initialize ( Game & game ) {
	game.gameInitialize = SDL_CreateWindowAndRenderer ( game.SCREEN_WIDTH , game.SCREEN_HEIGHT , 0 ,
														&game.window , &game.renderer );
	if ( game.gameInitialize != 0 ) {
		SDL_Quit ();
		printf ( "SDL_CreateWindowAndRenderer error: %s\n" , SDL_GetError () );
		exit ( 0 );
	};

	SDL_SetHint ( SDL_HINT_RENDER_SCALE_QUALITY , "linear" );
	SDL_RenderSetLogicalSize ( game.renderer , game.SCREEN_WIDTH , game.SCREEN_HEIGHT );
	SDL_SetRenderDrawColor ( game.renderer , 0 , 0 , 0 , 255 );
	SDL_SetWindowTitle ( game.window , "SNAKE BY KRZYSZTOF SZUDY s197771" );
	game.screen = SDL_CreateRGBSurface ( 0 , game.SCREEN_WIDTH , game.SCREEN_HEIGHT , 32 , 0x00FF0000 , 0x0000FF00 ,
										 0x000000FF , 0xFF000000 );
	game.scrtex = SDL_CreateTexture ( game.renderer , SDL_PIXELFORMAT_ARGB8888 , SDL_TEXTUREACCESS_STREAMING , 
									  game.SCREEN_WIDTH , game.SCREEN_HEIGHT );
	SDL_ShowCursor ( SDL_DISABLE );

}

int main( int argc , char ** argv )
{
	Game game;
	Snake snake;
	Food food;
	Board board;
	if ( SDL_Init ( SDL_INIT_EVERYTHING ) != 0 ) {
		printf ( "SDL_Init error: %s\n" , SDL_GetError () );
		return 1;
	}
	initialize ( game );
	loadFoodCharset ( game , food );
	loadSnake ( game , snake );
	settingColors ( game );
	loop ( game , snake, board );
	SDL_FreeSurface ( game.charset );
	SDL_FreeSurface ( game.screen );
	SDL_DestroyTexture ( game.scrtex );
	SDL_DestroyRenderer ( game.renderer );
	SDL_DestroyWindow ( game.window );

	SDL_Quit ();
	return 0;
}

