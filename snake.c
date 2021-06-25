#include <ncurses.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>
#include <time.h>

/*
Projekttitel: SNAKE
Beschreibung: Version des Videospiel-Klassikers Snake. Man steuert eine Schlange,
um Fruechte einzusammeln, wodurch die Schlange jeweils um ein Segment waechst. Bei 
Kollision mit der Wand oder mit sich selbst ist das Spiel vorbei.
*/

//===============================================================================================//
//Ein Segment der Schlange, beinhaltet die erforderlichen Koordinaten.

typedef struct {
    int x;
    int y;
} snakeSegment;

//===============================================================================================//
//Segment des Hindernisses.

typedef struct {
    int x;
    int y;
} obstacleSegment;

//===============================================================================================//
//Funktion, die ueberprüft, ob eine Taste gedrueckt wird.
//Quelle für die Funktion(https://www.linuxquestions.org/questions/programming-9/way-of-use-of-kbhit-function-in-gcc-776487/).

int kbhit (void) {
    struct timeval tv;
    fd_set read_fd;

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&read_fd);
    FD_SET(0, &read_fd);

    if(select(1, &read_fd, NULL, NULL, &tv) == -1) {
        return 0;
    }

    if(FD_ISSET(0,&read_fd)) {
        return 1;
    }

    return 0;
}

//===============================================================================================//
//Zeichnet das Hindernis.

void paintObstacle(obstacleSegment * obstacle) {
    for(int i = 0; i < 47; i++) {
        mvprintw(obstacle[i].y, obstacle[i].x, "+");
    }
}

//===============================================================================================//
//Zeichnet das Spielfeld.

void paintBorders(int height, int width) {
    for(int i = 1; i < width; i++) {
        mvprintw(0, i, "_");
        mvprintw(height, i, "-");
    }
    for(int i = 1; i < height; i++) {
        mvprintw(i, 0, "|");
        mvprintw(i, width, "|");
    }
}

//===============================================================================================//
//Funktion, die testest, ob sich der Kopf der Schlange im Hindernis befindet.

int obstacleCollisionTest(snakeSegment * snake, obstacleSegment * obstacle) {
    for(int i = 0; i < 47; i++) {
        if (snake[0].x == obstacle[i].x && snake[0].y == obstacle[i].y){
            return 1;
        }
    }
    return 0;
}

//===============================================================================================//
//Funktion, die testet, ob sich der Kopf der Schlange in der Wand befindet.

int wallCollisionTest(snakeSegment * snake, int width, int height) {
    if(snake[0].x == 0 || snake[0].x == width || snake[0].y == 0 || snake[0].y == height) {
        return 1;
    } else {
        return 0;
    }
}

//===============================================================================================//
//Testet, ob die Schlange mit sich selbst kollidiert ist, in dem die Koordinaten des Kopfes mit
//den Koordinaten aller anderen Segmente verglichen werden.

int snakeCollisionTest(snakeSegment * snake, int snakeLength) {
    for(int i = 1; i < snakeLength; i++) {
        if(snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            return 1;
        }
    }
    return 0;
}

//===============================================================================================//
//Verschiebt den Kopf der Schlange in die gerade angegebene Richtung, alle anderen Segmente folgen
//dem jeweils vorhergehenden Element.

void snakeMove(snakeSegment * snake, int dirX, int dirY, int snakeLength) {
    int previousX = 0;
    int previousY = 0;
    int currentX = snake[0].x;
    int currentY = snake[0].y;
    snake[0].x += dirX;
    snake[0].y += dirY;
    mvprintw(snake[0].y, snake[0].x, "x");
    
    for(int i = 1; i < snakeLength; i++) {
	    previousX = currentX;
        previousY = currentY;
        currentX = snake[i].x;
        currentY = snake[i].y;
        snake[i].x = previousX;
	    snake[i].y = previousY;
	    mvprintw(snake[i].y, snake[i].x, "*");
    }

    if(dirY == 0) {
        usleep(150000);
    } else {
        usleep(250000);
    }
}

//===============================================================================================//
//Zufaellige Fruchtkoordinaten werden generiert.

void generateFruit(obstacleSegment * obstacle, int * fruitX, int * fruitY, int width, int height) {
    srand(time(NULL));
    int stop = 0;
    int x;
    int y;
    while(!stop) {
        stop = 1;
        x = rand() % (width - 2) + 1;
        y = rand() % (height - 2) + 1;
        for (int i = 0; i < 47; i++) {
            if (obstacle[i].x == x && obstacle[i].y == y) {
                stop = 0;
                break;
            }
        } 
    }
    *fruitX = x;
    *fruitY = y;
}

//===============================================================================================//
//Wenn die Koordinaten des Kopfes der Schlange mit den Fruchtkoordinaten uebereinstimmen, wird ein 
//Schlangenelement hinzugefügt, abhaengig von der Richtung, in die sich die Schlange bewegt. 

int eatFruit(snakeSegment * snake, int * snakeLength, int * score, int dirX, int dirY, int fruitX, int fruitY) {
        if(snake[0].x == fruitX && snake[0].y == fruitY) {
            if(dirX == 1 && dirY == 0) {
                snake[*snakeLength].x = snake[*snakeLength - 1].x - 1;
                snake[*snakeLength].y = snake[*snakeLength - 1].y;
            }
            else if(dirX == -1 && dirY == 0) {
                snake[*snakeLength].x = snake[*snakeLength - 1].x + 1;
                snake[*snakeLength].y = snake[*snakeLength - 1].y;
            } 
            else if(dirX == 0 && dirY == 1) {
                snake[*snakeLength].x = snake[*snakeLength - 1].x;
                snake[*snakeLength].y = snake[*snakeLength - 1].y - 1;
            } 
            else {
                snake[*snakeLength].x = snake[*snakeLength - 1].x;
                snake[*snakeLength].y = snake[*snakeLength - 1].y + 1;
            } 
            
            *snakeLength += 1;
            *score += 1;
            return 1;

        } else {
            return 0;
        }
}

//===============================================================================================//

int main() {
    
    //-----------------------------------------------------// 
    //Koordinaten der einzelnen Segmente fuer das Hindernis auffuellen.

    obstacleSegment obstacle[47]; 
    int oCounter = 0;
    for (int i = 14; i > 7; i--) {
        obstacle[oCounter].x = i;
        obstacle[oCounter].y = 11;
        oCounter++;
    }
    for (int i = 11; i > 4; i--) {
        obstacle[oCounter].x = 8;
        obstacle[oCounter].y = i;
        oCounter++;
    }
    for (int i = 8; i < 27; i++) {
        obstacle[oCounter].x = i;
        obstacle[oCounter].y = 4;
        oCounter++;
    }
    for (int i = 4; i < 11; i++) {
        obstacle[oCounter].x = 27;
        obstacle[oCounter].y = i;
        oCounter++;
    }
    for (int i = 27; i > 20; i--) {
        obstacle[oCounter].x = i;
        obstacle[oCounter].y = 11;
        oCounter++;
    }
   
    //-----------------------------------------------------// 
    
    snakeSegment snake[100];
    snake[0].x = 5;
    snake[0].y = 13;
    int directionX = 1;
    int directionY = 0;
    int key;
    int fruitX, fruitY;
    int score = 0;
    int snakeLength = 1;

    //-----------------------------------------------------// 
    
    initscr(); //Fenster initialisieren. 
    cbreak(); //Ermoeglicht das unterbrechen des Prozesses mit ctrl+c.   
    
    int height, width, start_y, start_x;
    height = 15;
    width = 35;
    start_y = start_x = 0;    
    WINDOW * win = newwin(height, width, start_y, start_x);
    wrefresh(win);
    curs_set(0); //Cursor unsichtbar.
    start_color();

    //Startbildschirm.
    mvprintw(height / 2, (width - 20) / 2, "NAVIGATE WITH W/A/S/D\n  PRESS ANY KEY TO START THE GAME");
    
    noecho(); //Verhindert, dass die gedrueckte Taste auf dem Bildschirm ausgegeben wird.
    getch(); //Erwartet druecken einer Taste.
    
    generateFruit(obstacle, &fruitX, &fruitY, width, height); //Erste Frucht wird generiert.
    
    //-----------------------------------------------------// 
    //Hauptloop des Spiels, laeuft solange die drei Kollisionstests null zurueck geben.
    while(!obstacleCollisionTest(snake, obstacle) && !wallCollisionTest(snake, width, height) && !snakeCollisionTest(snake, snakeLength)) {
        clear(); //Bildschirm wird geloescht.
        
        //Spielfeld printen.
        paintBorders(height, width);        
        paintObstacle(obstacle);

        //Test, ob die Schlange eine Frucht gegessen hat, wenn ja, generieren neuer Fruchtkoordinaten.
        if(eatFruit(snake, &snakeLength, &score, directionX, directionY, fruitX, fruitY)) {
            generateFruit(obstacle, &fruitX, &fruitY, width, height);
        }
        
        //Schlangenbewegung.
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        attron(COLOR_PAIR(1));
        snakeMove(snake, directionX, directionY, snakeLength);
        attroff(COLOR_PAIR(1));
        
        //Frucht wird geprintet.
        init_pair(2, COLOR_RED, COLOR_BLACK);
        attron(COLOR_PAIR(2));    
        mvprintw(fruitY, fruitX, "@");
        attroff(COLOR_PAIR(2));    
        
        refresh(); //Bildschirm wird neu geladen.
        
        //Sobald kbhit() 1 zurueckgibt, wird ueberprueft, um welche Taste es sich handelt und 
        //dementsprechend die Richtung angepasst, wobei darauf geachtet wird, dass die Schlange
        //nicht sofort die Richtung um 180 Grad aendern kann.
        noecho();
        if(kbhit()) {
            key = getch();
            if(key == 'd' && !(directionX == -1 && directionY == 0)) {
                directionX = 1;
                directionY = 0;    
            } 
            if(key == 'a' && !(directionX == 1 && directionY == 0)) {
                directionX = -1;
                directionY = 0;    
            } 
            if(key == 'w' && !(directionX == 0 && directionY == 1)) {
                directionX = 0;
                directionY = -1;    
            } 
            if(key == 's' && !(directionX == 0 && directionY == -1)) {
                directionX = 0;
                directionY = 1;    
            } 
        }
    }
   
    //-----------------------------------------------------// 
    
    clear(); 

    //Endscreen
    mvprintw(height / 2, (width - 9) / 2, "GAME OVER!\n            YOUR SCORE: %d", score); 
    
    noecho();
    getch();
    endwin();

    return 0; 
}
