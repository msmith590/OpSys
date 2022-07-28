#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

/*  Diagram showing move order and possible moves
 *   from starting point S
 *
 *   +---+---+---+---+---+
 *   |   | 1 |   | 8 |   |
 *   +---+---+---+---+---+
 *   | 2 |   |   |   | 7 |
 *   +---+---+---+---+---+
 *   |   |   | S |   |   |
 *   +---+---+---+---+---+
 *   | 3 |   |   |   | 6 |
 *   +---+---+---+---+---+
 *   |   | 4 |   | 5 |   |
 *   +---+---+---+---+---+
*/

extern int next_thread_id;
extern int max_squares;
extern int total_tours;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief This struct stores the required argument information for child threads that are
 * created when multiple moves exist on a Knight's Tour
 * 
 */
typedef struct
{
    int** board;
    int move;
    int dimensions[2]; // dimensions[0] = m; dimensions[1] = n;
    int position[2]; // position[0] = r; position[1] = c;
    int threadID;
} boardData_t;

/**
 * @brief This struct stores two arrays that, when interpreted together, represent possible
 * future positions. The function pathfinder(..) inserts the integer values for both arrays
 * when determining future paths.
 * 
 */
typedef struct
{
    int row[8];
    int col[8];
} nextMoves_t;


/* Function that checks to see if a string can be fully converted into an integer */
int isInteger(char* string) {
    int i = 0, valid = 1; // valid is set to true initially (represented by a value of 1)
    while (*(string + i) != '\0') {
        if (isdigit(*(string + i)) == 0) {
            valid = 0;
            break;
        }
        i++;
    }
    return valid;
}

/* Debugging function that prints out board contents */
void printBoard(int** board, int m, int n)
{
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%d  ", board[i][j]);
        }
        printf("\n");
    }
}

void freeBoard(boardData_t* arg) {
    for (int i = 0; i < arg->dimensions[0]; i++) {
        free(arg->board[i]);
    }
    free(arg->board);
    free(arg);
}

int pathfinder(int m, int n, int r, int c, int** board, nextMoves_t* possiblePos)
{ // Assumes that we have a valid board and r, c are valid
    int moves = 0;
    for (int i = 0; i < 8; i++) {
        possiblePos->row[i] = -1;
        possiblePos->col[i] = -1;
    }

    if ((r - 2) >= 0 && (c - 1) >= 0)
    { /* Move 1 (see diagram above) */
        if (board[r - 2][c - 1] == 0)
        {
            moves++;
            possiblePos->row[0] = r - 2;
            possiblePos->col[0] = c - 1;
        }
    }
    if ((r - 1) >= 0 && (c - 2) >= 0)
    { /* Move 2 (see diagram above) */
        if (board[r - 1][c - 2] == 0)
        {
            moves++;
            possiblePos->row[1] = r - 1;
            possiblePos->col[1] = c - 2;
        }
    }
    if ((r + 1) < m && (c - 2) >= 0)
    { /* Move 3 (see diagram above) */
        if (board[r + 1][c - 2] == 0)
        {
            moves++;
            possiblePos->row[2] = r + 1;
            possiblePos->col[2] = c - 2;
        }
    }
    if ((r + 2) < m && (c - 1) >= 0)
    { /* Move 4 (see diagram above) */
        if (board[r + 2][c - 1] == 0)
        {
            moves++;
            possiblePos->row[3] = r + 2;
            possiblePos->col[3] = c - 1;
        }
    }
    if ((r + 2) < m && (c + 1) < n)
    { /* Move 5 (see diagram above) */
        if (board[r + 2][c + 1] == 0)
        {
            moves++;
            possiblePos->row[4] = r + 2;
            possiblePos->col[4] = c + 1;
        }
    }
    if ((r + 1) < m && (c + 2) < n)
    { /* Move 6 (see diagram above) */
        if (board[r + 1][c + 2] == 0)
        {
            moves++;
            possiblePos->row[5] = r + 1;
            possiblePos->col[5] = c + 2;
        }
    }
    if ((r - 1) >= 0 && (c + 2) < n)
    { /* Move 7 (see diagram above) */
        if (board[r - 1][c + 2] == 0)
        {
            moves++;
            possiblePos->row[6] = r - 1;
            possiblePos->col[6] = c + 2;
        }
    }
    if ((r - 2) >= 0 && (c + 1) < n)
    { /* Move 8 (see diagram above) */
        if (board[r - 2][c + 1] == 0)
        {
            moves++;
            possiblePos->row[7] = r - 2;
            possiblePos->col[7] = c + 1;
        }
    }

    return moves;
}

int runTour(boardData_t* data) {
    nextMoves_t* possibleMoves = calloc(1, sizeof(nextMoves_t));
    int boardSize = data->dimensions[0] * data->dimensions[1];
    
    while (data->move < boardSize) {
        int numMoves = pathfinder(data->dimensions[0], data->dimensions[1], data->position[0], data->position[1], data->board, possibleMoves);
        
        if (numMoves == 0) {
            
        }
    }
    
    return data->move;
}

int simulate( int argc, char * argv[] ) {
    setvbuf( stdout, NULL, _IONBF, 0 ); // disables buffered output for more predictable results

    if (argc != 5) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: a.out <m> <n> <r> <c>\n");
        return EXIT_FAILURE;
    }

#ifdef DEBUG_MODE
    int intTest = isInteger(*(argv + 1));
    printf("Argument provided for m: %s\n", *(argv + 1));
    printf("Integer? (1 means yes, 0 means no): %d\n", intTest);

    intTest = isInteger(*(argv + 2));
    printf("Argument provided for n: %s\n", *(argv + 2));
    printf("Integer? (1 means yes, 0 means no): %d\n", intTest);

    intTest = isInteger(*(argv + 3));
    printf("Argument provided for r: %s\n", *(argv + 3));
    printf("Integer? (1 means yes, 0 means no): %d\n", intTest);

    intTest = isInteger(*(argv + 4));
    printf("Argument provided for c: %s\n", *(argv + 4));
    printf("Integer? (1 means yes, 0 means no): %d\n", intTest);
#endif

    int m, n, r, c; // m == # of rows; n == # of columns; r == starting row of knight; c == starting column of knight

    if ((isInteger(argv[1]) && atoi(argv[1]) > 2) && (isInteger(argv[2]) && atoi(argv[2]) > 2) && // Validates inputs provided
        (isInteger(argv[3]) && atoi(argv[3]) >= 0 && atoi(argv[3]) < atoi(argv[1])) &&
        (isInteger(argv[4]) && atoi(argv[4]) >= 0 && atoi(argv[4]) < atoi(argv[2]))) {
        m = atoi(argv[1]);
        n = atoi(argv[2]);
        r = atoi(argv[3]);
        c = atoi(argv[4]);
    } else {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: a.out <m> <n> <r> <c>\n");
        return EXIT_FAILURE;
    }

    // Initial board creation
    int** board = calloc(m, sizeof(int*));
    for (int i = 0; i < m; i++) {
        *(board + i) = calloc(n, sizeof(int));
    }
    board[r][c] = 1; // Starting position
    boardData_t* initial = calloc(1, sizeof(boardData_t));
    initial->board = board;
    initial->dimensions[0] = m;
    initial->dimensions[1] = n;
    initial->position[0] = r;
    initial->position[1] = c;
    initial->move = 1;

    runTour(initial);
    
    freeBoard(initial);

    return EXIT_SUCCESS;
}
