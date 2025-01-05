#define _CRT_SECURE_NO_WARNINGS
#include "functions.h"
#include "Vector.h"

struct Snake {
	SDL_Surface * headUp , * headDown , * headLeft , * headRight;
	SDL_Surface * bodyVertical, *bodyHorizontal;
	SDL_Surface * tailUp , * tailDown , * tailLeft , * tailRight;
	SDL_Surface * turnLeftUp , * turnLeftDown , * turnRightUp , * turnRightDown;
	SDL_Surface * CurrentHead , * CurrentTail, *CurrentBody;
	SDL_Surface * tailAfterTurn , * bodyAfterTurn, *bodyForTurning;
	Vector<int>x , y;
	int bodySize = 3;
	Vector<int>directionX , directionY;
	Vector<bool>turn;
	int halfOfWidth = 0;
	int pictureWidth = 0;
	bool hitWall = false;
	bool turning = false;
	int turningX = 0;
	int turningY = 0;
	int snakeDelay = 8;
	Vector<Turn>turns;
	bool dead = false;
	int turnsTOdo = 0;
};

struct Game {
	int SCREEN_WIDTH = 480;
	int SCREEN_HEIGHT = 540;
	int RECTANGLE_HEIGHT = 60;
	int lastSnakeMove = 0;
	int startTime = 0;
	int endTime = 0;
	int score = 0;
	int frames = 0;
	int quit = 0;
	int gameInitialize = 0;
	Uint32 black , red , blue, grey;
	int points = 0;
	double deltaTime = 0;
	double worldTime = 0;
	double fpsTimer = 0;
	double fps = 0;
	bool restart = true;
	bool endGame = false;
	SDL_Event event;
	SDL_Surface * screen , * charset;
	SDL_Texture * scrtex;
	SDL_Window * window;
	SDL_Renderer * renderer;
};

struct BlueFood {
	SDL_Surface * picture;
	bool eaten = false;
	int number = 0;
	int x , y;
};

struct RedFood {
	SDL_Surface * picture;
	bool eaten = false;
	int number = 0;
	int x , y;
};

void printBoard ( Game & game) {
	SDLUtils::DrawRectangle ( game.screen , 0 , game.RECTANGLE_HEIGHT , game.SCREEN_WIDTH , game.SCREEN_HEIGHT - game.RECTANGLE_HEIGHT , game.red , game.black );
}

void printInfo (Game&game) {
	char text[128];
	SDLUtils::DrawRectangle ( game.screen , 0 , 0 , game.SCREEN_WIDTH , game.RECTANGLE_HEIGHT , game.red , game.blue );
	sprintf ( text , "time = %.1lf s  implemented points: 1,2,3,4" , game.worldTime);
	SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 10 , text , game.charset );
	sprintf ( text , "Esc - exit, n - new game, \x18 up, \x1B left, \x1A down, \x19 right" );
	SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 26 , text , game.charset );
	sprintf ( text , "%.0lf fps  points - %d", game.fps , game.points );
	SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 42 , text , game.charset );
}

void setSnake ( Snake & snake, Game & game ) {
	snake.CurrentBody = snake.bodyVertical;
	snake.CurrentHead = snake.headUp;
	snake.CurrentTail = snake.tailUp;
	for ( int i = 0; i < snake.bodySize; i++ ) {
		snake.x.push ( game.SCREEN_WIDTH / 2 + snake.pictureWidth);
		snake.y.push ( game.SCREEN_HEIGHT / 2 + i*snake.pictureWidth );
		snake.directionX.push ( 0 );
		snake.directionY.push ( -1 );
		snake.turn.push ( false );
	}

}

bool isInTurningArea ( Snake & snake, int index ) {
	int turningX1 = snake.turns.get(0).x - snake.halfOfWidth;
	int turningX2 = snake.turns.get ( 0 ).x + snake.halfOfWidth;
	int turningY1 = snake.turns.get ( 0 ).y - snake.halfOfWidth;
	int turningY2 = snake.turns.get ( 0 ).y + snake.halfOfWidth;
	
	if ( snake.x.get ( index ) >= turningX1 && snake.x.get ( index ) <= turningX2 &&
		 snake.y.get ( index ) >= turningY1 && snake.y.get ( index ) <= turningY2 ) {
		//printf ( "snake i: %d, x = %d, snakey = %d, turningY1 = %d, turningY2 = %d\n" ,index , snake.x.get ( index ) , snake.y.get ( index ) , turningY1 , turningY2 );
		return true;
	}
	return false;
}

void changeSkin ( Snake & snake ) {
	if ( snake.directionX.get(0) == 1 && snake.directionY.get(0) == 0 ) {
		snake.CurrentHead = snake.headRight;
		snake.CurrentTail = snake.tailRight;
		snake.CurrentBody = snake.bodyHorizontal;
		if ( snake.turns.get(0).pastDY == 1 )
			snake.bodyForTurning = snake.turnRightUp;
		else
			snake.bodyForTurning = snake.turnLeftDown;
	}
	else if ( snake.directionX.get(0) == -1 && snake.directionY.get(0) == 0 ) {
		snake.CurrentHead = snake.headLeft;
		snake.CurrentTail = snake.tailLeft;
		snake.CurrentBody = snake.bodyHorizontal;
		if ( snake.turns.get(0).pastDY == 1 )
			snake.bodyForTurning = snake.turnLeftUp;
		else
			snake.bodyForTurning = snake.turnRightDown;
	}
	else if ( snake.directionX.get(0) == 0 && snake.directionY.get(0) == 1 ) {
		snake.CurrentHead = snake.headDown;
		snake.CurrentTail = snake.tailDown;
		snake.CurrentBody = snake.bodyVertical;
		if ( snake.turns.get(0).pastDX == 1 )
			snake.bodyForTurning = snake.turnRightDown;
		else
			snake.bodyForTurning = snake.turnLeftDown;
	}
	else if ( snake.directionX.get(0) == 0 && snake.directionY.get (0) == -1 ) {
		snake.CurrentHead = snake.headUp;
		snake.CurrentTail = snake.tailUp;
		snake.CurrentBody = snake.bodyVertical;
		if ( snake.turns.get(0).pastDX == 1 )
			snake.bodyForTurning = snake.turnLeftUp;
		else
			snake.bodyForTurning = snake.turnRightUp;
	}
}

bool ifDirSame ( Snake & snake , int dir ) {
	if ( snake.directionX.get ( 0 ) == 1 && dir == 3 )
		return true;
	else if ( snake.directionX.get ( 0 ) == -1 && dir == 2 )
		return true;
	else if ( snake.directionY.get ( 0 ) == 1 && dir == 1 )
		return true;
	else if ( snake.directionY.get ( 0 ) == -1 && dir == 0 )
		return true;
	return false;
}

bool canTurn ( Snake & snake , int dir ) { // dir=0 - up, 1 - down, 2 - left, 3 - right
	int distanceX = snake.x.get ( 0 ) - snake.x.get ( 1 );
	int distanceY = snake.y.get ( 0 ) - snake.y.get ( 1 );
	if(distanceX < 0)
		distanceX *= -1;
	if ( distanceY < 0 )
		distanceY *= -1;
	if ( ifDirSame ( snake , dir ) )
		return false;
	else {
		if ( dir == 0 || dir == 1 ) {
			if ( distanceX <= snake.pictureWidth && snake.turnsTOdo > 0 )
				return false;
		}
		else if ( dir == 2 || dir == 3 ) {
			if ( distanceY <= snake.pictureWidth && snake.turnsTOdo > 0 )
				return false;
		}
	}
	snake.turnsTOdo++;
	return true;
}

void turnSnake ( Snake & snake , Game & game ) {
	if ( snake.turning || snake.hitWall ) {
		changeSkin ( snake );
		for ( int i = 1; i < snake.bodySize; i++ ) {
			if (snake.turns.getCurrentSize() != 0 && 
				snake.x.get ( i ) == snake.turns.get(0).x && snake.y.get (i) == snake.turns.get(0).y ) {
				snake.directionX.push ( snake.directionX.get ( i - 1 ) , i );
				snake.directionY.push ( snake.directionY.get ( i - 1 ) , i );
				snake.turn.push ( true , i );
				if ( i == snake.bodySize - 1 ) {
					snake.turns.pop ( 0 ); 
					snake.turning = false;
					snake.turnsTOdo--;
				}
			}
			else if ( isInTurningArea ( snake , i ) ) {
				snake.turn.push ( true , i );
			}
			else {
				snake.turn.push ( false , i );
			}
		}
		
	}
}

void changeDirection ( Snake & snake , Game & game, int dir ) {
	if(canTurn(snake, dir)){
		Turn newTurn = { snake.x.get ( 0 ),  snake.y.get ( 0 ), snake.directionX.get ( 0 ), snake.directionY.get ( 0 ) };
		snake.turns.push ( newTurn );
		
		switch ( dir ) {
			case 0:
				if ( snake.directionY.get ( 0 ) != 1 && snake.directionY.get ( 0 ) != -1 ) {
					snake.directionX.push ( 0 , 0 );
					snake.directionY.push ( -1 , 0 );
					snake.turning = true;
				}
				break;
			case 1:
				if ( snake.directionY.get ( 0 ) != -1 && snake.directionY.get ( 0 ) != 1 ) {
					snake.directionX.push ( 0 , 0 );
					snake.directionY.push ( 1 , 0 );
					snake.turning = true;
				}
				break;
			case 2:
				if ( snake.directionX.get ( 0 ) != 1 && snake.directionX.get ( 0 ) != -1 ) {
					snake.directionX.push ( -1 , 0 );
					snake.directionY.push ( 0 , 0 );
					snake.turning = true;
				}
				break;
			case 3:
				if ( snake.directionX.get ( 0 ) != -1 && snake.directionX.get ( 0 ) != 1 ) {
					snake.directionX.push ( 1 , 0 );
					snake.directionY.push ( 0 , 0 );
					snake.turning = true;
				}
				break;
		}
	}
}

void  getInput( Snake & snake , Game&game ) {
	while ( SDL_PollEvent ( &game.event ) ) {
		switch ( game.event.type ) {
			case SDL_KEYDOWN:
				switch ( game.event.key.keysym.sym ) {
					case SDLK_UP:
						changeDirection ( snake , game , 0 );
						break;
					case SDLK_DOWN:
						changeDirection ( snake , game , 1 );
						break;
					case SDLK_LEFT:
						changeDirection ( snake , game , 2 );
						break;
					case SDLK_RIGHT:
						changeDirection ( snake , game , 3 );
						break;
					case SDLK_n:
						game.endGame = true;
						break;
					case SDLK_ESCAPE:
						game.quit = 1;
						game.restart = false;
						break;
				}
				break;
		};
	};
}

int calculateDistanceRightWall ( Snake & snake , Game & game ) { 
	int currentX = snake.x.get ( 0 );
	int currentY = snake.y.get ( 0 );
	int distance = 0;
	if ( snake.directionX.get(0) == 1 ) {
		distance = ( game.SCREEN_HEIGHT - currentY );
	}
	else if ( snake.directionX.get(0) == -1 ) {
		distance = currentY - game.RECTANGLE_HEIGHT;
	}
	else if ( snake.directionY.get(0) == 1 ) {
		distance = currentX;
	}
	else if ( snake.directionY.get(0) == -1 ) {
		distance = game.SCREEN_WIDTH - currentX;
	}
	return distance;
}

void printFinalInfo (Game&game) {
	char text[128];
	SDLUtils::DrawRectangle ( game.screen , 0 , game.RECTANGLE_HEIGHT , game.SCREEN_WIDTH , game.SCREEN_HEIGHT - game.RECTANGLE_HEIGHT , game.blue , game.red );
	sprintf ( text , "you have survived %.1lf s" , game.worldTime );
	SDLUtils::DrawString ( game.screen , game.SCREEN_WIDTH / 2 - strlen ( text ) * 8 / 2 , game.SCREEN_HEIGHT / 2 , text , game.charset );
	sprintf ( text , "Do you want to play again?" );
	SDLUtils::DrawString ( game.screen , game.SCREEN_WIDTH / 2 - strlen ( text ) * 8 / 2 , game.SCREEN_HEIGHT / 2 + 10 , text , game.charset );
	sprintf ( text , "press n for new game or ESC to close game!" );
	SDLUtils::DrawString ( game.screen , game.SCREEN_WIDTH / 2 - strlen ( text ) * 8 / 2 , game.SCREEN_HEIGHT / 2 + 20 , text , game.charset );
	SDL_UpdateTexture ( game.scrtex , NULL , game.screen->pixels , game.screen->pitch );
	SDL_RenderCopy ( game.renderer , game.scrtex , NULL , NULL );
	SDL_RenderPresent ( game.renderer );
}

void ifHitItself ( Snake & snake , Game & game ) {
	for ( int i = 1; i < snake.bodySize; i++ ) {
		int snakeHeadX1 = snake.x.get ( 0 ) - snake.halfOfWidth;
		int snakeHeadX2 = snake.x.get ( 0 ) + snake.halfOfWidth;
		int snakeHeadY1 = snake.y.get ( 0 ) - snake.halfOfWidth;
		int snakeHeadY2 = snake.y.get ( 0 ) + snake.halfOfWidth;
		int snakeBodyX1 = snake.x.get ( i ) - snake.halfOfWidth;
		int snakeBodyX2 = snake.x.get ( i ) + snake.halfOfWidth;
		int snakeBodyY1 = snake.y.get ( i ) - snake.halfOfWidth;
		int snakeBodyY2 = snake.y.get ( i ) + snake.halfOfWidth;
		if ( ( snakeHeadX1 == snakeBodyX2 && snake.directionX.get(0) == -1) || 
			 ( snakeHeadX2 == snakeBodyX1 && snake.directionX.get(0) == 1) ||
			 ( snakeHeadY1 == snakeBodyY2 && snake.directionY.get(0) == -1 ) ||
			 ( snakeHeadY2 == snakeBodyY1  && snake.directionY.get(0) == 1 )) {
			while( !game.quit && !game.endGame ){
				snake.dead = true;
				printFinalInfo ( game );
				getInput ( snake , game );
			}
		}
	}
}

void ifHitWall ( Snake & snake , Game & game ) {
	if ( snake.hitWall ) {
		snake.hitWall = false;
		int distance = calculateDistanceRightWall ( snake , game );
		Turn newTurn = { snake.x.get ( 0 ) , snake.y.get ( 0 ), snake.directionX.get(0), snake.directionY.get(0)};
		snake.turns.push ( newTurn );
		if(distance >= snake.pictureWidth){
			if ( snake.directionX.get ( 0 ) == 1 ) {
				snake.directionX.push(0,0);
				snake.directionY.push(1,0);
			}
			else if ( snake.directionX.get ( 0 ) == -1 ) {
				snake.directionX.push ( 0 , 0 );
				snake.directionY.push ( -1 , 0 );
			}
			else if ( snake.directionY.get ( 0 ) == 1 ) {
				snake.directionX.push ( -1 , 0 );
				snake.directionY.push ( 0 , 0 );
			}
			else if ( snake.directionY.get ( 0 ) == -1 ) {
				snake.directionX.push ( 1 , 0 );
				snake.directionY.push ( 0 , 0 );
			}
		}
		else {
			if ( snake.directionX.get(0) == 1 ) {
				snake.directionX.push ( 0 , 0 );
				snake.directionY.push ( -1 , 0 );
			}
			else if ( snake.directionX.get(0) == -1 ) {
				snake.directionX.push ( 0 , 0 );
				snake.directionY.push ( 1 , 0 );
			}
			else if ( snake.directionY.get(0) == 1 ) {
				snake.directionX.push ( 1 , 0 );
				snake.directionY.push ( 0 , 0 );
			}
			else if ( snake.directionY.get(0) == -1 ) {
				snake.directionX.push ( -1 , 0 );
				snake.directionY.push ( 0 , 0 );
			}
		}
		snake.turning = true;
	}
}

void moveSnake ( Snake & snake , Game & game ) {
	int deltaSnakeMove = ( SDL_GetTicks () - game.lastSnakeMove ); //ms
	if(deltaSnakeMove >= snake.snakeDelay && !snake.dead){
		snake.x.push ( snake.x.get ( 0 ) + snake.directionX.get(0) , 0);
		snake.y.push ( snake.y.get ( 0 ) + snake.directionY.get(0) , 0);
		for ( int i = snake.bodySize - 1; i > 0; i-- ) {
			int moveX = snake.x.get ( i ) + snake.directionX.get(i);
			int moveY = snake.y.get ( i ) + snake.directionY.get(i);
			snake.x.push ( moveX , i );
			snake.y.push ( moveY , i );
		}
		game.lastSnakeMove = SDL_GetTicks ();
	}
}

void printSnake ( Game & game , Snake & snake ) {
	if(!snake.dead){
		for ( int i = 0; i < snake.bodySize; i++ ) {
			if ( i == 0 ) {
				SDLUtils::DrawSurface ( game.screen , snake.CurrentHead , snake.x.get ( i ) , snake.y.get ( i ) );
			}
			else if ( i == snake.bodySize - 1 ) {
				SDLUtils::DrawSurface ( game.screen , snake.CurrentTail , snake.x.get ( i ) , snake.y.get ( i ) );
			}
			else {
				if ( snake.turn.get ( i ) )
					SDLUtils::DrawSurface ( game.screen , snake.bodyForTurning , snake.x.get ( i ) , snake.y.get ( i ) );
				else
					SDLUtils::DrawSurface ( game.screen , snake.CurrentBody , snake.x.get ( i ) , snake.y.get ( i ) );
			}
		}
	}
}

void colision ( Snake & snake, Game&game ) {
	int snakeFrontX = snake.x.get ( 0 ) + snake.directionX.get(0) * ( snake.CurrentHead->w / 2 );
	int snakeFrontY = snake.y.get ( 0 ) + snake.directionY.get(0) * ( snake.CurrentHead->h / 2 );
	if ( snakeFrontY <= game.RECTANGLE_HEIGHT || snakeFrontX > game.SCREEN_WIDTH || snakeFrontX < 0 || snakeFrontY > game.SCREEN_HEIGHT ) {
		snake.hitWall = true;
	}
}

void loop (Game&game, Snake&snake, BlueFood&blueFood, RedFood&redFood) {
	game.startTime = SDL_GetTicks ();
	setSnake ( snake , game );
	while ( !game.quit && !game.endGame ) {
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
		printBoard ( game);
		ifHitWall ( snake , game );
		moveSnake ( snake , game );
		getInput ( snake , game );
		turnSnake ( snake , game );
		printSnake ( game , snake );
		colision ( snake , game );
		ifHitItself ( snake , game );
		SDL_UpdateTexture ( game.scrtex , NULL , game.screen->pixels , game.screen->pitch );

		SDL_UpdateTexture ( game.scrtex , NULL , game.screen->pixels , game.screen->pitch );
//		SDL_RenderClear(renderer);
		SDL_RenderCopy ( game.renderer , game.scrtex , NULL , NULL );
		SDL_RenderPresent ( game.renderer );
		game.frames++;
	};
}

void settingColors (Game&game) {
	game.black = SDL_MapRGB ( game.screen->format , 0x00 , 0x00 , 0x00 );
	game.red = SDL_MapRGB ( game.screen->format , 0xFF , 0x00 , 0x00 );
	game.blue = SDL_MapRGB ( game.screen->format , 0x11 , 0x11 , 0xCC );
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
	snake.halfOfWidth = snake.headUp->w/2;
	snake.pictureWidth = snake.headUp->w;
}

void loadFoodCharset (Game&game, BlueFood&bluefood, RedFood&redFood) {
	game.charset = SDL_LoadBMP ( "./cs8x8.bmp" );
	bluefood.picture = SDL_LoadBMP ( "./snakebmp/blueFood.bmp" );
	redFood.picture = SDL_LoadBMP ( "./snakebmp/redFood.bmp" );
	if ( game.charset == NULL || bluefood.picture==NULL||redFood.picture == NULL) {
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

void gameplay () {
	bool restart = true;
	while(restart){
		Game game;
		Snake snake;
		RedFood redFood;
		BlueFood blueFood;
		if ( SDL_Init ( SDL_INIT_EVERYTHING ) != 0 ) {
			printf ( "SDL_Init error: %s\n" , SDL_GetError () );
			exit ( 0 );
		}
		initialize ( game );
		loadFoodCharset ( game , blueFood, redFood );
		loadSnake ( game , snake );
		settingColors ( game );
		loop ( game , snake, blueFood, redFood );
		game.endGame = false;
		restart = game.restart;
		SDL_FreeSurface ( game.charset );
		SDL_FreeSurface ( game.screen );
		SDL_DestroyTexture ( game.scrtex );
		SDL_DestroyRenderer ( game.renderer );
		SDL_DestroyWindow ( game.window );

		SDL_Quit ();
	}
}

int main( int argc , char ** argv )
{
	gameplay ();
	return 0;
}

