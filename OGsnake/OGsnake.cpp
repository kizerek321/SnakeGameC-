#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "functions.h"
#include "Vector.h"

struct Snake {
    Vector<Cell> body;
    int directionX = 0;  // 1 = right, -1 = left, 0 = none
    int directionY = -1; // 1 = down, -1 = up, 0 = none
	double lastMoveTime = 0.0;
	double snakeDelay = 130.0;
	int bodySize = 3;
};

struct BlueFood {
    SDL_Surface * picture;
    bool eaten = false;
    int number = 0;
	Cell position;
};

struct RedFood {
    SDL_Surface * picture;
    bool eaten = true;
	Cell position;
    double spawnTime = 0.0;
    double spawnInterval = 5.0;
};

struct Game {
    int screenWidth = 640;
    int screenHeight = 540;
    int rectangleHeight = 60;
    int cellSize = 20;
    int rows , cols;
    int points = 0;
    bool gameOver = false;
	bool restart = true;
    bool userTurn = false;
    double deltaTime = 0;
    double worldTime = 0;
    double startTime = 0;
    double endTime = 0;
    double lastSpeedchange = 0;
    const double speedUpInterval = 5.0;
    const double speedUpValue = 0.01;
    Uint32 snakeColor , boardColor , backgroundColor;
    SDL_Surface * screen, *charset;
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Texture * texture;
	Uint32 red , blue, black;
};

void setColors ( Game & game ) {
    game.black = SDL_MapRGB ( game.screen->format , 0x00 , 0x00 , 0x00 );
    game.red = SDL_MapRGB ( game.screen->format , 0xFF , 0x00 , 0x00 );
    game.blue = SDL_MapRGB ( game.screen->format , 0x11 , 0x11 , 0xCC );
}

void printInfo ( Game & game, Snake&snake ) {
    char text[128];
    SDLUtils::DrawRectangle ( game.screen , 0 , 0 , game.screenWidth , game.rectangleHeight , game.red , game.blue );
    sprintf ( text , "time = %.1lf s  implemented points: 1,2,3,4" , game.worldTime );
    SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 10 , text , game.charset );
    sprintf ( text , "Esc - exit, n - new game, \x18 up, \x1B left, \x1A down, \x19 right" );
    SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 26 , text , game.charset );
    sprintf ( text , "points - %d body_size: %d" , game.points, snake.bodySize );
    SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 42 , text , game.charset );
}

void drawCell ( SDL_Surface * screen , int x , int y , int size , Uint32 color ) {
    SDLUtils::DrawRectangle ( screen , x * size , y * size , size , size , color , color );
}

void initGame ( Game & game ) {
    game.rows = game.screenHeight / game.cellSize;
    game.cols = game.screenWidth / game.cellSize;
    game.snakeColor = SDL_MapRGB ( game.screen->format , 0 , 255 , 0 );
    game.boardColor = SDL_MapRGB ( game.screen->format , 0 , 0 , 0 );
    game.backgroundColor = SDL_MapRGB ( game.screen->format , 50 , 50 , 50 );
}

void initSnake ( Snake & snake , Game & game ) {
	for ( int i = 0; i < snake.bodySize; i++ ) {
		snake.body.push ( { game.cols / 2 , game.rows / 2 + i + game.rectangleHeight / game.cellSize } );
	}
}

void updateSnake ( Snake & snake ) {
    for ( int i = snake.body.getCurrentSize () - 1; i > 0; --i ) {
        snake.body.push ( snake.body.get ( i - 1 ) , i );
    }
    snake.body.push ( { snake.body.get ( 0 ).x + snake.directionX, snake.body.get ( 0 ).y + snake.directionY } , 0 );
}

bool checkWallHit ( Snake & snake , Game & game ) {
	Cell head = snake.body.get ( 0 );
	if ( head.x + snake.directionX < 0 && snake.directionX == -1 )
        return true;
    if( head.x + snake.directionX >= game.cols && snake.directionX == 1)
        return true;
    if( head.y + snake.directionY <= 2 && snake.directionY == -1)
        return true;
    if( head.y + snake.directionY >= game.rows && snake.directionY == 1 )
        return true;
	return false;
}

bool checkCollision ( Snake & snake , Game & game ) {
    Cell head = snake.body.get ( 0 );
    for ( int i = 1; i < snake.body.getCurrentSize (); ++i ) {
        if ( snake.body.get ( i ).x == head.x && snake.body.get ( i ).y == head.y ) return true;
    }
    return false;
}

void hitWall ( Snake & snake, Game&game ) {
    if ( game.userTurn ) {
        game.userTurn = false;
        return;
    }
	Cell head = snake.body.get ( 0 );
    int rightDirectionX = -snake.directionY; // turn of 90 deegrees so x=-y, y=x
	int rightDirectionY = snake.directionX;
	int leftDX = snake.directionY;
    int leftDY = -snake.directionX;
	Cell rightCellCheck = { head.x + rightDirectionX, head.y + rightDirectionY };
    bool canTurnRight = ( rightCellCheck.x >= 0 && rightCellCheck.x < game.cols && rightCellCheck.y > 2 && rightCellCheck.y < game.rows );
    if ( canTurnRight ) {
		snake.directionX = rightDirectionX;
		snake.directionY = rightDirectionY;
    }
    else { // turn left
		snake.directionX = leftDX;
		snake.directionY = leftDY;
    }
}

void handleInput ( Snake & snake , Game & game ) {
    SDL_Event event;
    while ( SDL_PollEvent ( &event ) ) {
        if ( event.type == SDL_KEYDOWN ) {
            switch ( event.key.keysym.sym ) {
                case SDLK_UP: 
                    if ( snake.directionY == 0) { 
                        snake.directionX = 0; 
                        snake.directionY = -1; 
						game.userTurn = true;
                    } break;
                case SDLK_DOWN:
                    if ( snake.directionY == 0 ) { 
                        snake.directionX = 0; 
                        snake.directionY = 1; 
                        game.userTurn = true;
                    } break;
                case SDLK_LEFT: 
                    if ( snake.directionX == 0 ) { 
                        snake.directionX = -1; 
                        snake.directionY = 0;
                        game.userTurn = true;
                    } break;
                case SDLK_RIGHT: 
                    if ( snake.directionX == 0 ) { 
                        snake.directionX = 1; 
                        snake.directionY = 0;
                        game.userTurn = true;
                    } break;
                case SDLK_n:
                    game.gameOver = true;
                    break;
                case SDLK_ESCAPE: 
                    game.gameOver = true;
                    game.restart = false;
                    break;
            }
        }
    }
}

bool foodSpawnInSnake ( Snake & snake , BlueFood & blueFood ) {
    for ( int i = 0; i < snake.bodySize; i++ ) {
		Cell snakeC = snake.body.get ( i );
        if ( snakeC.x == blueFood.position.x && snakeC.y >= blueFood.position.y ) {
            return true;
        }
    }
    return false;
}

void spawnBlueFood ( Game & game , BlueFood & blueFood , Snake & snake ) {
    if ( blueFood.eaten || blueFood.number == 0 ) {
        bool spawned = false;
        while ( !spawned ) {
            blueFood.position.x = rand () % ( game.cols);
            blueFood.position.y = rand () % ( game.rows  - 3 ) + 3;
            if ( !foodSpawnInSnake ( snake , blueFood ) )
                spawned = true;
        }
        blueFood.eaten = false;
        blueFood.number = 1;
    }
}

void blueFoodEaten ( Snake & snake , BlueFood & blueFood , Game & game ) {
	Cell head = snake.body.get ( 0 );
    if ( head.x == blueFood.position.x && head.y == blueFood.position.y ) {
        blueFood.eaten = true;
        blueFood.number = 0;
		snake.bodySize++;
		snake.body.push ( { snake.body.get ( snake.body.getCurrentSize () - 1 ).x , snake.body.get ( snake.body.getCurrentSize () - 1 ).y } );
        game.points++;
    }
}

void spawnRedFood ( Game & game , RedFood & redFood ) {
    double currentTime = SDL_GetTicks () * 0.001;


    if ( !redFood.eaten && ( currentTime - redFood.spawnTime > redFood.spawnInterval ) ) {
        redFood.eaten = true;
        redFood.spawnTime = currentTime + 5.0;
    }
    if ( redFood.eaten && SDL_GetTicks () * 0.001 - redFood.spawnTime > redFood.spawnInterval ) {
        redFood.position.x = rand () % ( game.cols );
        redFood.position.y = rand () % ( game.rows - 3) + 3;
        redFood.eaten = false;
        redFood.spawnTime = SDL_GetTicks () * 0.001;
    }
}

void checkRedFoodTimer ( Game & game , RedFood & redFood ) {
    double currentTime = SDL_GetTicks () * 0.001;


    if ( !redFood.eaten && ( currentTime - redFood.spawnTime > redFood.spawnInterval ) ) {
        redFood.eaten = true;
        redFood.spawnTime = currentTime + 5.0;
    }
}

void redFoodEaten ( Snake & snake , RedFood & redFood , Game & game ) {
	Cell head = snake.body.get ( 0 );
    if ( head.x == redFood.position.x && head.y == redFood.position.y ) {
        if ( rand () % 2 == 0 && snake.bodySize > 2 )
            snake.bodySize--;
        else
            snake.snakeDelay += snake.snakeDelay * 0.05;
        redFood.eaten = true;

    }
}

void drawProgressBar ( Game & game , RedFood & redFood ) {
    if ( !redFood.eaten ) {
        double elapsed = SDL_GetTicks () * 0.001 - redFood.spawnTime;
        double progress = ( 1.0 - elapsed / redFood.spawnInterval );

        int barWidth = 200; 
        int barHeight = 10; 
        int x = ( game.screenWidth - barWidth ) / 2;
        int y = game.rectangleHeight - 2;
        SDLUtils::DrawRectangle ( game.screen , x , y , barWidth , barHeight , game.black , game.black );
        SDLUtils::DrawRectangle ( game.screen , x , y , ( int ) ( barWidth * progress ) , barHeight , game.red , game.red );
    }
}

void speedChange ( Game & game , Snake & snake ) {
    if ( game.worldTime - game.lastSpeedchange > game.speedUpInterval ) {
        game.lastSpeedchange = game.worldTime;
        snake.snakeDelay = ( int ) ( snake.snakeDelay * ( 1 - game.speedUpValue ) );
    }
}

void render ( Game & game , Snake & snake, BlueFood&blueFood, RedFood&redFood ) {
    SDL_FillRect ( game.screen , NULL , game.backgroundColor );
    SDLUtils::DrawRectangle ( game.screen , 0 , game.rectangleHeight , game.screenWidth , game.screenHeight - game.rectangleHeight , game.red , game.black );
	if ( !blueFood.eaten )
    {
        int x = blueFood.position.x * game.cellSize;
        int y = blueFood.position.y * game.cellSize;
        SDLUtils::DrawSurface ( game.screen , blueFood.picture , x , y );
    }
	if ( !redFood.eaten )
    {
        int x = redFood.position.x * game.cellSize;
        int y = redFood.position.y * game.cellSize;
        SDLUtils::DrawSurface ( game.screen , redFood.picture , x , y );
    }
    for ( int i = 0; i < snake.body.getCurrentSize (); ++i ) {
        Cell cell = snake.body.get ( i );
        drawCell ( game.screen , cell.x , cell.y , game.cellSize , game.snakeColor );
    }
    printInfo ( game ,snake);
	drawProgressBar ( game , redFood );
    SDL_UpdateTexture ( game.texture , NULL , game.screen->pixels , game.screen->pitch );
    SDL_RenderCopy ( game.renderer , game.texture , NULL , NULL );
    SDL_RenderPresent ( game.renderer );
}

void loadFoodCharset ( Game & game , BlueFood & bluefood , RedFood & redFood ) {
    game.charset = SDL_LoadBMP ( "./cs8x8.bmp" );
    bluefood.picture = SDL_LoadBMP ( "./snakebmp/blueFood.bmp" );
    redFood.picture = SDL_LoadBMP ( "./snakebmp/redFood.bmp" );
    if ( game.charset == NULL || bluefood.picture == NULL || redFood.picture == NULL ) {
        printf ( "charset or food loading error: %s\n" , SDL_GetError () );
        SDL_FreeSurface ( game.screen );
        SDL_DestroyTexture ( game.texture );
        SDL_DestroyWindow ( game.window );
        SDL_DestroyRenderer ( game.renderer );
        SDL_Quit ();
        exit ( 0 );
    };
    SDL_SetColorKey ( game.charset , true , 0x000000 );
}

void printFinalInfo ( Game & game ) {
    char text[128];
    SDLUtils::DrawRectangle ( game.screen , 0 , game.rectangleHeight , game.screenWidth , game.screenHeight - game.rectangleHeight , game.blue , game.red );
    sprintf ( text , "you have survived %.1lf s" , game.worldTime );
    SDLUtils::DrawString ( game.screen , game.screenWidth / 2 - strlen ( text ) * 8 / 2 , game.screenHeight / 2 , text , game.charset );
    sprintf ( text , "Do you want to play again?" );
    SDLUtils::DrawString ( game.screen , game.screenWidth / 2 - strlen ( text ) * 8 / 2 , game.screenHeight / 2 + 10 , text , game.charset );
    sprintf ( text , "press n for new game or ESC to close game!" );
    SDLUtils::DrawString ( game.screen , game.screenWidth / 2 - strlen ( text ) * 8 / 2 , game.screenHeight / 2 + 20 , text , game.charset );
    SDL_UpdateTexture ( game.texture , NULL , game.screen->pixels , game.screen->pitch );
    SDL_RenderCopy ( game.renderer , game.texture , NULL , NULL );
    SDL_RenderPresent ( game.renderer );
}

void gameLoop ( Game & game , Snake & snake, BlueFood& bluefood, RedFood& redFood ) {
    game.startTime = SDL_GetTicks ();
    while ( !game.gameOver ) {
        game.endTime = SDL_GetTicks ();
        game.deltaTime = ( game.endTime - game.startTime ) * 0.001;
        game.startTime = game.endTime;

        game.worldTime += game.deltaTime;

        if ( SDL_GetTicks () - snake.lastMoveTime > snake.snakeDelay ) {
            updateSnake ( snake );
            if ( checkCollision ( snake , game ) ) {
                while ( !game.gameOver ) {
                    printFinalInfo ( game );
                    handleInput ( snake , game );
                }
            }
			if ( checkWallHit ( snake , game ) ) {
				hitWall ( snake , game);
			}
            snake.lastMoveTime = SDL_GetTicks();
            game.userTurn = false;
        }
		spawnBlueFood ( game , bluefood , snake );
		blueFoodEaten ( snake , bluefood , game );
		spawnRedFood ( game , redFood );
		redFoodEaten ( snake , redFood , game );
        handleInput ( snake , game );
		speedChange ( game , snake );
        render ( game , snake, bluefood, redFood );
    }
}

int main ( int argc , char ** argv ) {
    if ( SDL_Init ( SDL_INIT_VIDEO ) != 0 ) {
        printf ( "SDL_Init error: %s\n" , SDL_GetError () );
        return 1;
    }
    bool restart = true;
    while(restart){
        Game game;
        Snake snake;
        BlueFood blueFood;
        RedFood redFood;
        loadFoodCharset ( game , blueFood , redFood );
        SDL_CreateWindowAndRenderer ( game.screenWidth , game.screenHeight , 0 , &game.window , &game.renderer );
        game.screen = SDL_CreateRGBSurface ( 0 , game.screenWidth , game.screenHeight , 32 , 0x00FF0000 , 0x0000FF00 , 0x000000FF , 0xFF000000 );
        game.texture = SDL_CreateTexture ( game.renderer , SDL_PIXELFORMAT_ARGB8888 , SDL_TEXTUREACCESS_STREAMING , game.screenWidth , game.screenHeight );
        setColors ( game );
        initGame ( game );
        initSnake ( snake , game );

        gameLoop ( game , snake, blueFood, redFood );
		restart = game.restart;
        SDL_DestroyTexture ( game.texture );
        SDL_FreeSurface ( game.screen );
        SDL_DestroyRenderer ( game.renderer );
        SDL_DestroyWindow ( game.window );
        SDL_Quit ();
    }

    return 0;
}
