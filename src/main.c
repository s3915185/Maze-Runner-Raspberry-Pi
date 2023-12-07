#include "Maze.h"
#include "../uart/uart.h"
#include "printf.h"
#include "mbox.h"
#include "framebf.h"
#include "timer.h"
#include "gameElement.h"
#include "Frontier.c"
#include "largeImage.h"
#include "img.h"
#include "video.h"
#include "monster.h"
#define MAX_CMD_SIZE 100
#define MAX_TOKENS 100
#define HISTORY_SIZE 10
#define MAX_REQ_VALUE 10
int widthScreen = 40;
int heightScreen = 20;



char history[HISTORY_SIZE][MAX_CMD_SIZE];
int history_count = 0;
int current_history_index = 0;
int current_image = 0;
int vertical_offset = 0;
int screenHeight = 768;
char *maze;
int *monsters;
int **pathsPointer;
int **monstersStepCountPointer;

static unsigned char our_memory[1024 * 1024]; // reserve 1 MB for malloc
static size_t next_index = 0;
int inGame = 0;
Frontier *myFrontier;
const char *names[] = {"Claire Macken", "Julia Gaimster", "Robert McClelland"};
const char *roles[] = {"General Director RMIT Vietnam", "Dean of School of Communication & Design", "Dean of The Business School"};
enum Color {
  RAINBOW = 0,
  COOLCOLOR = 1
};

static int gameLevel = 0;
int direction = 0;
int *x_bomb;
int *y_bomb;
int x_direct = 20;
int y_direct = 0;
static int bombIndex = 0;
int *bombListTimer;

int *monstersTimer;

int *monstersPathIndex;


int *x_monsters;
int *y_monsters;

int player = 0;
int *playerPointer;

int *path;
int *dist;


int x_treasure = 0;
int y_treasure = 0;

static int mapHorizontalPadding = 0;
static int mapVerticalPadding = 0;
void *malloc(size_t sz)
{
    void *mem;

    if (sizeof our_memory - next_index < sz)
        return NULL;

    mem = &our_memory[next_index];
    next_index += sz;
    return mem;
}

void free(void *mem)
{
    // we cheat, and don't free anything.
    
}

void getNearFrontier(const char *maze, int x, int y)
{
    if (y * widthScreen + x - widthScreen < 0)
    {
        myFrontier->north = 10;
    }
    else
    {
        myFrontier->north = maze[y * widthScreen + x - widthScreen];
    }
    if (y * widthScreen + x + 1 > ((y + 1) * widthScreen))
    {
        myFrontier->east = 10;
    }
    else
    {
        myFrontier->east = maze[y * widthScreen + x + 1];
    }
    if (y * widthScreen + x + widthScreen > widthScreen * heightScreen)
    {
        myFrontier->south = 10;
    }
    else
    {
        myFrontier->south = maze[y * widthScreen + x + widthScreen];
    }
    if (y * widthScreen + x - 1 < y * widthScreen)
    {
        myFrontier->west = 10;
    }
    else
    {
        myFrontier->west = maze[y * widthScreen + x - 1];
    }
}

int checkDirection(int dir) {
    switch (dir)
    {
    case 3:
        direction = 3;
        if (myFrontier->north == 0 || myFrontier->north == 3 || myFrontier->north == 4 || myFrontier->north == 5) {
            return 1;
        }
        if (myFrontier->north == 2) {
            return 2;
        }
        break;
    case 4:
        direction = 4;
        if (myFrontier->east == 0 || myFrontier->east == 3 || myFrontier->east == 4 || myFrontier->east == 5) {
            return 1;
        }
        if (myFrontier->east == 2) {
            return 2;
        }
        break;
    case 5:
        direction = 5;
        if (myFrontier->south == 0 || myFrontier->south == 3 || myFrontier->south == 4 || myFrontier->south == 5) {
            return 1;
        }
        if (myFrontier->south == 2) {
            return 2;
        }
        break;
    case 6:
        direction = 6;
        if (myFrontier->west == 0 || myFrontier->west == 3 || myFrontier->west == 4 || myFrontier->west == 5) {
            return 1;
        }
        if (myFrontier->west == 2) {
            return 2;
        }
        break;
    default:
        break;
    }
    return 0;
}

void clearPlayeFrame(int heightScreen, int widthScreen)
{
    for (int j = 0; j < heightScreen; j++)
    {
        for (int i = 0; i < widthScreen; i++)
        {
            drawPixelARGB32(i + x_direct + mapHorizontalPadding, j + y_direct + mapVerticalPadding, 0x00000000);
        }
    }
}

void draw_image()
{
    // Looping through image array line by line.
    for (int j = 0; j < 532; j++)
    {
        // Looping through image array pixel by pixel of line j.
        for (int i = 0; i < 800; i++)
        {
            // Printing each pixel in correct order of the array and lines, columns.
            drawPixelARGB32(i + 112, j + 118, managerallArray[0][j * 800 + i]);
        }
    }
    drawString(112, 60, roles[0], 0x0F);
    drawString(400, 680, names[0], 0x0F);
}

void draw_LargeImage()
{
    // Looping through image array line by line.
    for (int j = 0; j < 1820; j++)
    {
        // Looping through image array pixel by pixel of line j.
        for (int i = 0; i < 1024; i++)
        {
            // Printing each pixel in correct order of the array and lines, columns.
            drawPixelARGB32(i, j, largeImageallArray[0][j * 1024 + i]);
        }
    }
}



void draw_destination(int x, int y)
{
    for (int j = 0; j < 20; j++)
    {
        for (int i = 0; i < 21; i++)
        {
            drawPixelARGB32(i + x + mapHorizontalPadding, j + y + mapVerticalPadding, epd_bitmap_destination[j * 21 + i]);
        }
    }
}

void draw_monster(int x, int y)
{
    for (int a = 0; a < epd_bitmap_monsterallArray_LEN; a++) {
        for (int j = 0; j < 20; j++)
        {
            for (int i = 0; i < 20; i++)
            {
                drawPixelARGB32(i + x + mapHorizontalPadding, j + y + mapVerticalPadding, epd_bitmap_monsterallArray[a][j * 20 + i]);
            }
        }
    }
}


void drawMap(const char *maze, int widthScreen, int heightScreen)
{
    int x, y;
    for (y = 0; y < heightScreen; y++)
    {
        for (x = 0; x < widthScreen; x++)
        {
            switch (maze[y * widthScreen + x])
            {
            case 1:
                draw_wall(x * 20, y * 20);
                break;
            case 2:
                draw_destination(x * 20, y * 20);
                x_treasure = x * 20;
                y_treasure = y * 20;
                break;
            case 3:
                draw_monster(x * 20, y * 20);
                break;
            case 5:
                draw_destination(x * 20, y * 20);
                x_direct = x * 20;
                y_direct = y * 20;
                // getNearFrontier(maze, x, y);
                break;
            }
        }
    }
    for (int x = 0; x < widthScreen; x++)
    {
        draw_wall(x * 20, heightScreen * 20);
    }
    for (int y = 0; y < heightScreen; y++)
    {
        draw_wall(widthScreen * 20, y * 20);
    }
}

void draw_wall(int x, int y)
{
    for (int j = 0; j < 20; j++)
    {
        for (int i = 0; i < 20; i++)
        {
            drawPixelARGB32(i + x + mapHorizontalPadding, j + y + mapVerticalPadding, epd_bitmap_wall[j * 20 + i]);
        }
    }
}


// Function draw video
void draw_video()
{
    for (int a = 0; a < epd_bitmap_allArray_LEN; a++)
    {
        for (int j = 0; j < 240; j++)
        {
            for (int i = 0; i < 426; i++)
            {
                drawPixelARGB32(i, j, epd_bitmap_allArray1[a][j * 426 + i]);
            }
        }
        wait_msec(200000);
    }
}
const char *commands[] = {
    "help", "clear", "setcolor", "showinfo", "video", "smallimg", "game"
    // Add more commands as needed
};

char *strcpy(char *dest, const char *src)
{
    char *originalDest = dest;

    while (*src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0'; // Add the null-terminator at the end

    return originalDest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *originalDest = dest;

    size_t i;
    for (i = 0; i < n && *src != '\0'; i++)
    {
        *dest = *src;
        dest++;
        src++;
    }

    // Fill the remaining characters with null terminators if necessary
    for (; i < n; i++)
    {
        *dest = '\0';
        dest++;
    }

    return originalDest;
}

char *strtok(char *str, const char *delimiter, char **context)
{
    if (str != NULL)
        *context = str;

    if (*context == NULL || **context == '\0')
        return NULL;

    char *token_start = *context;
    while (**context != '\0')
    {
        int is_delimiter = 0;
        for (size_t i = 0; delimiter[i] != '\0'; i++)
        {
            if (**context == delimiter[i])
            {
                is_delimiter = 1;
                break;
            }
        }

        if (is_delimiter)
        {
            **context = '\0';
            (*context)++;
            if (token_start != *context)
                return token_start;
        }
        else
        {
            (*context)++;
            if (**context == '\0')
            {
                return token_start;
            }
        }
    }

    return token_start;
}

// String length Function
size_t strlen(const char *str)
{
    size_t length = 0;
    while (*str != '\0')
    {
        length++;
        str++;
    }
    return length;
}

// String Compare Function
int strcmp(const char *str1, const char *str2)
{
    while (*str1 != '\0' && *str2 != '\0')
    {
        if (*str1 != *str2)
        {
            return (*str1 > *str2) ? 1 : -1;
        }
        str1++;
        str2++;
    }

    if (*str1 == '\0' && *str2 == '\0')
    {
        return 0;
    }
    else if (*str1 == '\0')
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

void help_command(const char *cmd)
{
    // Implement help command logic here
    // Print the information about the supported commands
    // ...
    uart_puts("Available commands:\n");
    uart_puts("For more information on a specific command, type help <command-name>:\n");
    uart_puts("help                                 Show brief information of all commands\n");
    uart_puts("help <command_name>                  Show full information of the command\n");
    uart_puts("clear                                Clear screen\n");
    uart_puts("setcolor                             Set text color, and/or background color of the console to one of the following colors: BLACK, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE\n");
    uart_puts("showinfo                             Show board revision and board MAC address\n");
}

void help_info(const char *cmd)
{
    if (strcmp(cmd, "setcolor") == 0)
    {
        uart_puts("Set text color only:                         setcolor -t <color>.\n");
        uart_puts("Set background color only:                   setcolor -b <color>.\n");
        uart_puts("Set color for both background and text:      setcolor -t <color> -b <color or setcolor -b <color> -t<color>.\n");
        uart_puts("Accepted color and writing format:  Black,  Red, Green, Yellow, Blue, Purple, Cyan, White.\n");
        uart_puts("Examples\nMyBareMetalOS> setcolor -t yellow\nMyBareMetalOS> setcolor -b yellow -t white\n");
    }
    else if (strcmp(cmd, "clear") == 0)
    {
        uart_puts("Clear screen (in our terminal it will scroll down to current position of the cursor).\n");
        uart_puts("Example: MyBareMetalOS> clear\n");
    }
    else if (strcmp(cmd, "showinfo") == 0)
    {
        uart_puts("Show board revision and board MAC address in correct format/ meaningful information.\n");
        uart_puts("Example: MyBareMetalOS> showinfo\n");
    }
    else
    {
        uart_puts("Unrecognized command\n");
    }
}
int strncmp(const char *str1, const char *str2, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (str1[i] != str2[i])
        {
            return (unsigned char)str1[i] - (unsigned char)str2[i];
        }
        if (str1[i] == '\0')
        {
            return 0; // Reached end of one or both strings
        }
    }
    return 0; // Both strings are equal up to n characters
}

void clear_command()
{
    uart_puts("\x1B[2J\x1B[H");
}

void deleteCommand(char *cmd_buffer, int *index)
{
    if (*index > 0)
    {
        (*index)--;
        cmd_buffer[*index] = '\0';
        uart_puts("\b \b"); // Move the cursor back, write a space to clear the character, and move the cursor back again
    }
}

char *strcat(char *dest, const char *src)
{
    char *originalDest = dest;

    // Move the destination pointer to the end of the string
    while (*dest != '\0')
    {
        dest++;
    }

    // Copy the source string to the end of the destination string
    while (*src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
    }

    // Add the null-terminator at the end
    *dest = '\0';

    return originalDest;
}

const char *colorOptions[] = {
    "BLACK", "RED", "GREEN", "YELLOW", "BLUE", "PURPLE", "CYAN", "WHITE"};

const char *textColorCode[] = {
    "\033[30m", "\033[31m", "\033[32m", "\033[33m",
    "\033[34m", "\033[35m", "\033[36m", "\033[37m"};

const char *backgroundColorCode[] = {
    "\033[40m", "\033[41m", "\033[42m", "\033[43m",
    "\033[44m", "\033[45m", "\033[46m", "\033[47m"};

void setBackGroundColor(const char *backgroundArray)
{
    int backgroundColorIndex = -1;
    // If backgroundArray is provided, find the index of the background color
    if (backgroundArray != NULL)
    {
        for (int i = 0; i < sizeof(colorOptions) / sizeof(colorOptions[0]); i++)
        {
            if (strcmp(backgroundArray, colorOptions[i]) == 0)
            {
                backgroundColorIndex = i;
                break;
            }
        }
    }

    if (backgroundColorIndex != -1)
    {
        uart_puts((char *)backgroundColorCode[backgroundColorIndex]);
    }
    else
    {
        uart_puts("Invalid background color. Supported colors: BLACK, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE.\n");
    }
}

void setTextColor(const char *textColor)
{
    int textColorIndex = -1;

    // If textColor is provided, find the index of the text color in the colorOptions array
    if (textColor != NULL)
    {
        for (int i = 0; i < sizeof(colorOptions) / sizeof(colorOptions[0]); i++)
        {
            if (strcmp(textColor, colorOptions[i]) == 0)
            {
                textColorIndex = i;
                break;
            }
        }
    }

    if (textColorIndex != -1)
    {
        uart_puts((char *)textColorCode[textColorIndex]);
    }
    else
    {
        uart_puts("Invalid text color. Supported colors: BLACK, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE.\n");
    }
}

// ... (other code)

void *memset(void *ptr, int value, size_t num)
{
    unsigned char *bytePtr = (unsigned char *)ptr;

    for (size_t i = 0; i < num; i++)
    {
        *bytePtr = (unsigned char)value;
        bytePtr++;
    }

    return ptr;
}

// Function to perform auto-completion
void autocomplete(char *cmd_buffer, int *cmd_index)
{
    int matches = 0;
    int match_index = -1;
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        if (strncmp(commands[i], cmd_buffer, *cmd_index) == 0)
        {
            matches++;
            match_index = i;
        }
    }

    if (matches == 1)
    {
        strcpy(cmd_buffer, commands[match_index]);
        *cmd_index = strlen(commands[match_index]);
        uart_puts("\r");
        uart_puts("\nMyBareMetalOS> "); // Print the prompt
        uart_puts(cmd_buffer);
    }
    else if (matches > 1)
    {
        uart_puts("\nPossible completions:");
        for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
        {
            if (strncmp(commands[i], cmd_buffer, *cmd_index) == 0)
            {
                uart_puts(" ");
                uart_puts((char *)commands[i]);
            }
        }
        uart_puts("\n");
        *cmd_index = 0;
        uart_puts("MyBareMetalOS> ");
        memset(cmd_buffer, '\0', MAX_CMD_SIZE);
    }
    else
    {
        uart_puts("\nNo suggestions found.\n");
        uart_puts("MyBareMetalOS> ");
        uart_puts(cmd_buffer);
    }
}

void play_game()
{
    GameGenerator();
}

void clearGame(int widthScreen, int heightScreen)
{
    for (int j = 0; j < heightScreen * 20; j++)
    {
        for (int i = 0; i < widthScreen * 20; i++)
        {
            drawPixelARGB32(i + mapHorizontalPadding, j + mapVerticalPadding, 0x00000000);
        }
    }
}

void clear_image(int heightScreen, int widthScreen)
{
    for (int j = 0; j < heightScreen; j++)
    {
        for (int i = 0; i < widthScreen; i++)
        {
            drawPixelARGB32(i, j, 0x00000000);
        }
    }
} 
void clearFrameBox(int x, int y) {
    for (int j = 0; j < 20; j++)
    {
        for (int i = 0; i < 20; i++)
        {
            drawPixelARGB32(i + x + mapHorizontalPadding, j + y + mapVerticalPadding, 0x00000000);
        }
    }
}
void clearMonsterFrame(int heightScreen, int widthScreen, int x, int y)
{
    for (int j = 0; j < heightScreen; j++)
    {
        for (int i = 0; i < widthScreen; i++)
        {
            drawPixelARGB32(i + x + mapHorizontalPadding, j + y + mapVerticalPadding, 0x00000000);
        }
    }
}

int abs(int n) {
    return (n < 0) ? -n : n;
}


void GameGenerator()
{
    gameLevel++;
    mapHorizontalPadding = (1024 - widthScreen * 20) / 2 - 10;
    mapVerticalPadding = (768 - heightScreen * 20) / 2 - 10;
    x_bomb = (int *)malloc(sizeof(int) * 10);
    y_bomb = (int *)malloc(sizeof(int) * 10);
    bombListTimer = (int *)malloc(sizeof(int) * 10);
    dist = (int *)malloc(sizeof(int));


    monsters = (int *)malloc(sizeof(int)*gameLevel);
    maze = (char *)malloc(widthScreen * heightScreen * sizeof(char));
    pathsPointer = (int **)malloc(gameLevel * sizeof(int*));
    monstersStepCountPointer = (int**)malloc(gameLevel*sizeof(int*));


    monstersTimer = (int *)malloc(sizeof(int)*gameLevel);
    for (int i = 0; i < gameLevel; i++) {
        monstersTimer[i] = 0;
    }


    monstersPathIndex = (int *)malloc(sizeof(int)*gameLevel);
    for (int i = 0; i < gameLevel; i++) {
        monstersPathIndex[i] = 0;
    }


    x_monsters = (int *)malloc(sizeof(int)*gameLevel);
    y_monsters = (int *)malloc(sizeof(int)*gameLevel);

    if (maze == NULL)
    {
        printf("Not enough memory, the game cant be generated!");
    }
    clearGame(widthScreen, heightScreen);
    GenerateMaze(maze, widthScreen, heightScreen, playerPointer, monsters, gameLevel);
    for (int i = 0; i < gameLevel; i++) {
        int *pathTemp = (char *)malloc(widthScreen * heightScreen * sizeof(char));
        int *distTemp = (int *)malloc(sizeof(int));
        FindToDirection(maze, widthScreen, heightScreen, monsters[i], monsters, pathTemp, distTemp, 0, gameLevel);
        pathsPointer[i] = pathTemp;
        monstersStepCountPointer[i] = distTemp;
    }
    ShowMaze(maze, widthScreen, heightScreen);
    drawMap(maze, widthScreen, heightScreen);
    inGame = 1;
}
void cli()
{
    static char cli_buffer[MAX_CMD_SIZE]; // int location [][];
    static int index = 0;
    char c;
    int is_img = 0;
    int is_IMG = 0;
    static int bombTimer = 0;
    static int bombExploded = 0;
    // read and send back each char
    if (inGame == 0) {
        c = uart_getc();
    }
    if (inGame == 1) {
        wait_msec(100000);
        for (int i = 0; i < bombIndex; i++) {
            if (bombListTimer[i] > 30) {
                bombListTimer[i] = 0;
                bombExploded++;
                maze[y_bomb[i] * widthScreen + x_bomb[i] - 1] = 0;
                maze[y_bomb[i] * widthScreen + x_bomb[i] + 1] = 0;
                maze[y_bomb[i] * widthScreen + x_bomb[i] - widthScreen] = 0;
                maze[y_bomb[i] * widthScreen + x_bomb[i] + widthScreen] = 0;
                clearFrameBox((x_bomb[i]) * 20 - 20, y_bomb[i] * 20);
                clearFrameBox((x_bomb[i]) * 20 + 20, y_bomb[i] * 20);
                clearFrameBox(x_bomb[i] * 20, (y_bomb[i]) * 20 + 20);
                clearFrameBox(x_bomb[i] * 20, (y_bomb[i]) * 20 - 20);
                for (int b = 0; b < gameLevel; b++) {
                    if ((abs(x_monsters[b] - x_bomb[i] * 20) <= 21 && abs(y_monsters[b] - y_bomb[i] * 20) <= 11) || (abs(x_monsters[b] - x_bomb[i] * 20) <= 11 && abs(y_monsters[b] - y_bomb[i] * 20) <= 21)) {
                        monstersStepCountPointer[b] = 0;
                        printf("Got a monster");
                        printf("this is monstersPathIndex: %d\n", *(monstersPathIndex + b));
                        printf("this i monsterStepCountPointer: %d\n", **(monstersStepCountPointer + b));
                    }
                }
                if (bombExploded == bombIndex) {
                    bombExploded = 0;
                    bombIndex = 0;
                }
            }
        }

        for (int i = 0; i < bombIndex; i++) {
            bombListTimer[i]++;
        }
        for (int i = 0; i < gameLevel; i++) {
            monstersTimer[i]++;
            if (monstersTimer[i] % 5 == 0) {
                if (monstersStepCountPointer[i] == 0) {

                }
                else if (*(monstersPathIndex + i) >= (**(monstersStepCountPointer + i))) {
                    int *pathTemp = (char *)malloc(widthScreen * heightScreen * sizeof(char));
                    int *distTemp = (int *)malloc(sizeof(int));
                    FindToDirection(maze, widthScreen, heightScreen, monsters[i], monsters, pathTemp, distTemp, 0, gameLevel);
                    monstersPathIndex[i] = 0;
                    pathsPointer[i] = pathTemp;
                    monstersStepCountPointer[i] = distTemp;
                }
                if (*(monstersPathIndex + i) < (**(monstersStepCountPointer + i))) {
                    clearMonsterFrame(20, 21, x_monsters[i], y_monsters[i]);
                    x_monsters[i] = pathsPointer[i][*(monstersPathIndex + i)] % widthScreen * 20;
                    y_monsters[i] = pathsPointer[i][*(monstersPathIndex + i)] / widthScreen * 20;
                    ShowMaze(maze, widthScreen, heightScreen);
                    maze[monsters[i]] = 0;
                    (*(monstersPathIndex + i))++;
                    if (*(monstersPathIndex + i) >= (**(monstersStepCountPointer + i))) {
                        maze[monsters[i]] = 3;
                    }
                    else {
                        monsters[i] = pathsPointer[i][*(monstersPathIndex + i)];
                        maze[monsters[i]] = 3;
                    }
                    draw_monster(x_monsters[i], y_monsters[i]);
                    if ((abs(x_monsters[i] - x_direct) <= 21 && abs(y_monsters[i] - y_direct) <= 11) || (abs(x_monsters[i] - x_direct) <= 11 && abs(y_monsters[i] - y_direct) <= 21)) {
                        inGame = 0;
                        gameLevel = 0;
                        uart_puts("\n");
                        uart_puts("MyBareMetalOS> ");
                    }
                }
            }
        }
    }
    if (inGame == 1) {
        c = getUart();
        if (c == 'w')
        {
            if (checkDirection(3) == 1)
            {
                clearPlayeFrame(20, 21);
                y_direct -= 20;
                maze[player] = 0;
                player = player - widthScreen;
                maze[player] = 5;
            }
            if (checkDirection(3) == 2)
            {
                GameGenerator();
                return;
            }
        }
        else if (c == 'a')
        {
            if (checkDirection(6) == 1)
            {
                clearPlayeFrame(20, 21);
                x_direct -= 20;
                maze[player] = 0;
                player = player - 1;
                maze[player] = 5;
            }
            if (checkDirection(6) == 2)
            {
                GameGenerator();
                return;
            }
        }
        else if (c == 's')
        {
            if (checkDirection(5) == 1)
            {
                clearPlayeFrame(20, 21);
                y_direct += 20;
                maze[player] = 0;
                player = player + widthScreen;
                maze[player] = 5;
            }
            if (checkDirection(5) == 2)
            {
                GameGenerator();
                return;
            }
        }
        else if (c == 'd')
        {
            if (checkDirection(4) == 1)
            {
                clearPlayeFrame(20, 21);
                x_direct += 20;
                maze[player] = 0;
                player = player + 1;
                maze[player] = 5;
            }
            if (checkDirection(4) == 2)
            {
                GameGenerator();
                return;
            }
        }
        /* WALL BOMB
        */
        else if (c == 'k') {
            x_bomb[bombIndex] = x_direct / 20;
            y_bomb[bombIndex] = y_direct / 20;
            bombListTimer[bombIndex] = 0;
            bombIndex++;
        }
        /* MONSTER KILLER BOMB
        */
        else if (c == 'j') {
            int x_bomb = x_direct / 20;
            int y_bomb = y_direct / 20;
        }
        getNearFrontier(maze, x_direct / 20, y_direct / 20);
        draw_destination(x_direct, y_direct);
        draw_destination(x_treasure, y_treasure);
        return;
    }



    uart_sendc(c);

    // put into a buffer until got new line character
    if (c == 127)
    {
        deleteCommand(cli_buffer, &index);
    }
    else if (c != '\n' && c != '\t')
    {
        cli_buffer[index] = c; // Store into the buffer
        index++;
    }
    else if (c == '\n')
    {
        cli_buffer[index] = '\0';
        uart_puts("\nGot commands: ");
        uart_puts(cli_buffer);
        uart_puts("\n");
        current_history_index = history_count; // Set the index for history browsing
        /* Compare with supported commands and execute
         * ........................................... */
        char *tokens[MAX_TOKENS]; // Array to store tokens
        char *token;
        char *saveptr = NULL;

        token = strtok(cli_buffer, " ", &saveptr);
        int numTokens = 0;

        while (token != NULL && numTokens < MAX_TOKENS)
        {
            tokens[numTokens] = token;
            numTokens++;
            token = strtok(NULL, " ", &saveptr);
        }

        if (numTokens > 0 && strcmp(tokens[0], "help") == 0)
        {
            if (numTokens == 1)
            {
                help_command(cli_buffer);
            }
            else if (numTokens == 2)
            {
                help_info(tokens[1]);
            }
            else
            {
                uart_puts("Unrecognized command: \n");
            }
        }
        else if (strcmp(tokens[0], "img") == 0)
        {
            clearPlayeFrame(1024, 1820);
            is_img = 1;
            draw_image();
            uart_puts("Welcome to Image Slideshow Mode\n");
            uart_puts("To navigate, use the following keys:\n");
            uart_puts("Press 'a' for previous image\n");
            uart_puts("Press 'd' for next image\n");
            uart_puts("Press 'q' to exit slideshow\n");
        }
        else if (strcmp(tokens[0], "IMG") == 0)
        {
            clearPlayeFrame(1024, 1820);
            is_IMG = 1;
            draw_LargeImage();
            uart_puts("Welcome to Large Image Mode\n");
            uart_puts("To review the image, use the following keys:\n");
            uart_puts("Press 'w' to scroll up the image\n");
            uart_puts("Press 's' to scroll down the image\n");
            uart_puts("Press 'q' to exit slideshow\n");
        }
        else if (strcmp(tokens[0], "video") == 0)
        {   
            clearPlayeFrame(1024, 1820);
            draw_video();
        }
        else if (strcmp(tokens[0], "game") == 0)
        {
            play_game();
        }

        // Return to command line
        index = 0;

        uart_puts("MyBareMetalOS> ");
    }
    else if (c == '\t')
    {
        cli_buffer[index] = '\0';
        autocomplete(cli_buffer, &index);
    }

    while (is_img == 1)
    {
        char keyPressed = getUart();
        if (keyPressed == 'd')
        {   
            clear_image(768,1024);
            current_image++;
            if (current_image > managerallArray_LEN - 1)
            {
                current_image = 0;
            }
       
            for (int j = 0; j < 532; j++)
            {
                // Looping through image array pixel by pixel of line j.
                for (int i = 0; i < 800; i++)
                {
                    // Printing each pixel in correct order of the array and lines, columns.
                    drawPixelARGB32(i + 112, j + 118, managerallArray[current_image][j * 800 + i]);
                }
            }
            // draw_ImageString(100, 59, roles[current_image], COOLCOLOR);
            // draw_ImageString(350, 680, names[current_image], COOLCOLOR);
            drawString(112, 60, roles[current_image], 0x0F);
            drawString(400, 680, names[current_image], 0x0F);
        }

        if (keyPressed == 'a')
        {   
            clear_image(768,1024);
            if (current_image >= 0)
            {
                current_image--;
            }

            if (current_image < 0)
            {
                current_image = managerallArray_LEN - 1;
            }
    
            for (int j = 0; j < 532; j++)
            {
                // Looping through image array pixel by pixel of line j.
                for (int i = 0; i < 800; i++)
                {
                    // Printing each pixel in correct order of the array and lines, columns.
                    drawPixelARGB32(i + 112, j + 118, managerallArray[current_image][j * 800 + i]);
                }
            }   
            // draw_ImageString(100, 59, roles[current_image], COOLCOLOR);
            // draw_ImageString(350, 680, names[current_image], COOLCOLOR);
            drawString(112, 60, roles[current_image], 0x0F);
            drawString(400, 680, names[current_image], 0x0F);
        }

        if (keyPressed == 'q')
        {   
            uart_puts("\x1B[K");
            uart_puts("\r");
            uart_puts("You have exited the Slideshow Mode !!!\n");
            uart_puts("MyBareMetalOS> ");
            break;
        }
    }
    while (is_IMG == 1)
    {
        char keyPressed = getUart();
        if (keyPressed == 'w')
        {
            cli_buffer[index] = '\0';
            vertical_offset -= 20;
            if (vertical_offset < 0)
                vertical_offset = 0;

            int start_row = vertical_offset;
            int end_row = vertical_offset + screenHeight; // Only draw up to the screen height

            // Looping through image array line by line.
            for (int j = start_row; j < end_row; j++)
            {
                // Looping through image array pixel by pixel of line j.
                for (int i = 0; i < 1024; i++)
                {
                    // Printing each pixel in correct order of the array and lines, columns.
                    drawPixelARGB32(i, j - start_row, largeImageallArray[0][j * 1024 + i]);
                }
            }
        }

        if (keyPressed == 's')
        {
            vertical_offset += 20;
            if (vertical_offset > (1820 - screenHeight))
                vertical_offset = 1820 - screenHeight;

            int start_row = vertical_offset;
            int end_row = vertical_offset + screenHeight; // Only draw up to the screen height

            // Looping through image array line by line.
            for (int j = start_row; j < end_row; j++)
            {
                // Looping through image array pixel by pixel of line j.
                for (int i = 0; i < 1024; i++)
                {
                    // Printing each pixel in correct order of the array and lines, columns.
                    drawPixelARGB32(i, j - start_row, largeImageallArray[0][j * 1024 + i]);
                }
            }
        }
        if (keyPressed == 'q')
        {   
            uart_puts("\x1B[K");
            uart_puts("\r");
            uart_puts("You have exited the Large Image Mode !!!\n");
            uart_puts("MyBareMetalOS> ");
            break;
        }
    }
}

void mbox_buffer_setup(unsigned int buffer_addr, unsigned int tag_identifier,
                       unsigned int **res_data, unsigned int res_length,
                       unsigned int req_length, ...);

void main()
{
    // set up serial console
    framebf_init();
    uart_init();
    drawString(200, 300,"EEET2490 - Embedded System: OS and Interfacing", 0x04);
    drawString(200, 350,"Assignment 3 - Group 21", 0x0E);
    drawString(200, 400,"<Student name>          <Student ID>", 0x06);
    drawString(200,500,"Nguyen Giang Huy   -     S3836454", 0x0A);
    drawString(200,450,"Hua Nam Huy        -     S3811308", 0x09);
    drawString(200,550,"Le Hong Thai       -     S3752577", 0x0F);
    drawString(200,600,"Tran Hoang Vu      -     S3915185", 0x0D);

    uart_puts("\n");
    uart_puts("MyBareMetalOS> ");
    
    playerPointer = &player;
    // run CLI
    while (1)
    {
        cli();
    }
}