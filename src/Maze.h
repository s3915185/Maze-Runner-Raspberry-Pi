#include "../gcclib/stddef.h"
#include "../gcclib/stdint.h"
#include "../gcclib/stdarg.h"

static int destination;
static int monster;
/* Display the maze. */
void ShowMaze(const char *maze, int width, int height);

/*  Carve the maze starting at x, y. */
void CarveMaze(char *maze, int width, int height, int x, int y);

/* Generate maze in matrix maze with size width, height. */
void GenerateMaze(char *maze, int width, int height, int *playerPointer, int *monsters, int gameLevel);

int FindToDirection(char *maze, int width, int height, int chaser, int *monsters, int *path, int *dist, int findingDist, int gameLevel);

int rand_range(int min, int max);