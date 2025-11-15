#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "raylib.h"

#define N 8
#define SQUARE_SIZE 80        
#define BOARD_SIZE (N * SQUARE_SIZE)       // here it is 640 px
#define UI_WIDTH 250          // UI is referred to as the right side panel
#define SCREEN_WIDTH (BOARD_SIZE + UI_WIDTH)
#define SCREEN_HEIGHT BOARD_SIZE
#define MAX_MOVES 11
#define totaltime 120.0f 

int board[N][N];
int rowUsed[N];
int columnused[N];
int diag1[2 * N - 1];      // from top right to bottom left
int diag2[2 * N - 1];      // from top left to bottom right
int move_r[N];    // used to keep the track of r and c as we have undo option as well
int move_c[N];
int moveCount = 0;
int totalMoves = 0;
int hintUsed[N][N];

int queensPlaced = 0;   // shows how many queens have been placed
int hintsRemaining = 3; // kept max hints = 3
int gameWon = 0;
int timeUp = 0;
int gameOver = 0;

float timepassed = 0.0f; 

char message[100] = "Welcome! Click a square.";
int hint_r = -1, hint_c = -1; // -1 means no hint yet

//rectangle in raylib is a structure that contains:{x,y,width,height}
Rectangle hintButton;
Rectangle undoButton;
Rectangle resetButton;
Rectangle quitButton;

void setboard()
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            board[i][j] = 0;             // if a cell is assigned zero it means no queens is placed there 
            hintUsed[i][j] = 0;
        }
        rowUsed[i] = columnused[i] = 0;
    }
    for (int i = 0; i < 2 * N - 1; i++)
    {
        diag1[i] = diag2[i] = 0;
    }
    moveCount = 0;    
    totalMoves = 0;
    queensPlaced = 0;
    hintsRemaining = 3;
    gameWon = 0;
    timeUp = 0;
    gameOver = 0;
    timepassed = 0.0f;
    hint_r = -1;
    hint_c = -1;
    strcpy(message, "Board reset. Place your first queen.");
}

int isSafe(int r, int c)    // if gets 1 then pops queen is there already
{
    return !(rowUsed[r] || columnused[c] || diag1[r + c] || diag2[r - c + N - 1]);
}

void placeQueen(int r, int c)
{
    board[r][c] = 1;
    rowUsed[r]++;
    columnused[c]++;
    diag1[r + c]++;           
    diag2[r - c + N - 1]++;
    move_r[moveCount] = r;
    move_c[moveCount] = c;
    moveCount++;
    queensPlaced++;
}

void removeQueen(int r, int c)       // we are changing the index so that garbage value is not stored in the removed index
/*6 goes to 4 
  8 goes to 6 and so on
  */
{
    board[r][c] = 0;
    rowUsed[r]--;
    columnused[c]--;
    diag1[r + c]--;
    diag2[r - c + N - 1]--;
    for (int i = 0; i < moveCount; i++)
    {
        if (move_r[i] == r && move_c[i] == c)
        {
            for (int j = i; j < moveCount - 1; j++)
            {
                move_r[j] = move_r[j + 1];    // this stores value where queen is placed so that we may undo if we have the track of it
                move_c[j] = move_c[j + 1];
            }
            moveCount--;
            queensPlaced--;
            break;
        }
    }
}

int findHint(int *r_out, int *c_out)
// ensures hint is not repeated
// how it checks: if cell has 1 leaves that
{
    for (int r = 0; r < N; r++)
    {
        for (int c = 0; c < N; c++)
        {
            if (!board[r][c] && !hintUsed[r][c] && isSafe(r, c))
            {
                hintUsed[r][c] = 1;
                *r_out = r;      // pointer returning acceptable r and c
                *c_out = c;
                return 1;
            }
        }
    }
    return 0;
}

void DrawGame()
// f(x) of raylib 
{
    for (int r = 0; r < N; r++)
    {
        for (int c = 0; c < N; c++)
        {
            // choosing alternate colors to be of different colors
            Color tileColor = ((r + c) % 2 == 0)
                                  ? (Color){240, 217, 181, 255} // cream square
                                  : (Color){118, 150, 86, 255}; // green square
            // in raylib color code is [R,G,B,A] ; A = opacity's measure {0-transparent and 255-opaque}

            
            DrawRectangle(c * SQUARE_SIZE, r * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, tileColor);
            // f(x) of raylib
            // inputs:{x,y,width,height,color}

            if (board[r][c])
            {
                int textSize = 50;
                int textWidth = MeasureText("Q", textSize);
                DrawText("Q", c * SQUARE_SIZE + (SQUARE_SIZE - textWidth) / 2,
                         r * SQUARE_SIZE + (SQUARE_SIZE - textSize) / 2, textSize, BLACK);
            }

            if (r == hint_r && c == hint_c)
            {
                DrawRectangleLinesEx((Rectangle){c * SQUARE_SIZE, r * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE}, 4, YELLOW);
            }
        }
    }
// DrawText is a raylib f(x) used to enter text using parameter: {string,x,y,textsize,color}
// DrawRectangleLinesEx is a raylib f(x) that draws outline for the rectangle
// parameters : struct rectangle ; linethickness ; color

    for (int i = 0; i <= N; i++)
    {
        DrawLine(i * SQUARE_SIZE, 0, i * SQUARE_SIZE, BOARD_SIZE, DARKGRAY);
        DrawLine(0, i * SQUARE_SIZE, BOARD_SIZE, i * SQUARE_SIZE, DARKGRAY);
    }
// drawline is also a raylib f(x) that makes a line
    for (int i = 0; i < N; i++)
    {
        DrawText(TextFormat("%c", 'a' + i), i * SQUARE_SIZE + SQUARE_SIZE / 2 - 8, BOARD_SIZE - 25, 20, BLACK);
        DrawText(TextFormat("%d", i + 1), BOARD_SIZE + 5, i * SQUARE_SIZE + SQUARE_SIZE / 2 - 10, 20, BLACK);
    }
}
// textformat is also a raylib f(x) that returns formatted string that can be passed directly to drawtext

// drawui - used to draw other in built functions like drwa rectangel drwa line and draw text
// uiX - x coordinate that represents the starting position ofnthe UI panel
void DrawUI()
{
    int uiX = BOARD_SIZE + 20;
    int uiY = 20;
    DrawRectangle(BOARD_SIZE, 0, UI_WIDTH, SCREEN_HEIGHT, WHITE);
    DrawLine(BOARD_SIZE, 0, BOARD_SIZE, SCREEN_HEIGHT, LIGHTGRAY);

    DrawText("Eight Queens", uiX, uiY, 30, BLACK);
    uiY += 60;

    DrawText(TextFormat("Queens Placed: %d / %d", queensPlaced, N), uiX, uiY, 20, BLACK);
    uiY += 30;

    DrawText(TextFormat("Moves Used: %d / %d", totalMoves, MAX_MOVES), uiX, uiY, 20, (totalMoves < MAX_MOVES) ? BLACK : RED);
    uiY += 30;

    float timeLeft = totaltime - timepassed;
    if (timeLeft < 0)
        timeLeft = 0;
    int minutes = (int)(timeLeft / 60);
    int seconds = (int)timeLeft % 60;
    DrawText(TextFormat("Time Left: %02d:%02d", minutes, seconds), uiX, uiY, 20, (timeUp || gameOver) ? RED : BLACK);
    uiY += 30;

    DrawText(TextFormat("Hints Remaining: %d", hintsRemaining), uiX, uiY, 20, (hintsRemaining > 0) ? BLACK : RED);
    uiY += 50; // Extra space before buttons

    //leveling buttons lower so they don’t overlap
    hintButton.y = uiY;
    undoButton.y = hintButton.y + 65;
    resetButton.y = undoButton.y + 65;
    quitButton.y = SCREEN_HEIGHT - 80;

    DrawRectangleRec(hintButton, (!timeUp && !gameOver && hintsRemaining > 0) ? LIGHTGRAY : GRAY);
    DrawText("Hint", hintButton.x + 55, hintButton.y + 15, 20, BLACK);

    DrawRectangleRec(undoButton, (!timeUp && !gameOver && moveCount > 0) ? LIGHTGRAY : GRAY);
    DrawText("Undo Last", undoButton.x + 35, undoButton.y + 15, 20, BLACK);

    DrawRectangleRec(resetButton, LIGHTGRAY);
    DrawText("Reset", resetButton.x + 50, resetButton.y + 15, 20, BLACK);

    DrawRectangleRec(quitButton, MAROON);
    DrawText("Quit", quitButton.x + 55, quitButton.y + 15, 20, WHITE);

    int msgY = quitButton.y - 150;
    DrawText("Status:", uiX, msgY, 20, DARKGRAY);
    DrawTextEx(GetFontDefault(), message, (Vector2){(float)uiX, (float)(msgY + 25)}, 20.0f, 2.0f, BLACK);

    if (gameWon)
    {
        DrawRectangle(0, 0, BOARD_SIZE, SCREEN_HEIGHT, (Color){0, 0, 0, 150});
        DrawText("CONGRATULATIONS! ", BOARD_SIZE / 2 - 250, SCREEN_HEIGHT / 2 - 30, 40, GOLD);
        DrawText("Press 'Reset' to play again.", BOARD_SIZE / 2 - 160, SCREEN_HEIGHT / 2 + 20, 20, RAYWHITE);
    }

    if (timeUp)
    {
        DrawRectangle(0, 0, BOARD_SIZE, SCREEN_HEIGHT, (Color){0, 0, 0, 150});
        DrawText("TIME UP!", BOARD_SIZE / 2 - 120, SCREEN_HEIGHT / 2 - 30, 40, RED);
        DrawText("Press 'Reset' to try again.", BOARD_SIZE / 2 - 150, SCREEN_HEIGHT / 2 + 20, 20, RAYWHITE);
    }

    if (gameOver)
    {
        DrawRectangle(0, 0, BOARD_SIZE, SCREEN_HEIGHT, (Color){0, 0, 0, 150});
        DrawText("GAME OVER!", BOARD_SIZE / 2 - 180, SCREEN_HEIGHT / 2 - 30, 40, RED);
        DrawText("Press 'Reset' to try again.", BOARD_SIZE / 2 - 150, SCREEN_HEIGHT / 2 + 20, 20, RAYWHITE);
    }
}


void UpdateGame()
{
    if (!gameWon && !timeUp && !gameOver)
        timepassed += GetFrameTime();

    if (timepassed >= totaltime && !timeUp && !gameWon)
    {
        timeUp = 1;
        strcpy(message, "Time Up! Press Reset.");
    }

    Vector2 mousePos = GetMousePosition();

    if ((gameWon || timeUp || gameOver) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mousePos, resetButton))
            setboard();
        return;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mousePos, hintButton))
        {
            if (!timeUp && !gameOver && hintsRemaining > 0)
            {
                if (findHint(&hint_r, &hint_c))
                {
                    hintsRemaining--;
                    sprintf(message, "HINT: Try %c%d.", 'a' + hint_c, hint_r + 1);
                }
                else
                    strcpy(message, "No new safe moves.");
            }
            else if (timeUp || gameOver)
                strcpy(message, "You cannot use hints now!");
            else
                strcpy(message, "No hints remaining!");
        }

        else if (CheckCollisionPointRec(mousePos, undoButton))
        {
            if (!timeUp && !gameOver && moveCount > 0)
            {
                int r = move_r[moveCount - 1];
                int c = move_c[moveCount - 1];
                removeQueen(r, c);
                hint_r = -1;
                hint_c = -1;
                sprintf(message, "↩ Undid last move: %c%d.", 'a' + c, r + 1);
            }
            else if (timeUp || gameOver)
                strcpy(message, "You cannot undo now!");
            else
                strcpy(message, "⚠ Nothing to undo!");
        }

        else if (CheckCollisionPointRec(mousePos, resetButton))
            setboard();

        else if (!timeUp && !gameOver && mousePos.x < BOARD_SIZE && mousePos.y < SCREEN_HEIGHT)
        {
            totalMoves++;
            int c = (int)(mousePos.x / SQUARE_SIZE);
            int r = (int)(mousePos.y / SQUARE_SIZE);
            hint_r = -1;
            hint_c = -1;

            if (board[r][c] == 1)
            {
                removeQueen(r, c);
                sprintf(message, "Removed queen at %c%d.", 'a' + c, r + 1);
            }
            else
            {
                if (isSafe(r, c))
                {
                    placeQueen(r, c);
                    sprintf(message, "Queen placed at %c%d.", 'a' + c, r + 1);
                }
                else
                {
                    sprintf(message, "Invalid move! %c%d.", 'a' + c, r + 1);
                }
            }

            if (totalMoves >= MAX_MOVES && !gameWon)
            {
                gameOver = 1;
                strcpy(message, "GAME OVER! Out of moves.");
            }
        }
    }

    if (queensPlaced == N && !gameWon)
    {
        gameWon = 1;
        strcpy(message, "You won! Press Reset.");
    }
}

// getframetime() - raylib f(x) used for FPS make the game fps independent
// getmouseposition() - gets current coordinates of where mouse clicked
// checkcollisionpointrec(mousepos , reset button) - checks for mouse ptr inside resent button area
// ismousebuttonpressed(mouse_left_button) - checks for clicking of button
// vector2: structure that stores 2 numbers here x and y
// settargetfps : used to setfps
int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Eight Queens Game [raylib]");
    SetTargetFPS(60);

    int btnX = BOARD_SIZE + 40;
    int btnWidth = UI_WIDTH - 80;
    int btnHeight = 50;
    hintButton = (Rectangle){(float)btnX, 0, (float)btnWidth, (float)btnHeight};
    undoButton = (Rectangle){(float)btnX, 0, (float)btnWidth, (float)btnHeight};
    resetButton = (Rectangle){(float)btnX, 0, (float)btnWidth, (float)btnHeight};
    quitButton = (Rectangle){(float)btnX, 0, (float)btnWidth, (float)btnHeight};

    setboard();

    int quitGame = 0;
    while (!WindowShouldClose() && !quitGame)
    {
        UpdateGame();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), quitButton))
            quitGame = 1;

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawGame();
        DrawUI();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
