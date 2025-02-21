#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "functions.h"
#include "Vector.h"

#define LEFT_WALL 0
#define HEAD 0
#define NONE 0
#define TOP_WALL 2
#define UP -1
#define DOWN 1
#define RIGHT 1
#define LEFT -1
#define BAR_WIDTH 200
#define BAR_HEIGHT 10
#define BAR_Y 50
#define TIME_CONVERTER 0.001
#define FIVE_PERCENT 0.05

struct Snake {
    Vector<Cell> body;
    int directionX = NONE;  
    int directionY = UP; 
	double lastMoveTime = 0.0;
	double delay = 130.0;
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
    double spawnInterval = 8.0;
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
    sprintf ( text , "time = %.1lf s  implemented points: 1,2,3,4,A,B,C,D" , game.worldTime );
    SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 10 , text , game.charset );
    sprintf ( text , "Esc - exit, n - new game, \x18 up, \x1B left, \x1A down, \x19 right" );
    SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 26 , text , game.charset );
    sprintf ( text , "points - %d snake size: %d snake delay: %.2f" , game.points, snake.bodySize, snake.delay );
    SDLUtils::DrawString ( game.screen , game.screen->w / 2 - strlen ( text ) * 8 / 2 , 42 , text , game.charset );
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

void moveSnake ( Snake & snake ) {
    if ( SDL_GetTicks () - snake.lastMoveTime > snake.delay ) {
        for ( int i = snake.body.getCurrentSize () - 1; i > 0; --i ) {
            snake.body.push ( snake.body.get ( i - 1 ) , i );
        }
        snake.body.push ( { snake.body.get ( HEAD ).x + snake.directionX, snake.body.get ( HEAD ).y + snake.directionY } , HEAD );
        snake.lastMoveTime = SDL_GetTicks ();
    }
}

bool checkWallHit ( Snake & snake , Game & game ) {
	Cell head = snake.body.get ( HEAD );
	if ( head.x + snake.directionX < LEFT_WALL && snake.directionX == LEFT )
        return true;
    if( head.x + snake.directionX >= game.cols && snake.directionX == RIGHT)
        return true;
    if( head.y + snake.directionY <= TOP_WALL && snake.directionY == UP)
        return true;
    if( head.y + snake.directionY >= game.rows && snake.directionY == DOWN )
        return true;
	return false;
}

bool checkCollision ( Snake & snake , Game & game ) {
    Cell head = snake.body.get ( HEAD );
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
	Cell head = snake.body.get ( HEAD );
    int rightDirectionX = -snake.directionY; // turn of 90 deegrees so x=-y, y=x
	int rightDirectionY = snake.directionX;
	int leftDX = snake.directionY;
    int leftDY = -snake.directionX;
	Cell rightCellCheck = { head.x + rightDirectionX, head.y + rightDirectionY };
    bool canTurnRight = ( rightCellCheck.x >= LEFT_WALL && rightCellCheck.x < game.cols && rightCellCheck.y > TOP_WALL && rightCellCheck.y < game.rows );
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
                    if ( snake.directionY == NONE) { 
                        snake.directionX = NONE; 
                        snake.directionY = UP; 
						game.userTurn = true;
                    } break;
                case SDLK_DOWN:
                    if ( snake.directionY == NONE ) { 
                        snake.directionX = NONE; 
                        snake.directionY = DOWN; 
                        game.userTurn = true;
                    } break;
                case SDLK_LEFT: 
                    if ( snake.directionX == NONE ) { 
                        snake.directionX = LEFT; 
                        snake.directionY = NONE;
                        game.userTurn = true;
                    } break;
                case SDLK_RIGHT: 
                    if ( snake.directionX == NONE ) { 
                        snake.directionX = RIGHT; 
                        snake.directionY = NONE;
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
        if ( snakeC.x == blueFood.position.x && snakeC.y == blueFood.position.y ) {
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
	Cell head = snake.body.get ( HEAD );
    if ( head.x == blueFood.position.x && head.y == blueFood.position.y ) {
        blueFood.eaten = true;
        blueFood.number = 0;
		snake.bodySize++;
		snake.body.push ( { snake.body.get ( snake.body.getCurrentSize () - 1 ).x , snake.body.get ( snake.body.getCurrentSize () - 1 ).y } );
        game.points++;
    }
}

void spawnRedFood ( Game & game , RedFood & redFood ) {
    double currentTime = SDL_GetTicks () * TIME_CONVERTER;
    if ( !redFood.eaten && ( currentTime - redFood.spawnTime > redFood.spawnInterval ) ) {
        redFood.eaten = true;
        redFood.spawnTime = currentTime + redFood.spawnInterval;
        game.points--;
    }
    if ( redFood.eaten && SDL_GetTicks () * TIME_CONVERTER - redFood.spawnTime > redFood.spawnInterval ) {
        redFood.position.x = rand () % ( game.cols );
        redFood.position.y = rand () % ( game.rows - game.rectangleHeight / game.cellSize) + game.rectangleHeight / game.cellSize;
        redFood.eaten = false;
        redFood.spawnTime = SDL_GetTicks () * TIME_CONVERTER;
    }
}

void redFoodEaten ( Snake & snake , RedFood & redFood , Game & game ) {
	Cell head = snake.body.get ( HEAD );
    if ( head.x == redFood.position.x && head.y == redFood.position.y && !redFood.eaten ) {
        if ( rand () % 2 == 0 && snake.bodySize > 2 )
            snake.bodySize--;
        else
            snake.delay += snake.delay * FIVE_PERCENT;
        redFood.eaten = true;
		game.points++;
    }
    
}

void drawProgressBar ( Game & game , RedFood & redFood ) {
    if ( !redFood.eaten ) {
        double elapsed = SDL_GetTicks () * TIME_CONVERTER - redFood.spawnTime;
        double progress = ( 1.0 - elapsed / redFood.spawnInterval );
        int x = ( game.screenWidth - BAR_WIDTH ) / 2;
        SDLUtils::DrawRectangle ( game.screen , x , BAR_Y , BAR_WIDTH , BAR_HEIGHT , game.black , game.black );
        SDLUtils::DrawRectangle ( game.screen , x , BAR_Y , ( int ) ( BAR_WIDTH * progress ) , BAR_HEIGHT , game.red , game.red );
    }
}

void speedChange ( Game & game , Snake & snake ) {
    if ( game.worldTime - game.lastSpeedchange > game.speedUpInterval ) {
        game.lastSpeedchange = game.worldTime;
        snake.delay = ( int ) ( snake.delay * ( 1 - game.speedUpValue ) );
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
        SDLUtils::DrawRectangle ( game.screen , cell.x * game.cellSize , cell.y * game.cellSize , game.cellSize , game.cellSize , game.snakeColor , game.snakeColor );
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
        if ( checkWallHit ( snake , game ) ) {
            hitWall ( snake , game );
        }
        if ( checkCollision ( snake , game ) ) {
            while ( !game.gameOver ) {
                printFinalInfo ( game );
                handleInput ( snake , game );
            }
        }
        moveSnake ( snake );
        handleInput ( snake , game );
		spawnBlueFood ( game , bluefood , snake );
		blueFoodEaten ( snake , bluefood , game );
		spawnRedFood ( game , redFood );
		redFoodEaten ( snake , redFood , game );
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
    }
    SDL_Quit ();
    return 0;
}
